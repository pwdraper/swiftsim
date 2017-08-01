/*******************************************************************************
 * This file is part of SWIFT.
 * Copyright (c) 2012 Pedro Gonnet (pedro.gonnet@durham.ac.uk)
 *                    Matthieu Schaller (matthieu.schaller@durham.ac.uk)
 *               2015 Peter W. Draper (p.w.draper@durham.ac.uk)
 *               2016 John A. Regan (john.a.regan@durham.ac.uk)
 *                    Tom Theuns (tom.theuns@durham.ac.uk)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

/* Config parameters. */
#include "../config.h"

/* Some standard headers. */
#include <float.h>
#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* MPI headers. */
#ifdef WITH_MPI
#include <mpi.h>
#endif

/* Switch off timers. */
#ifdef TIMER
#undef TIMER
#endif

/* This object's header. */
#include "cell.h"

/* Local headers. */
#include "active.h"
#include "atomic.h"
#include "drift.h"
#include "error.h"
#include "gravity.h"
#include "hydro.h"
#include "hydro_properties.h"
#include "memswap.h"
#include "minmax.h"
#include "scheduler.h"
#include "space.h"
#include "timers.h"

/* Global variables. */
int cell_next_tag = 0;

/**
 * @brief Get the size of the cell subtree.
 *
 * @param c The #cell.
 */
int cell_getsize(struct cell *c) {

  /* Number of cells in this subtree. */
  int count = 1;

  /* Sum up the progeny if split. */
  if (c->split)
    for (int k = 0; k < 8; k++)
      if (c->progeny[k] != NULL) count += cell_getsize(c->progeny[k]);

  /* Return the final count. */
  return count;
}

/**
 * @brief Unpack the data of a given cell and its sub-cells.
 *
 * @param pc An array of packed #pcell.
 * @param c The #cell in which to unpack the #pcell.
 * @param s The #space in which the cells are created.
 *
 * @return The number of cells created.
 */
int cell_unpack(struct pcell *pc, struct cell *c, struct space *s) {

#ifdef WITH_MPI

  /* Unpack the current pcell. */
  c->h_max = pc->h_max;
  c->ti_end_min = pc->ti_end_min;
  c->ti_end_max = pc->ti_end_max;
  c->ti_old_part = pc->ti_old_part;
  c->ti_old_gpart = pc->ti_old_gpart;
  c->count = pc->count;
  c->gcount = pc->gcount;
  c->scount = pc->scount;
  c->tag = pc->tag;

  /* Number of new cells created. */
  int count = 1;

  /* Fill the progeny recursively, depth-first. */
  for (int k = 0; k < 8; k++)
    if (pc->progeny[k] >= 0) {
      struct cell *temp;
      space_getcells(s, 1, &temp);
      temp->count = 0;
      temp->gcount = 0;
      temp->scount = 0;
      temp->loc[0] = c->loc[0];
      temp->loc[1] = c->loc[1];
      temp->loc[2] = c->loc[2];
      temp->width[0] = c->width[0] / 2;
      temp->width[1] = c->width[1] / 2;
      temp->width[2] = c->width[2] / 2;
      temp->dmin = c->dmin / 2;
      if (k & 4) temp->loc[0] += temp->width[0];
      if (k & 2) temp->loc[1] += temp->width[1];
      if (k & 1) temp->loc[2] += temp->width[2];
      temp->depth = c->depth + 1;
      temp->split = 0;
      temp->dx_max_part = 0.f;
      temp->dx_max_gpart = 0.f;
      temp->dx_max_sort = 0.f;
      temp->nodeID = c->nodeID;
      temp->parent = c;
      c->progeny[k] = temp;
      c->split = 1;
      count += cell_unpack(&pc[pc->progeny[k]], temp, s);
    }

  /* Return the total number of unpacked cells. */
  c->pcell_size = count;
  return count;

#else
  error("SWIFT was not compiled with MPI support.");
  return 0;
#endif
}

/**
 * @brief Link the cells recursively to the given #part array.
 *
 * @param c The #cell.
 * @param parts The #part array.
 *
 * @return The number of particles linked.
 */
int cell_link_parts(struct cell *c, struct part *parts) {

  c->parts = parts;

  /* Fill the progeny recursively, depth-first. */
  if (c->split) {
    int offset = 0;
    for (int k = 0; k < 8; k++) {
      if (c->progeny[k] != NULL)
        offset += cell_link_parts(c->progeny[k], &parts[offset]);
    }
  }

  /* Return the total number of linked particles. */
  return c->count;
}

/**
 * @brief Link the cells recursively to the given #gpart array.
 *
 * @param c The #cell.
 * @param gparts The #gpart array.
 *
 * @return The number of particles linked.
 */
int cell_link_gparts(struct cell *c, struct gpart *gparts) {

  c->gparts = gparts;

  /* Fill the progeny recursively, depth-first. */
  if (c->split) {
    int offset = 0;
    for (int k = 0; k < 8; k++) {
      if (c->progeny[k] != NULL)
        offset += cell_link_gparts(c->progeny[k], &gparts[offset]);
    }
  }

  /* Return the total number of linked particles. */
  return c->gcount;
}

/**
 * @brief Link the cells recursively to the given #spart array.
 *
 * @param c The #cell.
 * @param sparts The #spart array.
 *
 * @return The number of particles linked.
 */
int cell_link_sparts(struct cell *c, struct spart *sparts) {

  c->sparts = sparts;

  /* Fill the progeny recursively, depth-first. */
  if (c->split) {
    int offset = 0;
    for (int k = 0; k < 8; k++) {
      if (c->progeny[k] != NULL)
        offset += cell_link_sparts(c->progeny[k], &sparts[offset]);
    }
  }

  /* Return the total number of linked particles. */
  return c->scount;
}

/**
 * @brief Pack the data of the given cell and all it's sub-cells.
 *
 * @param c The #cell.
 * @param pc Pointer to an array of packed cells in which the
 *      cells will be packed.
 *
 * @return The number of packed cells.
 */
int cell_pack(struct cell *c, struct pcell *pc) {

#ifdef WITH_MPI

  /* Start by packing the data of the current cell. */
  pc->h_max = c->h_max;
  pc->ti_end_min = c->ti_end_min;
  pc->ti_end_max = c->ti_end_max;
  pc->ti_old_part = c->ti_old_part;
  pc->ti_old_gpart = c->ti_old_gpart;
  pc->count = c->count;
  pc->gcount = c->gcount;
  pc->scount = c->scount;
  c->tag = pc->tag = atomic_inc(&cell_next_tag) % cell_max_tag;

  /* Fill in the progeny, depth-first recursion. */
  int count = 1;
  for (int k = 0; k < 8; k++)
    if (c->progeny[k] != NULL) {
      pc->progeny[k] = count;
      count += cell_pack(c->progeny[k], &pc[count]);
    } else
      pc->progeny[k] = -1;

  /* Return the number of packed cells used. */
  c->pcell_size = count;
  return count;

#else
  error("SWIFT was not compiled with MPI support.");
  return 0;
#endif
}

/**
 * @brief Pack the time information of the given cell and all it's sub-cells.
 *
 * @param c The #cell.
 * @param ti_ends (output) The time information we pack into
 *
 * @return The number of packed cells.
 */
int cell_pack_ti_ends(struct cell *c, integertime_t *ti_ends) {

#ifdef WITH_MPI

  /* Pack this cell's data. */
  ti_ends[0] = c->ti_end_min;

  /* Fill in the progeny, depth-first recursion. */
  int count = 1;
  for (int k = 0; k < 8; k++)
    if (c->progeny[k] != NULL) {
      count += cell_pack_ti_ends(c->progeny[k], &ti_ends[count]);
    }

  /* Return the number of packed values. */
  return count;

#else
  error("SWIFT was not compiled with MPI support.");
  return 0;
#endif
}

/**
 * @brief Unpack the time information of a given cell and its sub-cells.
 *
 * @param c The #cell
 * @param ti_ends The time information to unpack
 *
 * @return The number of cells created.
 */
int cell_unpack_ti_ends(struct cell *c, integertime_t *ti_ends) {

#ifdef WITH_MPI

  /* Unpack this cell's data. */
  c->ti_end_min = ti_ends[0];

  /* Fill in the progeny, depth-first recursion. */
  int count = 1;
  for (int k = 0; k < 8; k++)
    if (c->progeny[k] != NULL) {
      count += cell_unpack_ti_ends(c->progeny[k], &ti_ends[count]);
    }

  /* Return the number of packed values. */
  return count;

#else
  error("SWIFT was not compiled with MPI support.");
  return 0;
#endif
}

/**
 * @brief Lock a cell for access to its array of #part and hold its parents.
 *
 * @param c The #cell.
 * @return 0 on success, 1 on failure
 */
int cell_locktree(struct cell *c) {

  TIMER_TIC

  /* First of all, try to lock this cell. */
  if (c->hold || lock_trylock(&c->lock) != 0) {
    TIMER_TOC(timer_locktree);
    return 1;
  }

  /* Did somebody hold this cell in the meantime? */
  if (c->hold) {

    /* Unlock this cell. */
    if (lock_unlock(&c->lock) != 0) error("Failed to unlock cell.");

    /* Admit defeat. */
    TIMER_TOC(timer_locktree);
    return 1;
  }

  /* Climb up the tree and lock/hold/unlock. */
  struct cell *finger;
  for (finger = c->parent; finger != NULL; finger = finger->parent) {

    /* Lock this cell. */
    if (lock_trylock(&finger->lock) != 0) break;

    /* Increment the hold. */
    atomic_inc(&finger->hold);

    /* Unlock the cell. */
    if (lock_unlock(&finger->lock) != 0) error("Failed to unlock cell.");
  }

  /* If we reached the top of the tree, we're done. */
  if (finger == NULL) {
    TIMER_TOC(timer_locktree);
    return 0;
  }

  /* Otherwise, we hit a snag. */
  else {

    /* Undo the holds up to finger. */
    for (struct cell *finger2 = c->parent; finger2 != finger;
         finger2 = finger2->parent)
      atomic_dec(&finger2->hold);

    /* Unlock this cell. */
    if (lock_unlock(&c->lock) != 0) error("Failed to unlock cell.");

    /* Admit defeat. */
    TIMER_TOC(timer_locktree);
    return 1;
  }
}

/**
 * @brief Lock a cell for access to its array of #gpart and hold its parents.
 *
 * @param c The #cell.
 * @return 0 on success, 1 on failure
 */
int cell_glocktree(struct cell *c) {

  TIMER_TIC

  /* First of all, try to lock this cell. */
  if (c->ghold || lock_trylock(&c->glock) != 0) {
    TIMER_TOC(timer_locktree);
    return 1;
  }

  /* Did somebody hold this cell in the meantime? */
  if (c->ghold) {

    /* Unlock this cell. */
    if (lock_unlock(&c->glock) != 0) error("Failed to unlock cell.");

    /* Admit defeat. */
    TIMER_TOC(timer_locktree);
    return 1;
  }

  /* Climb up the tree and lock/hold/unlock. */
  struct cell *finger;
  for (finger = c->parent; finger != NULL; finger = finger->parent) {

    /* Lock this cell. */
    if (lock_trylock(&finger->glock) != 0) break;

    /* Increment the hold. */
    atomic_inc(&finger->ghold);

    /* Unlock the cell. */
    if (lock_unlock(&finger->glock) != 0) error("Failed to unlock cell.");
  }

  /* If we reached the top of the tree, we're done. */
  if (finger == NULL) {
    TIMER_TOC(timer_locktree);
    return 0;
  }

  /* Otherwise, we hit a snag. */
  else {

    /* Undo the holds up to finger. */
    for (struct cell *finger2 = c->parent; finger2 != finger;
         finger2 = finger2->parent)
      atomic_dec(&finger2->ghold);

    /* Unlock this cell. */
    if (lock_unlock(&c->glock) != 0) error("Failed to unlock cell.");

    /* Admit defeat. */
    TIMER_TOC(timer_locktree);
    return 1;
  }
}

/**
 * @brief Lock a cell for access to its #multipole and hold its parents.
 *
 * @param c The #cell.
 * @return 0 on success, 1 on failure
 */
int cell_mlocktree(struct cell *c) {

  TIMER_TIC

  /* First of all, try to lock this cell. */
  if (c->mhold || lock_trylock(&c->mlock) != 0) {
    TIMER_TOC(timer_locktree);
    return 1;
  }

  /* Did somebody hold this cell in the meantime? */
  if (c->mhold) {

    /* Unlock this cell. */
    if (lock_unlock(&c->mlock) != 0) error("Failed to unlock cell.");

    /* Admit defeat. */
    TIMER_TOC(timer_locktree);
    return 1;
  }

  /* Climb up the tree and lock/hold/unlock. */
  struct cell *finger;
  for (finger = c->parent; finger != NULL; finger = finger->parent) {

    /* Lock this cell. */
    if (lock_trylock(&finger->mlock) != 0) break;

    /* Increment the hold. */
    atomic_inc(&finger->mhold);

    /* Unlock the cell. */
    if (lock_unlock(&finger->mlock) != 0) error("Failed to unlock cell.");
  }

  /* If we reached the top of the tree, we're done. */
  if (finger == NULL) {
    TIMER_TOC(timer_locktree);
    return 0;
  }

  /* Otherwise, we hit a snag. */
  else {

    /* Undo the holds up to finger. */
    for (struct cell *finger2 = c->parent; finger2 != finger;
         finger2 = finger2->parent)
      atomic_dec(&finger2->mhold);

    /* Unlock this cell. */
    if (lock_unlock(&c->mlock) != 0) error("Failed to unlock cell.");

    /* Admit defeat. */
    TIMER_TOC(timer_locktree);
    return 1;
  }
}

/**
 * @brief Lock a cell for access to its array of #spart and hold its parents.
 *
 * @param c The #cell.
 * @return 0 on success, 1 on failure
 */
int cell_slocktree(struct cell *c) {

  TIMER_TIC

  /* First of all, try to lock this cell. */
  if (c->shold || lock_trylock(&c->slock) != 0) {
    TIMER_TOC(timer_locktree);
    return 1;
  }

  /* Did somebody hold this cell in the meantime? */
  if (c->shold) {

    /* Unlock this cell. */
    if (lock_unlock(&c->slock) != 0) error("Failed to unlock cell.");

    /* Admit defeat. */
    TIMER_TOC(timer_locktree);
    return 1;
  }

  /* Climb up the tree and lock/hold/unlock. */
  struct cell *finger;
  for (finger = c->parent; finger != NULL; finger = finger->parent) {

    /* Lock this cell. */
    if (lock_trylock(&finger->slock) != 0) break;

    /* Increment the hold. */
    atomic_inc(&finger->shold);

    /* Unlock the cell. */
    if (lock_unlock(&finger->slock) != 0) error("Failed to unlock cell.");
  }

  /* If we reached the top of the tree, we're done. */
  if (finger == NULL) {
    TIMER_TOC(timer_locktree);
    return 0;
  }

  /* Otherwise, we hit a snag. */
  else {

    /* Undo the holds up to finger. */
    for (struct cell *finger2 = c->parent; finger2 != finger;
         finger2 = finger2->parent)
      atomic_dec(&finger2->shold);

    /* Unlock this cell. */
    if (lock_unlock(&c->slock) != 0) error("Failed to unlock cell.");

    /* Admit defeat. */
    TIMER_TOC(timer_locktree);
    return 1;
  }
}

/**
 * @brief Unlock a cell's parents for access to #part array.
 *
 * @param c The #cell.
 */
void cell_unlocktree(struct cell *c) {

  TIMER_TIC

  /* First of all, try to unlock this cell. */
  if (lock_unlock(&c->lock) != 0) error("Failed to unlock cell.");

  /* Climb up the tree and unhold the parents. */
  for (struct cell *finger = c->parent; finger != NULL; finger = finger->parent)
    atomic_dec(&finger->hold);

  TIMER_TOC(timer_locktree);
}

/**
 * @brief Unlock a cell's parents for access to #gpart array.
 *
 * @param c The #cell.
 */
void cell_gunlocktree(struct cell *c) {

  TIMER_TIC

  /* First of all, try to unlock this cell. */
  if (lock_unlock(&c->glock) != 0) error("Failed to unlock cell.");

  /* Climb up the tree and unhold the parents. */
  for (struct cell *finger = c->parent; finger != NULL; finger = finger->parent)
    atomic_dec(&finger->ghold);

  TIMER_TOC(timer_locktree);
}

/**
 * @brief Unlock a cell's parents for access to its #multipole.
 *
 * @param c The #cell.
 */
void cell_munlocktree(struct cell *c) {

  TIMER_TIC

  /* First of all, try to unlock this cell. */
  if (lock_unlock(&c->mlock) != 0) error("Failed to unlock cell.");

  /* Climb up the tree and unhold the parents. */
  for (struct cell *finger = c->parent; finger != NULL; finger = finger->parent)
    atomic_dec(&finger->mhold);

  TIMER_TOC(timer_locktree);
}

/**
 * @brief Unlock a cell's parents for access to #spart array.
 *
 * @param c The #cell.
 */
void cell_sunlocktree(struct cell *c) {

  TIMER_TIC

  /* First of all, try to unlock this cell. */
  if (lock_unlock(&c->slock) != 0) error("Failed to unlock cell.");

  /* Climb up the tree and unhold the parents. */
  for (struct cell *finger = c->parent; finger != NULL; finger = finger->parent)
    atomic_dec(&finger->shold);

  TIMER_TOC(timer_locktree);
}

/**
 * @brief Sort the parts into eight bins along the given pivots.
 *
 * @param c The #cell array to be sorted.
 * @param parts_offset Offset of the cell parts array relative to the
 *        space's parts array, i.e. c->parts - s->parts.
 * @param sparts_offset Offset of the cell sparts array relative to the
 *        space's sparts array, i.e. c->sparts - s->sparts.
 * @param buff A buffer with at least max(c->count, c->gcount) entries,
 *        used for sorting indices.
 * @param sbuff A buffer with at least max(c->scount, c->gcount) entries,
 *        used for sorting indices for the sparts.
 * @param gbuff A buffer with at least max(c->count, c->gcount) entries,
 *        used for sorting indices for the gparts.
 */
void cell_split(struct cell *c, ptrdiff_t parts_offset, ptrdiff_t sparts_offset,
                struct cell_buff *buff, struct cell_buff *sbuff,
                struct cell_buff *gbuff) {

  const int count = c->count, gcount = c->gcount, scount = c->scount;
  struct part *parts = c->parts;
  struct xpart *xparts = c->xparts;
  struct gpart *gparts = c->gparts;
  struct spart *sparts = c->sparts;
  const double pivot[3] = {c->loc[0] + c->width[0] / 2,
                           c->loc[1] + c->width[1] / 2,
                           c->loc[2] + c->width[2] / 2};
  int bucket_count[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  int bucket_offset[9];

#ifdef SWIFT_DEBUG_CHECKS
  /* Check that the buffs are OK. */
  for (int k = 0; k < count; k++) {
    if (buff[k].x[0] != parts[k].x[0] || buff[k].x[1] != parts[k].x[1] ||
        buff[k].x[2] != parts[k].x[2])
      error("Inconsistent buff contents.");
  }
  for (int k = 0; k < gcount; k++) {
    if (gbuff[k].x[0] != gparts[k].x[0] || gbuff[k].x[1] != gparts[k].x[1] ||
        gbuff[k].x[2] != gparts[k].x[2])
      error("Inconsistent gbuff contents.");
  }
  for (int k = 0; k < scount; k++) {
    if (sbuff[k].x[0] != sparts[k].x[0] || sbuff[k].x[1] != sparts[k].x[1] ||
        sbuff[k].x[2] != sparts[k].x[2])
      error("Inconsistent sbuff contents.");
  }
#endif /* SWIFT_DEBUG_CHECKS */

  /* Fill the buffer with the indices. */
  for (int k = 0; k < count; k++) {
    const int bid = (buff[k].x[0] > pivot[0]) * 4 +
                    (buff[k].x[1] > pivot[1]) * 2 + (buff[k].x[2] > pivot[2]);
    bucket_count[bid]++;
    buff[k].ind = bid;
  }

  /* Set the buffer offsets. */
  bucket_offset[0] = 0;
  for (int k = 1; k <= 8; k++) {
    bucket_offset[k] = bucket_offset[k - 1] + bucket_count[k - 1];
    bucket_count[k - 1] = 0;
  }

  /* Run through the buckets, and swap particles to their correct spot. */
  for (int bucket = 0; bucket < 8; bucket++) {
    for (int k = bucket_offset[bucket] + bucket_count[bucket];
         k < bucket_offset[bucket + 1]; k++) {
      int bid = buff[k].ind;
      if (bid != bucket) {
        struct part part = parts[k];
        struct xpart xpart = xparts[k];
        struct cell_buff temp_buff = buff[k];
        while (bid != bucket) {
          int j = bucket_offset[bid] + bucket_count[bid]++;
          while (buff[j].ind == bid) {
            j++;
            bucket_count[bid]++;
          }
          memswap(&parts[j], &part, sizeof(struct part));
          memswap(&xparts[j], &xpart, sizeof(struct xpart));
          memswap(&buff[j], &temp_buff, sizeof(struct cell_buff));
          bid = temp_buff.ind;
        }
        parts[k] = part;
        xparts[k] = xpart;
        buff[k] = temp_buff;
      }
      bucket_count[bid]++;
    }
  }

  /* Store the counts and offsets. */
  for (int k = 0; k < 8; k++) {
    c->progeny[k]->count = bucket_count[k];
    c->progeny[k]->parts = &c->parts[bucket_offset[k]];
    c->progeny[k]->xparts = &c->xparts[bucket_offset[k]];
  }

  /* Re-link the gparts. */
  if (count > 0 && gcount > 0)
    part_relink_gparts_to_parts(parts, count, parts_offset);

#ifdef SWIFT_DEBUG_CHECKS
  /* Check that the buffs are OK. */
  for (int k = 1; k < count; k++) {
    if (buff[k].ind < buff[k - 1].ind) error("Buff not sorted.");
    if (buff[k].x[0] != parts[k].x[0] || buff[k].x[1] != parts[k].x[1] ||
        buff[k].x[2] != parts[k].x[2])
      error("Inconsistent buff contents (k=%i).", k);
  }

  /* Verify that _all_ the parts have been assigned to a cell. */
  for (int k = 1; k < 8; k++)
    if (&c->progeny[k - 1]->parts[c->progeny[k - 1]->count] !=
        c->progeny[k]->parts)
      error("Particle sorting failed (internal consistency).");
  if (c->progeny[0]->parts != c->parts)
    error("Particle sorting failed (left edge).");
  if (&c->progeny[7]->parts[c->progeny[7]->count] != &c->parts[count])
    error("Particle sorting failed (right edge).");

  /* Verify a few sub-cells. */
  for (int k = 0; k < c->progeny[0]->count; k++)
    if (c->progeny[0]->parts[k].x[0] > pivot[0] ||
        c->progeny[0]->parts[k].x[1] > pivot[1] ||
        c->progeny[0]->parts[k].x[2] > pivot[2])
      error("Sorting failed (progeny=0).");
  for (int k = 0; k < c->progeny[1]->count; k++)
    if (c->progeny[1]->parts[k].x[0] > pivot[0] ||
        c->progeny[1]->parts[k].x[1] > pivot[1] ||
        c->progeny[1]->parts[k].x[2] <= pivot[2])
      error("Sorting failed (progeny=1).");
  for (int k = 0; k < c->progeny[2]->count; k++)
    if (c->progeny[2]->parts[k].x[0] > pivot[0] ||
        c->progeny[2]->parts[k].x[1] <= pivot[1] ||
        c->progeny[2]->parts[k].x[2] > pivot[2])
      error("Sorting failed (progeny=2).");
  for (int k = 0; k < c->progeny[3]->count; k++)
    if (c->progeny[3]->parts[k].x[0] > pivot[0] ||
        c->progeny[3]->parts[k].x[1] <= pivot[1] ||
        c->progeny[3]->parts[k].x[2] <= pivot[2])
      error("Sorting failed (progeny=3).");
  for (int k = 0; k < c->progeny[4]->count; k++)
    if (c->progeny[4]->parts[k].x[0] <= pivot[0] ||
        c->progeny[4]->parts[k].x[1] > pivot[1] ||
        c->progeny[4]->parts[k].x[2] > pivot[2])
      error("Sorting failed (progeny=4).");
  for (int k = 0; k < c->progeny[5]->count; k++)
    if (c->progeny[5]->parts[k].x[0] <= pivot[0] ||
        c->progeny[5]->parts[k].x[1] > pivot[1] ||
        c->progeny[5]->parts[k].x[2] <= pivot[2])
      error("Sorting failed (progeny=5).");
  for (int k = 0; k < c->progeny[6]->count; k++)
    if (c->progeny[6]->parts[k].x[0] <= pivot[0] ||
        c->progeny[6]->parts[k].x[1] <= pivot[1] ||
        c->progeny[6]->parts[k].x[2] > pivot[2])
      error("Sorting failed (progeny=6).");
  for (int k = 0; k < c->progeny[7]->count; k++)
    if (c->progeny[7]->parts[k].x[0] <= pivot[0] ||
        c->progeny[7]->parts[k].x[1] <= pivot[1] ||
        c->progeny[7]->parts[k].x[2] <= pivot[2])
      error("Sorting failed (progeny=7).");
#endif

  /* Now do the same song and dance for the sparts. */
  for (int k = 0; k < 8; k++) bucket_count[k] = 0;

  /* Fill the buffer with the indices. */
  for (int k = 0; k < scount; k++) {
    const int bid = (sbuff[k].x[0] > pivot[0]) * 4 +
                    (sbuff[k].x[1] > pivot[1]) * 2 + (sbuff[k].x[2] > pivot[2]);
    bucket_count[bid]++;
    sbuff[k].ind = bid;
  }

  /* Set the buffer offsets. */
  bucket_offset[0] = 0;
  for (int k = 1; k <= 8; k++) {
    bucket_offset[k] = bucket_offset[k - 1] + bucket_count[k - 1];
    bucket_count[k - 1] = 0;
  }

  /* Run through the buckets, and swap particles to their correct spot. */
  for (int bucket = 0; bucket < 8; bucket++) {
    for (int k = bucket_offset[bucket] + bucket_count[bucket];
         k < bucket_offset[bucket + 1]; k++) {
      int bid = sbuff[k].ind;
      if (bid != bucket) {
        struct spart spart = sparts[k];
        struct cell_buff temp_buff = sbuff[k];
        while (bid != bucket) {
          int j = bucket_offset[bid] + bucket_count[bid]++;
          while (sbuff[j].ind == bid) {
            j++;
            bucket_count[bid]++;
          }
          memswap(&sparts[j], &spart, sizeof(struct spart));
          memswap(&sbuff[j], &temp_buff, sizeof(struct cell_buff));
          bid = temp_buff.ind;
        }
        sparts[k] = spart;
        sbuff[k] = temp_buff;
      }
      bucket_count[bid]++;
    }
  }

  /* Store the counts and offsets. */
  for (int k = 0; k < 8; k++) {
    c->progeny[k]->scount = bucket_count[k];
    c->progeny[k]->sparts = &c->sparts[bucket_offset[k]];
  }

  /* Re-link the gparts. */
  if (scount > 0 && gcount > 0)
    part_relink_gparts_to_sparts(sparts, scount, sparts_offset);

  /* Finally, do the same song and dance for the gparts. */
  for (int k = 0; k < 8; k++) bucket_count[k] = 0;

  /* Fill the buffer with the indices. */
  for (int k = 0; k < gcount; k++) {
    const int bid = (gbuff[k].x[0] > pivot[0]) * 4 +
                    (gbuff[k].x[1] > pivot[1]) * 2 + (gbuff[k].x[2] > pivot[2]);
    bucket_count[bid]++;
    gbuff[k].ind = bid;
  }

  /* Set the buffer offsets. */
  bucket_offset[0] = 0;
  for (int k = 1; k <= 8; k++) {
    bucket_offset[k] = bucket_offset[k - 1] + bucket_count[k - 1];
    bucket_count[k - 1] = 0;
  }

  /* Run through the buckets, and swap particles to their correct spot. */
  for (int bucket = 0; bucket < 8; bucket++) {
    for (int k = bucket_offset[bucket] + bucket_count[bucket];
         k < bucket_offset[bucket + 1]; k++) {
      int bid = gbuff[k].ind;
      if (bid != bucket) {
        struct gpart gpart = gparts[k];
        struct cell_buff temp_buff = gbuff[k];
        while (bid != bucket) {
          int j = bucket_offset[bid] + bucket_count[bid]++;
          while (gbuff[j].ind == bid) {
            j++;
            bucket_count[bid]++;
          }
          memswap(&gparts[j], &gpart, sizeof(struct gpart));
          memswap(&gbuff[j], &temp_buff, sizeof(struct cell_buff));
          bid = temp_buff.ind;
        }
        gparts[k] = gpart;
        gbuff[k] = temp_buff;
      }
      bucket_count[bid]++;
    }
  }

  /* Store the counts and offsets. */
  for (int k = 0; k < 8; k++) {
    c->progeny[k]->gcount = bucket_count[k];
    c->progeny[k]->gparts = &c->gparts[bucket_offset[k]];
  }

  /* Re-link the parts. */
  if (count > 0 && gcount > 0)
    part_relink_parts_to_gparts(gparts, gcount, parts - parts_offset);

  /* Re-link the sparts. */
  if (scount > 0 && gcount > 0)
    part_relink_sparts_to_gparts(gparts, gcount, sparts - sparts_offset);
}

/**
 * @brief Sanitizes the smoothing length values of cells by setting large
 * outliers to more sensible values.
 *
 * Each cell with <1000 part will be processed. We limit h to be the size of
 * the cell and replace 0s with a good estimate.
 *
 * @param c The cell.
 * @param treated Has the cell already been sanitized at this level ?
 */
void cell_sanitize(struct cell *c, int treated) {

  const int count = c->count;
  struct part *parts = c->parts;
  float h_max = 0.f;

  /* Treat cells will <1000 particles */
  if (count < 1000 && !treated) {

    /* Get an upper bound on h */
    const float upper_h_max = c->dmin / (1.2f * kernel_gamma);

    /* Apply it */
    for (int i = 0; i < count; ++i) {
      if (parts[i].h == 0.f || parts[i].h > upper_h_max)
        parts[i].h = upper_h_max;
    }
  }

  /* Recurse and gather the new h_max values */
  if (c->split) {

    for (int k = 0; k < 8; ++k) {
      if (c->progeny[k] != NULL) {

        /* Recurse */
        cell_sanitize(c->progeny[k], (count < 1000));

        /* And collect */
        h_max = max(h_max, c->progeny[k]->h_max);
      }
    }
  } else {

    /* Get the new value of h_max */
    for (int i = 0; i < count; ++i) h_max = max(h_max, parts[i].h);
  }

  /* Record the change */
  c->h_max = h_max;
}

/**
 * @brief Converts hydro quantities to a valid state after the initial density
 * calculation
 *
 * @param c Cell to act upon
 * @param data Unused parameter
 */
void cell_convert_hydro(struct cell *c, void *data) {

  struct part *p = c->parts;
  struct xpart *xp = c->xparts;

  for (int i = 0; i < c->count; ++i) {
    hydro_convert_quantities(&p[i], &xp[i]);
  }
}

/**
 * @brief Cleans the links in a given cell.
 *
 * @param c Cell to act upon
 * @param data Unused parameter
 */
void cell_clean_links(struct cell *c, void *data) {
  c->density = NULL;
  c->gradient = NULL;
  c->force = NULL;
  c->grav = NULL;
}

/**
 * @brief Checks that the #part in a cell are at the
 * current point in time
 *
 * Calls error() if the cell is not at the current time.
 *
 * @param c Cell to act upon
 * @param data The current time on the integer time-line
 */
void cell_check_part_drift_point(struct cell *c, void *data) {

#ifdef SWIFT_DEBUG_CHECKS

  const integertime_t ti_drift = *(integertime_t *)data;

  /* Only check local cells */
  if (c->nodeID != engine_rank) return;

  if (c->ti_old_part != ti_drift)
    error("Cell in an incorrect time-zone! c->ti_old_part=%lld ti_drift=%lld",
          c->ti_old_part, ti_drift);

  for (int i = 0; i < c->count; ++i)
    if (c->parts[i].ti_drift != ti_drift)
      error("part in an incorrect time-zone! p->ti_drift=%lld ti_drift=%lld",
            c->parts[i].ti_drift, ti_drift);
#else
  error("Calling debugging code without debugging flag activated.");
#endif
}

/**
 * @brief Checks that the #gpart and #spart in a cell are at the
 * current point in time
 *
 * Calls error() if the cell is not at the current time.
 *
 * @param c Cell to act upon
 * @param data The current time on the integer time-line
 */
void cell_check_gpart_drift_point(struct cell *c, void *data) {

#ifdef SWIFT_DEBUG_CHECKS

  const integertime_t ti_drift = *(integertime_t *)data;

  /* Only check local cells */
  if (c->nodeID != engine_rank) return;

  if (c->ti_old_gpart != ti_drift)
    error("Cell in an incorrect time-zone! c->ti_old_gpart=%lld ti_drift=%lld",
          c->ti_old_gpart, ti_drift);

  for (int i = 0; i < c->gcount; ++i)
    if (c->gparts[i].ti_drift != ti_drift)
      error("g-part in an incorrect time-zone! gp->ti_drift=%lld ti_drift=%lld",
            c->gparts[i].ti_drift, ti_drift);

  for (int i = 0; i < c->scount; ++i)
    if (c->sparts[i].ti_drift != ti_drift)
      error("s-part in an incorrect time-zone! sp->ti_drift=%lld ti_drift=%lld",
            c->sparts[i].ti_drift, ti_drift);
#else
  error("Calling debugging code without debugging flag activated.");
#endif
}

/**
 * @brief Checks that the multipole of a cell is at the current point in time
 *
 * Calls error() if the cell is not at the current time.
 *
 * @param c Cell to act upon
 * @param data The current time on the integer time-line
 */
void cell_check_multipole_drift_point(struct cell *c, void *data) {

#ifdef SWIFT_DEBUG_CHECKS

  const integertime_t ti_drift = *(integertime_t *)data;

  /* Only check local cells */
  if (c->nodeID != engine_rank) return;

  if (c->ti_old_multipole != ti_drift)
    error(
        "Cell multipole in an incorrect time-zone! c->ti_old_multipole=%lld "
        "ti_drift=%lld (depth=%d)",
        c->ti_old_multipole, ti_drift, c->depth);

#else
  error("Calling debugging code without debugging flag activated.");
#endif
}

/**
 * @brief Resets all the individual cell task counters to 0.
 *
 * Should only be used for debugging purposes.
 *
 * @param c The #cell to reset.
 */
void cell_reset_task_counters(struct cell *c) {

#ifdef SWIFT_DEBUG_CHECKS
  for (int t = 0; t < task_type_count; ++t) c->tasks_executed[t] = 0;
  for (int t = 0; t < task_subtype_count; ++t) c->subtasks_executed[t] = 0;
#else
  error("Calling debugging code without debugging flag activated.");
#endif
}

/**
 * @brief Recursively construct all the multipoles in a cell hierarchy.
 *
 * @param c The #cell.
 * @param ti_current The current integer time.
 */
void cell_make_multipoles(struct cell *c, integertime_t ti_current) {

  /* Reset everything */
  gravity_reset(c->multipole);

  if (c->split) {

    /* Compute CoM of all progenies */
    double CoM[3] = {0., 0., 0.};
    double mass = 0.;

    for (int k = 0; k < 8; ++k) {
      if (c->progeny[k] != NULL) {
        const struct gravity_tensors *m = c->progeny[k]->multipole;
        CoM[0] += m->CoM[0] * m->m_pole.M_000;
        CoM[1] += m->CoM[1] * m->m_pole.M_000;
        CoM[2] += m->CoM[2] * m->m_pole.M_000;
        mass += m->m_pole.M_000;
      }
    }
    c->multipole->CoM[0] = CoM[0] / mass;
    c->multipole->CoM[1] = CoM[1] / mass;
    c->multipole->CoM[2] = CoM[2] / mass;

    /* Now shift progeny multipoles and add them up */
    struct multipole temp;
    double r_max = 0.;
    for (int k = 0; k < 8; ++k) {
      if (c->progeny[k] != NULL) {
        const struct cell *cp = c->progeny[k];
        const struct multipole *m = &cp->multipole->m_pole;

        /* Contribution to multipole */
        gravity_M2M(&temp, m, c->multipole->CoM, cp->multipole->CoM);
        gravity_multipole_add(&c->multipole->m_pole, &temp);

        /* Upper limit of max CoM<->gpart distance */
        const double dx = c->multipole->CoM[0] - cp->multipole->CoM[0];
        const double dy = c->multipole->CoM[1] - cp->multipole->CoM[1];
        const double dz = c->multipole->CoM[2] - cp->multipole->CoM[2];
        const double r2 = dx * dx + dy * dy + dz * dz;
        r_max = max(r_max, cp->multipole->r_max + sqrt(r2));
      }
    }
    /* Alternative upper limit of max CoM<->gpart distance */
    const double dx = c->multipole->CoM[0] > c->loc[0] + c->width[0] / 2.
                          ? c->multipole->CoM[0] - c->loc[0]
                          : c->loc[0] + c->width[0] - c->multipole->CoM[0];
    const double dy = c->multipole->CoM[1] > c->loc[1] + c->width[1] / 2.
                          ? c->multipole->CoM[1] - c->loc[1]
                          : c->loc[1] + c->width[1] - c->multipole->CoM[1];
    const double dz = c->multipole->CoM[2] > c->loc[2] + c->width[2] / 2.
                          ? c->multipole->CoM[2] - c->loc[2]
                          : c->loc[2] + c->width[2] - c->multipole->CoM[2];

    /* Take minimum of both limits */
    c->multipole->r_max = min(r_max, sqrt(dx * dx + dy * dy + dz * dz));

  } else {

    if (c->gcount > 0) {
      gravity_P2M(c->multipole, c->gparts, c->gcount);
      const double dx = c->multipole->CoM[0] > c->loc[0] + c->width[0] / 2.
                            ? c->multipole->CoM[0] - c->loc[0]
                            : c->loc[0] + c->width[0] - c->multipole->CoM[0];
      const double dy = c->multipole->CoM[1] > c->loc[1] + c->width[1] / 2.
                            ? c->multipole->CoM[1] - c->loc[1]
                            : c->loc[1] + c->width[1] - c->multipole->CoM[1];
      const double dz = c->multipole->CoM[2] > c->loc[2] + c->width[2] / 2.
                            ? c->multipole->CoM[2] - c->loc[2]
                            : c->loc[2] + c->width[2] - c->multipole->CoM[2];
      c->multipole->r_max = sqrt(dx * dx + dy * dy + dz * dz);
    } else {
      gravity_multipole_init(&c->multipole->m_pole);
      c->multipole->CoM[0] = c->loc[0] + c->width[0] / 2.;
      c->multipole->CoM[1] = c->loc[1] + c->width[1] / 2.;
      c->multipole->CoM[2] = c->loc[2] + c->width[2] / 2.;
      c->multipole->r_max = 0.;
    }
  }

  c->ti_old_multipole = ti_current;
}

/**
 * @brief Computes the multi-pole brutally and compare to the
 * recursively computed one.
 *
 * @param c Cell to act upon
 * @param data Unused parameter
 */
void cell_check_multipole(struct cell *c, void *data) {

#ifdef SWIFT_DEBUG_CHECKS
  struct gravity_tensors ma;
  const double tolerance = 1e-3; /* Relative */

  return;

  /* First recurse */
  if (c->split)
    for (int k = 0; k < 8; k++)
      if (c->progeny[k] != NULL) cell_check_multipole(c->progeny[k], NULL);

  if (c->gcount > 0) {

    /* Brute-force calculation */
    gravity_P2M(&ma, c->gparts, c->gcount);

    /* Now  compare the multipole expansion */
    if (!gravity_multipole_equal(&ma, c->multipole, tolerance)) {
      message("Multipoles are not equal at depth=%d! tol=%f", c->depth,
              tolerance);
      message("Correct answer:");
      gravity_multipole_print(&ma.m_pole);
      message("Recursive multipole:");
      gravity_multipole_print(&c->multipole->m_pole);
      error("Aborting");
    }

    /* Check that the upper limit of r_max is good enough */
    if (!(c->multipole->r_max >= ma.r_max)) {
      error("Upper-limit r_max=%e too small. Should be >=%e.",
            c->multipole->r_max, ma.r_max);
    } else if (c->multipole->r_max * c->multipole->r_max >
               3. * c->width[0] * c->width[0]) {
      error("r_max=%e larger than cell diagonal %e.", c->multipole->r_max,
            sqrt(3. * c->width[0] * c->width[0]));
    }
  }
#else
  error("Calling debugging code without debugging flag activated.");
#endif
}

/**
 * @brief Frees up the memory allocated for this #cell.
 *
 * @param c The #cell.
 */
void cell_clean(struct cell *c) {

  for (int i = 0; i < 13; i++)
    if (c->sort[i] != NULL) {
      free(c->sort[i]);
      c->sort[i] = NULL;
      }

  /* Recurse */
  for (int k = 0; k < 8; k++)
    if (c->progeny[k]) cell_clean(c->progeny[k]);
}

/**
 * @brief Checks whether a given cell needs drifting or not.
 *
 * @param c the #cell.
 * @param e The #engine (holding current time information).
 *
 * @return 1 If the cell needs drifting, 0 otherwise.
 */
int cell_is_drift_needed(struct cell *c, const struct engine *e) {

  /* Do we have at least one active particle in the cell ?*/
  if (cell_is_active(c, e)) return 1;

  /* Loop over the pair tasks that involve this cell */
  for (struct link *l = c->density; l != NULL; l = l->next) {

    if (l->t->type != task_type_pair && l->t->type != task_type_sub_pair)
      continue;

    /* Is the other cell in the pair active ? */
    if ((l->t->ci == c && cell_is_active(l->t->cj, e)) ||
        (l->t->cj == c && cell_is_active(l->t->ci, e)))
      return 1;
  }

  /* No neighbouring cell has active particles. Drift not necessary */
  return 0;
}

/**
 * @brief Clear the drift flags on the given cell.
 */
void cell_clear_drift_flags(struct cell *c, void *data) {
  c->do_drift = 0;
  c->do_sub_drift = 0;
}

/**
 * @brief Activate the drifts on the given cell.
 */
void cell_activate_drift_part(struct cell *c, struct scheduler *s) {

  /* If this cell is already marked for drift, quit early. */
  if (c->do_drift) return;

  /* Mark this cell for drifting. */
  c->do_drift = 1;

  /* Set the do_sub_drifts all the way up and activate the super drift
     if this has not yet been done. */
  if (c == c->super) {
    scheduler_activate(s, c->drift_part);
  } else {
    for (struct cell *parent = c->parent;
         parent != NULL && !parent->do_sub_drift; parent = parent->parent) {
      parent->do_sub_drift = 1;
      if (parent == c->super) {
        scheduler_activate(s, parent->drift_part);
        break;
      }
    }
  }
}

/**
 * @brief Activate the sorts up a cell hierarchy.
 */

void cell_activate_sorts_up(struct cell *c, struct scheduler *s) {
  if (c == c->super) {
    scheduler_activate(s, c->sorts);
    if (c->nodeID == engine_rank) cell_activate_drift_part(c, s);
  } else {
    for (struct cell *parent = c->parent;
         parent != NULL && !parent->do_sub_sort; parent = parent->parent) {
      parent->do_sub_sort = 1;
      if (parent == c->super) {
        scheduler_activate(s, parent->sorts);
        if (parent->nodeID == engine_rank) cell_activate_drift_part(parent, s);
        break;
      }
    }
  }
}

/**
 * @brief Activate the sorts on a given cell, if needed.
 */
void cell_activate_sorts(struct cell *c, int sid, struct scheduler *s) {

  /* Do we need to re-sort? */
  if (c->dx_max_sort > space_maxreldx * c->dmin) {

    /* Climb up the tree to active the sorts in that direction */
    for (struct cell *finger = c; finger != NULL; finger = finger->parent) {
      if (finger->requires_sorts) {
        atomic_or(&finger->do_sort, finger->requires_sorts);
        cell_activate_sorts_up(finger, s);
      }
      finger->sorted = 0;
    }
  }

  /* Has this cell been sorted at all for the given sid? */
  if (!(c->sorted & (1 << sid)) || c->nodeID != engine_rank) {
    atomic_or(&c->do_sort, (1 << sid));
    cell_activate_sorts_up(c, s);
  }
}

/**
 * @brief Traverse a sub-cell task and activate the sort tasks along the way.
 */
void cell_activate_subcell_tasks(struct cell *ci, struct cell *cj,
                                 struct scheduler *s) {
  const struct engine *e = s->space->e;

  /* Store the current dx_max and h_max values. */
  ci->dx_max_old = ci->dx_max_part;
  ci->h_max_old = ci->h_max;
  if (cj != NULL) {
    cj->dx_max_old = cj->dx_max_part;
    cj->h_max_old = cj->h_max;
  }

  /* Self interaction? */
  if (cj == NULL) {
    /* Do anything? */
    if (!cell_is_active(ci, e)) return;

    /* Recurse? */
    if (cell_can_recurse_in_self_task(ci)) {

      /* Loop over all progenies and pairs of progenies */
      for (int j = 0; j < 8; j++) {
        if (ci->progeny[j] != NULL) {
          cell_activate_subcell_tasks(ci->progeny[j], NULL, s);
          for (int k = j + 1; k < 8; k++)
            if (ci->progeny[k] != NULL)
              cell_activate_subcell_tasks(ci->progeny[j], ci->progeny[k], s);
        }
      }
    } else {

      /* We have reached the bottom of the tree: activate drift */
      cell_activate_drift_part(ci, s);
    }
  }

  /* Otherwise, pair interation, recurse? */
  else if (cell_can_recurse_in_pair_task(ci) &&
           cell_can_recurse_in_pair_task(cj)) {

    /* Get the type of pair if not specified explicitly. */
    double shift[3];
    int sid = space_getsid(s->space, &ci, &cj, shift);

    /* Different types of flags. */
    switch (sid) {

      /* Regular sub-cell interactions of a single cell. */
      case 0: /* (  1 ,  1 ,  1 ) */
        if (ci->progeny[7] != NULL && cj->progeny[0] != NULL)
          cell_activate_subcell_tasks(ci->progeny[7], cj->progeny[0], s);
        break;

      case 1: /* (  1 ,  1 ,  0 ) */
        if (ci->progeny[6] != NULL && cj->progeny[0] != NULL)
          cell_activate_subcell_tasks(ci->progeny[6], cj->progeny[0], s);
        if (ci->progeny[6] != NULL && cj->progeny[1] != NULL)
          cell_activate_subcell_tasks(ci->progeny[6], cj->progeny[1], s);
        if (ci->progeny[7] != NULL && cj->progeny[0] != NULL)
          cell_activate_subcell_tasks(ci->progeny[7], cj->progeny[0], s);
        if (ci->progeny[7] != NULL && cj->progeny[1] != NULL)
          cell_activate_subcell_tasks(ci->progeny[7], cj->progeny[1], s);
        break;

      case 2: /* (  1 ,  1 , -1 ) */
        if (ci->progeny[6] != NULL && cj->progeny[1] != NULL)
          cell_activate_subcell_tasks(ci->progeny[6], cj->progeny[1], s);
        break;

      case 3: /* (  1 ,  0 ,  1 ) */
        if (ci->progeny[5] != NULL && cj->progeny[0] != NULL)
          cell_activate_subcell_tasks(ci->progeny[5], cj->progeny[0], s);
        if (ci->progeny[5] != NULL && cj->progeny[2] != NULL)
          cell_activate_subcell_tasks(ci->progeny[5], cj->progeny[2], s);
        if (ci->progeny[7] != NULL && cj->progeny[0] != NULL)
          cell_activate_subcell_tasks(ci->progeny[7], cj->progeny[0], s);
        if (ci->progeny[7] != NULL && cj->progeny[2] != NULL)
          cell_activate_subcell_tasks(ci->progeny[7], cj->progeny[2], s);
        break;

      case 4: /* (  1 ,  0 ,  0 ) */
        if (ci->progeny[4] != NULL && cj->progeny[0] != NULL)
          cell_activate_subcell_tasks(ci->progeny[4], cj->progeny[0], s);
        if (ci->progeny[4] != NULL && cj->progeny[1] != NULL)
          cell_activate_subcell_tasks(ci->progeny[4], cj->progeny[1], s);
        if (ci->progeny[4] != NULL && cj->progeny[2] != NULL)
          cell_activate_subcell_tasks(ci->progeny[4], cj->progeny[2], s);
        if (ci->progeny[4] != NULL && cj->progeny[3] != NULL)
          cell_activate_subcell_tasks(ci->progeny[4], cj->progeny[3], s);
        if (ci->progeny[5] != NULL && cj->progeny[0] != NULL)
          cell_activate_subcell_tasks(ci->progeny[5], cj->progeny[0], s);
        if (ci->progeny[5] != NULL && cj->progeny[1] != NULL)
          cell_activate_subcell_tasks(ci->progeny[5], cj->progeny[1], s);
        if (ci->progeny[5] != NULL && cj->progeny[2] != NULL)
          cell_activate_subcell_tasks(ci->progeny[5], cj->progeny[2], s);
        if (ci->progeny[5] != NULL && cj->progeny[3] != NULL)
          cell_activate_subcell_tasks(ci->progeny[5], cj->progeny[3], s);
        if (ci->progeny[6] != NULL && cj->progeny[0] != NULL)
          cell_activate_subcell_tasks(ci->progeny[6], cj->progeny[0], s);
        if (ci->progeny[6] != NULL && cj->progeny[1] != NULL)
          cell_activate_subcell_tasks(ci->progeny[6], cj->progeny[1], s);
        if (ci->progeny[6] != NULL && cj->progeny[2] != NULL)
          cell_activate_subcell_tasks(ci->progeny[6], cj->progeny[2], s);
        if (ci->progeny[6] != NULL && cj->progeny[3] != NULL)
          cell_activate_subcell_tasks(ci->progeny[6], cj->progeny[3], s);
        if (ci->progeny[7] != NULL && cj->progeny[0] != NULL)
          cell_activate_subcell_tasks(ci->progeny[7], cj->progeny[0], s);
        if (ci->progeny[7] != NULL && cj->progeny[1] != NULL)
          cell_activate_subcell_tasks(ci->progeny[7], cj->progeny[1], s);
        if (ci->progeny[7] != NULL && cj->progeny[2] != NULL)
          cell_activate_subcell_tasks(ci->progeny[7], cj->progeny[2], s);
        if (ci->progeny[7] != NULL && cj->progeny[3] != NULL)
          cell_activate_subcell_tasks(ci->progeny[7], cj->progeny[3], s);
        break;

      case 5: /* (  1 ,  0 , -1 ) */
        if (ci->progeny[4] != NULL && cj->progeny[1] != NULL)
          cell_activate_subcell_tasks(ci->progeny[4], cj->progeny[1], s);
        if (ci->progeny[4] != NULL && cj->progeny[3] != NULL)
          cell_activate_subcell_tasks(ci->progeny[4], cj->progeny[3], s);
        if (ci->progeny[6] != NULL && cj->progeny[1] != NULL)
          cell_activate_subcell_tasks(ci->progeny[6], cj->progeny[1], s);
        if (ci->progeny[6] != NULL && cj->progeny[3] != NULL)
          cell_activate_subcell_tasks(ci->progeny[6], cj->progeny[3], s);
        break;

      case 6: /* (  1 , -1 ,  1 ) */
        if (ci->progeny[5] != NULL && cj->progeny[2] != NULL)
          cell_activate_subcell_tasks(ci->progeny[5], cj->progeny[2], s);
        break;

      case 7: /* (  1 , -1 ,  0 ) */
        if (ci->progeny[4] != NULL && cj->progeny[2] != NULL)
          cell_activate_subcell_tasks(ci->progeny[4], cj->progeny[2], s);
        if (ci->progeny[4] != NULL && cj->progeny[3] != NULL)
          cell_activate_subcell_tasks(ci->progeny[4], cj->progeny[3], s);
        if (ci->progeny[5] != NULL && cj->progeny[2] != NULL)
          cell_activate_subcell_tasks(ci->progeny[5], cj->progeny[2], s);
        if (ci->progeny[5] != NULL && cj->progeny[3] != NULL)
          cell_activate_subcell_tasks(ci->progeny[5], cj->progeny[3], s);
        break;

      case 8: /* (  1 , -1 , -1 ) */
        if (ci->progeny[4] != NULL && cj->progeny[3] != NULL)
          cell_activate_subcell_tasks(ci->progeny[4], cj->progeny[3], s);
        break;

      case 9: /* (  0 ,  1 ,  1 ) */
        if (ci->progeny[3] != NULL && cj->progeny[0] != NULL)
          cell_activate_subcell_tasks(ci->progeny[3], cj->progeny[0], s);
        if (ci->progeny[3] != NULL && cj->progeny[4] != NULL)
          cell_activate_subcell_tasks(ci->progeny[3], cj->progeny[4], s);
        if (ci->progeny[7] != NULL && cj->progeny[0] != NULL)
          cell_activate_subcell_tasks(ci->progeny[7], cj->progeny[0], s);
        if (ci->progeny[7] != NULL && cj->progeny[4] != NULL)
          cell_activate_subcell_tasks(ci->progeny[7], cj->progeny[4], s);
        break;

      case 10: /* (  0 ,  1 ,  0 ) */
        if (ci->progeny[2] != NULL && cj->progeny[0] != NULL)
          cell_activate_subcell_tasks(ci->progeny[2], cj->progeny[0], s);
        if (ci->progeny[2] != NULL && cj->progeny[1] != NULL)
          cell_activate_subcell_tasks(ci->progeny[2], cj->progeny[1], s);
        if (ci->progeny[2] != NULL && cj->progeny[4] != NULL)
          cell_activate_subcell_tasks(ci->progeny[2], cj->progeny[4], s);
        if (ci->progeny[2] != NULL && cj->progeny[5] != NULL)
          cell_activate_subcell_tasks(ci->progeny[2], cj->progeny[5], s);
        if (ci->progeny[3] != NULL && cj->progeny[0] != NULL)
          cell_activate_subcell_tasks(ci->progeny[3], cj->progeny[0], s);
        if (ci->progeny[3] != NULL && cj->progeny[1] != NULL)
          cell_activate_subcell_tasks(ci->progeny[3], cj->progeny[1], s);
        if (ci->progeny[3] != NULL && cj->progeny[4] != NULL)
          cell_activate_subcell_tasks(ci->progeny[3], cj->progeny[4], s);
        if (ci->progeny[3] != NULL && cj->progeny[5] != NULL)
          cell_activate_subcell_tasks(ci->progeny[3], cj->progeny[5], s);
        if (ci->progeny[6] != NULL && cj->progeny[0] != NULL)
          cell_activate_subcell_tasks(ci->progeny[6], cj->progeny[0], s);
        if (ci->progeny[6] != NULL && cj->progeny[1] != NULL)
          cell_activate_subcell_tasks(ci->progeny[6], cj->progeny[1], s);
        if (ci->progeny[6] != NULL && cj->progeny[4] != NULL)
          cell_activate_subcell_tasks(ci->progeny[6], cj->progeny[4], s);
        if (ci->progeny[6] != NULL && cj->progeny[5] != NULL)
          cell_activate_subcell_tasks(ci->progeny[6], cj->progeny[5], s);
        if (ci->progeny[7] != NULL && cj->progeny[0] != NULL)
          cell_activate_subcell_tasks(ci->progeny[7], cj->progeny[0], s);
        if (ci->progeny[7] != NULL && cj->progeny[1] != NULL)
          cell_activate_subcell_tasks(ci->progeny[7], cj->progeny[1], s);
        if (ci->progeny[7] != NULL && cj->progeny[4] != NULL)
          cell_activate_subcell_tasks(ci->progeny[7], cj->progeny[4], s);
        if (ci->progeny[7] != NULL && cj->progeny[5] != NULL)
          cell_activate_subcell_tasks(ci->progeny[7], cj->progeny[5], s);
        break;

      case 11: /* (  0 ,  1 , -1 ) */
        if (ci->progeny[2] != NULL && cj->progeny[1] != NULL)
          cell_activate_subcell_tasks(ci->progeny[2], cj->progeny[1], s);
        if (ci->progeny[2] != NULL && cj->progeny[5] != NULL)
          cell_activate_subcell_tasks(ci->progeny[2], cj->progeny[5], s);
        if (ci->progeny[6] != NULL && cj->progeny[1] != NULL)
          cell_activate_subcell_tasks(ci->progeny[6], cj->progeny[1], s);
        if (ci->progeny[6] != NULL && cj->progeny[5] != NULL)
          cell_activate_subcell_tasks(ci->progeny[6], cj->progeny[5], s);
        break;

      case 12: /* (  0 ,  0 ,  1 ) */
        if (ci->progeny[1] != NULL && cj->progeny[0] != NULL)
          cell_activate_subcell_tasks(ci->progeny[1], cj->progeny[0], s);
        if (ci->progeny[1] != NULL && cj->progeny[2] != NULL)
          cell_activate_subcell_tasks(ci->progeny[1], cj->progeny[2], s);
        if (ci->progeny[1] != NULL && cj->progeny[4] != NULL)
          cell_activate_subcell_tasks(ci->progeny[1], cj->progeny[4], s);
        if (ci->progeny[1] != NULL && cj->progeny[6] != NULL)
          cell_activate_subcell_tasks(ci->progeny[1], cj->progeny[6], s);
        if (ci->progeny[3] != NULL && cj->progeny[0] != NULL)
          cell_activate_subcell_tasks(ci->progeny[3], cj->progeny[0], s);
        if (ci->progeny[3] != NULL && cj->progeny[2] != NULL)
          cell_activate_subcell_tasks(ci->progeny[3], cj->progeny[2], s);
        if (ci->progeny[3] != NULL && cj->progeny[4] != NULL)
          cell_activate_subcell_tasks(ci->progeny[3], cj->progeny[4], s);
        if (ci->progeny[3] != NULL && cj->progeny[6] != NULL)
          cell_activate_subcell_tasks(ci->progeny[3], cj->progeny[6], s);
        if (ci->progeny[5] != NULL && cj->progeny[0] != NULL)
          cell_activate_subcell_tasks(ci->progeny[5], cj->progeny[0], s);
        if (ci->progeny[5] != NULL && cj->progeny[2] != NULL)
          cell_activate_subcell_tasks(ci->progeny[5], cj->progeny[2], s);
        if (ci->progeny[5] != NULL && cj->progeny[4] != NULL)
          cell_activate_subcell_tasks(ci->progeny[5], cj->progeny[4], s);
        if (ci->progeny[5] != NULL && cj->progeny[6] != NULL)
          cell_activate_subcell_tasks(ci->progeny[5], cj->progeny[6], s);
        if (ci->progeny[7] != NULL && cj->progeny[0] != NULL)
          cell_activate_subcell_tasks(ci->progeny[7], cj->progeny[0], s);
        if (ci->progeny[7] != NULL && cj->progeny[2] != NULL)
          cell_activate_subcell_tasks(ci->progeny[7], cj->progeny[2], s);
        if (ci->progeny[7] != NULL && cj->progeny[4] != NULL)
          cell_activate_subcell_tasks(ci->progeny[7], cj->progeny[4], s);
        if (ci->progeny[7] != NULL && cj->progeny[6] != NULL)
          cell_activate_subcell_tasks(ci->progeny[7], cj->progeny[6], s);
        break;
    }

  }

  /* Otherwise, activate the sorts and drifts. */
  else if (cell_is_active(ci, e) || cell_is_active(cj, e)) {

    /* Get the type of pair if not specified explicitly. */
    double shift[3];
    int sid = space_getsid(s->space, &ci, &cj, shift);

    /* We are going to interact this pair, so store some values. */
    atomic_or(&ci->requires_sorts, 1 << sid);
    atomic_or(&cj->requires_sorts, 1 << sid);
    ci->dx_max_sort_old = ci->dx_max_sort;
    cj->dx_max_sort_old = cj->dx_max_sort;

    /* Activate the drifts if the cells are local. */
    if (ci->nodeID == engine_rank) cell_activate_drift_part(ci, s);
    if (cj->nodeID == engine_rank) cell_activate_drift_part(cj, s);

    /* Do we need to sort the cells? */
    cell_activate_sorts(ci, sid, s);
    cell_activate_sorts(cj, sid, s);
  }
}

/**
 * @brief Un-skips all the tasks associated with a given cell and checks
 * if the space needs to be rebuilt.
 *
 * @param c the #cell.
 * @param s the #scheduler.
 *
 * @return 1 If the space needs rebuilding. 0 otherwise.
 */
int cell_unskip_tasks(struct cell *c, struct scheduler *s) {

  struct engine *e = s->space->e;
  int rebuild = 0;

  /* Un-skip the density tasks involved with this cell. */
  for (struct link *l = c->density; l != NULL; l = l->next) {
    struct task *t = l->t;
    struct cell *ci = t->ci;
    struct cell *cj = t->cj;

    /* Only activate tasks that involve a local active cell. */
    if ((cell_is_active(ci, e) && ci->nodeID == engine_rank) ||
        (cj != NULL && cell_is_active(cj, e) && cj->nodeID == engine_rank)) {
      scheduler_activate(s, t);

      /* Set the correct sorting flags */
      if (t->type == task_type_pair) {
        /* Store some values. */
        atomic_or(&ci->requires_sorts, 1 << t->flags);
        atomic_or(&cj->requires_sorts, 1 << t->flags);
        ci->dx_max_sort_old = ci->dx_max_sort;
        cj->dx_max_sort_old = cj->dx_max_sort;

        /* Activate the drift tasks. */
        if (ci->nodeID == engine_rank) cell_activate_drift_part(ci, s);
        if (cj->nodeID == engine_rank) cell_activate_drift_part(cj, s);

        /* Check the sorts and activate them if needed. */
        cell_activate_sorts(ci, t->flags, s);
        cell_activate_sorts(cj, t->flags, s);
      }
      /* Store current values of dx_max and h_max. */
      else if (t->type == task_type_sub_pair || t->type == task_type_sub_self) {
        cell_activate_subcell_tasks(t->ci, t->cj, s);
      }
    }

    /* Only interested in pair interactions as of here. */
    if (t->type == task_type_pair || t->type == task_type_sub_pair) {

      /* Check whether there was too much particle motion, i.e. the
         cell neighbour conditions were violated. */
      if (cell_need_rebuild_for_pair(ci, cj)) rebuild = 1;

#ifdef WITH_MPI
      /* Activate the send/recv tasks. */
      if (ci->nodeID != engine_rank) {

        /* If the local cell is active, receive data from the foreign cell. */
        if (cell_is_active(cj, e)) {
          scheduler_activate(s, ci->recv_xv);
          if (cell_is_active(ci, e)) {
            scheduler_activate(s, ci->recv_rho);
#ifdef EXTRA_HYDRO_LOOP
            scheduler_activate(s, ci->recv_gradient);
#endif
          }
        }

        /* If the foreign cell is active, we want its ti_end values. */
        if (cell_is_active(ci, e)) scheduler_activate(s, ci->recv_ti);

        /* Look for the local cell cj's send tasks. */
        if (cell_is_active(ci, e)) {
          struct link *l = NULL;
          for (l = cj->send_xv; l != NULL && l->t->cj->nodeID != ci->nodeID;
               l = l->next)
            ;
          if (l == NULL) error("Missing link to send_xv task.");
          scheduler_activate(s, l->t);

          /* Drift the cell which will be sent; note that not all sent
             particles will be drifted, only those that are needed. */
          cell_activate_drift_part(cj, s);

          if (cell_is_active(cj, e)) {
            struct link *l = NULL;
            for (l = cj->send_rho; l != NULL && l->t->cj->nodeID != ci->nodeID;
                 l = l->next)
              ;
            if (l == NULL) error("Missing link to send_rho task.");
            scheduler_activate(s, l->t);

#ifdef EXTRA_HYDRO_LOOP
            for (l = cj->send_gradient;
                 l != NULL && l->t->cj->nodeID != ci->nodeID; l = l->next)
              ;
            if (l == NULL) error("Missing link to send_gradient task.");
            scheduler_activate(s, l->t);
#endif
          }
        }

        /* If the local cell is active, send its ti_end values. */
        if (cell_is_active(cj, e)) {
          struct link *l = NULL;
          for (l = cj->send_ti; l != NULL && l->t->cj->nodeID != ci->nodeID;
               l = l->next)
            ;
          if (l == NULL) error("Missing link to send_ti task.");
          scheduler_activate(s, l->t);
        }

      } else if (cj->nodeID != engine_rank) {

        /* If the local cell is active, receive data from the foreign cell. */
        if (cell_is_active(ci, e)) {
          scheduler_activate(s, cj->recv_xv);
          if (cell_is_active(cj, e)) {
            scheduler_activate(s, cj->recv_rho);
#ifdef EXTRA_HYDRO_LOOP
            scheduler_activate(s, cj->recv_gradient);
#endif
          }
        }

        /* If the foreign cell is active, we want its ti_end values. */
        if (cell_is_active(cj, e)) scheduler_activate(s, cj->recv_ti);

        /* Look for the local cell ci's send tasks. */
        if (cell_is_active(cj, e)) {
          struct link *l = NULL;
          for (l = ci->send_xv; l != NULL && l->t->cj->nodeID != cj->nodeID;
               l = l->next)
            ;
          if (l == NULL) error("Missing link to send_xv task.");
          scheduler_activate(s, l->t);

          /* Drift the cell which will be sent; note that not all sent
             particles will be drifted, only those that are needed. */
          cell_activate_drift_part(ci, s);

          if (cell_is_active(ci, e)) {

            struct link *l = NULL;
            for (l = ci->send_rho; l != NULL && l->t->cj->nodeID != cj->nodeID;
                 l = l->next)
              ;
            if (l == NULL) error("Missing link to send_rho task.");
            scheduler_activate(s, l->t);

#ifdef EXTRA_HYDRO_LOOP
            for (l = ci->send_gradient;
                 l != NULL && l->t->cj->nodeID != cj->nodeID; l = l->next)
              ;
            if (l == NULL) error("Missing link to send_gradient task.");
            scheduler_activate(s, l->t);
#endif
          }
        }

        /* If the local cell is active, send its ti_end values. */
        if (cell_is_active(ci, e)) {
          struct link *l = NULL;
          for (l = ci->send_ti; l != NULL && l->t->cj->nodeID != cj->nodeID;
               l = l->next)
            ;
          if (l == NULL) error("Missing link to send_ti task.");
          scheduler_activate(s, l->t);
        }
      }
#endif
    }
  }

  /* Unskip all the other task types. */
  if (c->nodeID == engine_rank && cell_is_active(c, e)) {

    for (struct link *l = c->gradient; l != NULL; l = l->next)
      scheduler_activate(s, l->t);
    for (struct link *l = c->force; l != NULL; l = l->next)
      scheduler_activate(s, l->t);
    for (struct link *l = c->grav; l != NULL; l = l->next)
      scheduler_activate(s, l->t);

    if (c->extra_ghost != NULL) scheduler_activate(s, c->extra_ghost);
    if (c->ghost_in != NULL) scheduler_activate(s, c->ghost_in);
    if (c->ghost_out != NULL) scheduler_activate(s, c->ghost_out);
    if (c->ghost != NULL) scheduler_activate(s, c->ghost);
    if (c->init_grav != NULL) scheduler_activate(s, c->init_grav);
    if (c->drift_gpart != NULL) scheduler_activate(s, c->drift_gpart);
    if (c->kick1 != NULL) scheduler_activate(s, c->kick1);
    if (c->kick2 != NULL) scheduler_activate(s, c->kick2);
    if (c->timestep != NULL) scheduler_activate(s, c->timestep);
    if (c->grav_ghost[0] != NULL) scheduler_activate(s, c->grav_ghost[0]);
    if (c->grav_ghost[1] != NULL) scheduler_activate(s, c->grav_ghost[1]);
    if (c->grav_down != NULL) scheduler_activate(s, c->grav_down);
    if (c->grav_long_range != NULL) scheduler_activate(s, c->grav_long_range);
    if (c->cooling != NULL) scheduler_activate(s, c->cooling);
    if (c->sourceterms != NULL) scheduler_activate(s, c->sourceterms);
  }

  return rebuild;
}

/**
 * @brief Set the super-cell pointers for all cells in a hierarchy.
 *
 * @param c The top-level #cell to play with.
 * @param super Pointer to the deepest cell with tasks in this part of the tree.
 */
void cell_set_super(struct cell *c, struct cell *super) {

  /* Are we in a cell with some kind of self/pair task ? */
  if (super == NULL && c->nr_tasks > 0) super = c;

  /* Set the super-cell */
  c->super = super;

  /* Recurse */
  if (c->split)
    for (int k = 0; k < 8; k++)
      if (c->progeny[k] != NULL) cell_set_super(c->progeny[k], super);
}

/**
 * @brief Recursively drifts the #part in a cell hierarchy.
 *
 * @param c The #cell.
 * @param e The #engine (to get ti_current).
 * @param force Drift the particles irrespective of the #cell flags.
 */
void cell_drift_part(struct cell *c, const struct engine *e, int force) {

  const float hydro_h_max = e->hydro_properties->h_max;
  const double timeBase = e->timeBase;
  const integertime_t ti_old_part = c->ti_old_part;
  const integertime_t ti_current = e->ti_current;
  struct part *const parts = c->parts;
  struct xpart *const xparts = c->xparts;

  /* Drift from the last time the cell was drifted to the current time */
  const double dt = (ti_current - ti_old_part) * timeBase;
  float dx_max = 0.f, dx2_max = 0.f;
  float dx_max_sort = 0.0f, dx2_max_sort = 0.f;
  float cell_h_max = 0.f;

  /* Drift irrespective of cell flags? */
  force |= c->do_drift;

#ifdef SWIFT_DEBUG_CHECKS
  /* Check that we only drift local cells. */
  if (c->nodeID != engine_rank) error("Drifting a foreign cell is nope.");

  /* Check that we are actually going to move forward. */
  if (ti_current < ti_old_part) error("Attempt to drift to the past");
#endif  // SWIFT_DEBUG_CHECKS

  /* Are we not in a leaf ? */
  if (c->split && (force || c->do_sub_drift)) {

    /* Loop over the progeny and collect their data. */
    for (int k = 0; k < 8; k++)
      if (c->progeny[k] != NULL) {
        struct cell *cp = c->progeny[k];

        /* Collect */
        cell_drift_part(cp, e, force);

        /* Update */
        dx_max = max(dx_max, cp->dx_max_part);
        dx_max_sort = max(dx_max_sort, cp->dx_max_sort);
        cell_h_max = max(cell_h_max, cp->h_max);
      }

    /* Store the values */
    c->h_max = cell_h_max;
    c->dx_max_part = dx_max;
    c->dx_max_sort = dx_max_sort;

    /* Update the time of the last drift */
    c->ti_old_part = ti_current;

  } else if (!c->split && force && ti_current > ti_old_part) {

    /* Loop over all the gas particles in the cell */
    const size_t nr_parts = c->count;
    for (size_t k = 0; k < nr_parts; k++) {

      /* Get a handle on the part. */
      struct part *const p = &parts[k];
      struct xpart *const xp = &xparts[k];

      /* Drift... */
      drift_part(p, xp, dt, timeBase, ti_old_part, ti_current);

      /* Limit h to within the allowed range */
      p->h = min(p->h, hydro_h_max);

      /* Compute (square of) motion since last cell construction */
      const float dx2 = xp->x_diff[0] * xp->x_diff[0] +
                        xp->x_diff[1] * xp->x_diff[1] +
                        xp->x_diff[2] * xp->x_diff[2];
      dx2_max = max(dx2_max, dx2);
      const float dx2_sort = xp->x_diff_sort[0] * xp->x_diff_sort[0] +
                             xp->x_diff_sort[1] * xp->x_diff_sort[1] +
                             xp->x_diff_sort[2] * xp->x_diff_sort[2];
      dx2_max_sort = max(dx2_max_sort, dx2_sort);

      /* Maximal smoothing length */
      cell_h_max = max(cell_h_max, p->h);

      /* Get ready for a density calculation */
      if (part_is_active(p, e)) {
        hydro_init_part(p, &e->s->hs);
      }
    }

    /* Now, get the maximal particle motion from its square */
    dx_max = sqrtf(dx2_max);
    dx_max_sort = sqrtf(dx2_max_sort);

    /* Store the values */
    c->h_max = cell_h_max;
    c->dx_max_part = dx_max;
    c->dx_max_sort = dx_max_sort;

    /* Update the time of the last drift */
    c->ti_old_part = ti_current;
  }

  /* Clear the drift flags. */
  c->do_drift = 0;
  c->do_sub_drift = 0;
}

/**
 * @brief Recursively drifts the #gpart in a cell hierarchy.
 *
 * @param c The #cell.
 * @param e The #engine (to get ti_current).
 */
void cell_drift_gpart(struct cell *c, const struct engine *e) {

  const double timeBase = e->timeBase;
  const integertime_t ti_old_gpart = c->ti_old_gpart;
  const integertime_t ti_current = e->ti_current;
  struct gpart *const gparts = c->gparts;
  struct spart *const sparts = c->sparts;

  /* Drift from the last time the cell was drifted to the current time */
  const double dt = (ti_current - ti_old_gpart) * timeBase;
  float dx_max = 0.f, dx2_max = 0.f;

  /* Check that we are actually going to move forward. */
  if (ti_current < ti_old_gpart) error("Attempt to drift to the past");

  /* Are we not in a leaf ? */
  if (c->split) {

    /* Loop over the progeny and collect their data. */
    for (int k = 0; k < 8; k++)
      if (c->progeny[k] != NULL) {
        struct cell *cp = c->progeny[k];

        /* Recurse */
        cell_drift_gpart(cp, e);

        /* Update */
        dx_max = max(dx_max, cp->dx_max_gpart);
      }

  } else if (ti_current > ti_old_gpart) {

    /* Loop over all the g-particles in the cell */
    const size_t nr_gparts = c->gcount;
    for (size_t k = 0; k < nr_gparts; k++) {

      /* Get a handle on the gpart. */
      struct gpart *const gp = &gparts[k];

      /* Drift... */
      drift_gpart(gp, dt, timeBase, ti_old_gpart, ti_current);

      /* Compute (square of) motion since last cell construction */
      const float dx2 = gp->x_diff[0] * gp->x_diff[0] +
                        gp->x_diff[1] * gp->x_diff[1] +
                        gp->x_diff[2] * gp->x_diff[2];
      dx2_max = max(dx2_max, dx2);

      /* Init gravity force fields. */
      if (gpart_is_active(gp, e)) {
        gravity_init_gpart(gp);
      }
    }

    /* Loop over all the star particles in the cell */
    const size_t nr_sparts = c->scount;
    for (size_t k = 0; k < nr_sparts; k++) {

      /* Get a handle on the spart. */
      struct spart *const sp = &sparts[k];

      /* Drift... */
      drift_spart(sp, dt, timeBase, ti_old_gpart, ti_current);

      /* Note: no need to compute dx_max as all spart have a gpart */
    }

    /* Now, get the maximal particle motion from its square */
    dx_max = sqrtf(dx2_max);

  } else {

    dx_max = c->dx_max_gpart;
  }

  /* Store the values */
  c->dx_max_gpart = dx_max;

  /* Update the time of the last drift */
  c->ti_old_gpart = ti_current;
}

/**
 * @brief Recursively drifts all multipoles in a cell hierarchy.
 *
 * @param c The #cell.
 * @param e The #engine (to get ti_current).
 */
void cell_drift_all_multipoles(struct cell *c, const struct engine *e) {

  const double timeBase = e->timeBase;
  const integertime_t ti_old_multipole = c->ti_old_multipole;
  const integertime_t ti_current = e->ti_current;

  /* Drift from the last time the cell was drifted to the current time */
  const double dt = (ti_current - ti_old_multipole) * timeBase;

  /* Check that we are actually going to move forward. */
  if (ti_current < ti_old_multipole) error("Attempt to drift to the past");

  /* Drift the multipole */
  if (ti_current > ti_old_multipole) gravity_drift(c->multipole, dt);

  /* Are we not in a leaf ? */
  if (c->split) {

    /* Loop over the progeny and recurse. */
    for (int k = 0; k < 8; k++)
      if (c->progeny[k] != NULL) cell_drift_all_multipoles(c->progeny[k], e);
  }

  /* Update the time of the last drift */
  c->ti_old_multipole = ti_current;
}

/**
 * @brief Drifts the multipole of a cell to the current time.
 *
 * Only drifts the multipole at this level. Multipoles deeper in the
 * tree are not updated.
 *
 * @param c The #cell.
 * @param e The #engine (to get ti_current).
 */
void cell_drift_multipole(struct cell *c, const struct engine *e) {

  const double timeBase = e->timeBase;
  const integertime_t ti_old_multipole = c->ti_old_multipole;
  const integertime_t ti_current = e->ti_current;

  /* Drift from the last time the cell was drifted to the current time */
  const double dt = (ti_current - ti_old_multipole) * timeBase;

  /* Check that we are actually going to move forward. */
  if (ti_current < ti_old_multipole) error("Attempt to drift to the past");

  if (ti_current > ti_old_multipole) gravity_drift(c->multipole, dt);

  /* Update the time of the last drift */
  c->ti_old_multipole = ti_current;
}

/**
 * @brief Recursively checks that all particles in a cell have a time-step
 */
void cell_check_timesteps(struct cell *c) {
#ifdef SWIFT_DEBUG_CHECKS

  if (c->ti_end_min == 0 && c->nr_tasks > 0)
    error("Cell without assigned time-step");

  if (c->split) {
    for (int k = 0; k < 8; ++k)
      if (c->progeny[k] != NULL) cell_check_timesteps(c->progeny[k]);
  } else {

    if (c->nodeID == engine_rank)
      for (int i = 0; i < c->count; ++i)
        if (c->parts[i].time_bin == 0)
          error("Particle without assigned time-bin");
  }
#else
  error("Calling debugging code without debugging flag activated.");
#endif
}
