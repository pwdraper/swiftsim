/*******************************************************************************
 * This file is part of SWIFT.
 * Copyright (c) 2016 James Willis (james.s.willis@durham.ac.uk)
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

/* This object's header. */
#include "runner_doiact_vec.h"

/* Local headers. */
#include "active.h"

#if defined(WITH_VECTORIZATION) && (defined(GADGET2_SPH) || defined(MINIMAL_SPH))

static const vector kernel_gamma2_vec = FILL_VEC(kernel_gamma2);

/**
 * @brief Compute the vector remainder interactions from the secondary cache.
 *
 * @param int_cache (return) secondary cache of interactions between two
 * particles.
 * @param icount Interaction count.
 * @param sum_cache (return) Cache of #vector holding the cumulative sum of updates on pi.
 * @param params Input parameters for SPH scheme.
 * interactions have been performed, should be a multiple of the vector length.
 */
__attribute__((always_inline)) INLINE static void calcRemInteractions(
    struct c2_cache *const int_cache, int *icount, struct update_cache_density *sum_cache,
    const struct input_params_density *params) {

  mask_t int_mask, int_mask2;

  /* Work out the number of remainder interactions and pad secondary cache. */
  int icount_padded = *icount;
  int rem = *icount % (NUM_VEC_PROC * VEC_SIZE);
  if (rem != 0) {
    int pad = (NUM_VEC_PROC * VEC_SIZE) - rem;
    icount_padded += pad;

    /* Initialise masks to true. */
    vec_init_mask_true(int_mask);
    vec_init_mask_true(int_mask2);

    /* Pad secondary cache so that there are no contributions in the interaction
     * function. */
    pad_c2_cache(int_cache, *icount, icount_padded);
    
    /* Zero parts of mask that represent the padded values.*/
    if (pad < VEC_SIZE) {
      vec_pad_mask(int_mask2, pad);
    } else {
      vec_pad_mask(int_mask, VEC_SIZE - rem);
      vec_zero_mask(int_mask2);
    }

    /* Perform remainder interaction and remove remainder from aligned
     * interaction count. */
    *icount -= rem;
    runner_iact_nonsym_2_vec_density(
        int_cache, *icount,
        params, sum_cache, int_mask,
        int_mask2, 1);

  }
}

/**
 * @brief Left-packs the values needed by an interaction into the secondary
 * cache (Supports AVX, AVX2 and AVX512 instruction sets).
 *
 * @param mask Contains which particles need to interact.
 * @param pjd Index of the particle to store into.
 * @param v_r2 #vector of the separation between two particles squared.
 * @param v_dx #vector of the x separation between two particles.
 * @param v_dy #vector of the y separation between two particles.
 * @param v_dz #vector of the z separation between two particles.
 * @param cell_cache cache of all particles in the cell.
 * @param int_cache (return) secondary cache of interactions between two
 * particles.
 * @param icount Interaction count.
 * @param sum_cache (return) Cache of #vector holding the cumulative sum of updates on pi.
 * @param params Input parameters for SPH scheme.
 */
__attribute__((always_inline)) INLINE static void storeInteractions(
    const int mask, const int pjd, vector *v_r2, vector *v_dx, vector *v_dy,
    vector *v_dz, const struct cache *const cell_cache,
    struct c2_cache *const int_cache, int *icount, struct update_cache_density *sum_cache,
    const struct input_params_density *params) {

  /* Left-pack values needed into the secondary cache using the interaction mask. */
  left_pack_c2_cache(mask, pjd, v_r2, v_dx, v_dy, v_dz, cell_cache, int_cache, icount);
  
  /* Flush the c2 cache if it has reached capacity. */
  if (*icount >= (C2_CACHE_SIZE - (NUM_VEC_PROC * VEC_SIZE))) {

    /* Peform remainder interactions. */
    calcRemInteractions(int_cache, icount, sum_cache, params);

    mask_t int_mask, int_mask2;
    vec_init_mask_true(int_mask);
    vec_init_mask_true(int_mask2);

    /* Perform interactions. */
    for (int j = 0; j < *icount; j += (NUM_VEC_PROC * VEC_SIZE)) {
      runner_iact_nonsym_2_vec_density(
          int_cache, j, params, sum_cache, int_mask, int_mask2, 0);
    }

    /* Reset interaction count. */
    *icount = 0;
  }
}

/**
 * @brief Populates the arrays max_index_i and max_index_j with the maximum
 * indices of
 * particles into their neighbouring cells. Also finds the first pi that
 * interacts with any particle in cj and the last pj that interacts with any
 * particle in ci.
 *
 * @param ci #cell pointer to ci
 * @param cj #cell pointer to cj
 * @param sort_i #entry array for particle distance in ci
 * @param sort_j #entry array for particle distance in cj
 * @param dx_max maximum particle movement allowed in cell
 * @param rshift cutoff shift
 * @param hi_max Maximal smoothing length in cell ci
 * @param hj_max Maximal smoothing length in cell cj
 * @param di_max Maximal position on the axis that can interact in cell ci
 * @param dj_min Minimal position on the axis that can interact in cell ci
 * @param max_index_i array to hold the maximum distances of pi particles into
 * #cell cj
 * @param max_index_j array to hold the maximum distances of pj particles into
 * #cell cj
 * @param init_pi first pi to interact with a pj particle
 * @param init_pj last pj to interact with a pi particle
 * @param max_active_bin The largest time-bin active during this step.
 * @param active_ci Is any particle in cell ci active?
 * @param active_cj Is any particle in cell cj active?
 */
__attribute__((always_inline)) INLINE static void populate_max_index_density(
    const struct cell *ci, const struct cell *cj,
    const struct entry *restrict sort_i, const struct entry *restrict sort_j,
    const float dx_max, const float rshift, const double hi_max,
    const double hj_max, const double di_max, const double dj_min,
    int *max_index_i, int *max_index_j, int *init_pi, int *init_pj,
    const timebin_t max_active_bin, const int active_ci, const int active_cj) {

  const struct part *restrict parts_i = ci->parts;
  const struct part *restrict parts_j = cj->parts;

  int first_pi = 0, last_pj = cj->count - 1;
  int temp, active_id;

  /* Only populate max_index array for local actve cells. */
  if (active_ci) {

    /* Find the leftmost active particle in cell i that interacts with any
     * particle in cell j. */
    first_pi = ci->count;
    active_id = first_pi - 1;
    while (first_pi > 0 && sort_i[first_pi - 1].d + dx_max + hi_max > dj_min) {
      first_pi--;
      /* Store the index of the particle if it is active. */
      if (part_is_active_no_debug(&parts_i[sort_i[first_pi].i], max_active_bin))
        active_id = first_pi;
    }

    /* Set the first active pi in range of any particle in cell j. */
    first_pi = active_id;

    /* Find the maximum index into cell j for each particle in range in cell i.
     */
    if (first_pi < ci->count) {

      /* Start from the first particle in cell j. */
      temp = 0;

      const struct part *pi = &parts_i[sort_i[first_pi].i];
      const float first_di =
          sort_i[first_pi].d + pi->h * kernel_gamma + dx_max - rshift;

      /* Loop through particles in cell j until they are not in range of pi.
       * Make sure that temp stays between 0 and cj->count - 1.*/
      while (temp < cj->count - 1 && first_di > sort_j[temp].d) temp++;

      max_index_i[first_pi] = temp;

      /* Populate max_index_i for remaining particles that are within range. */
      for (int i = first_pi + 1; i < ci->count; i++) {
        temp = max_index_i[i - 1];
        pi = &parts_i[sort_i[i].i];

        const float di = sort_i[i].d + pi->h * kernel_gamma + dx_max - rshift;

        /* Make sure that temp stays between 0 and cj->count - 1.*/
        while (temp < cj->count - 1 && di > sort_j[temp].d) temp++;

        max_index_i[i] = temp;
      }
    } else {
      /* Make sure that max index is set to first particle in cj.*/
      max_index_i[ci->count - 1] = 0;
    }
  } else {
    /* Make sure that foreign cells are only read into the cache if the local
     * cell requires it.
     * Also ensure that it does not require any particles from cj. */
    first_pi = ci->count - 1;
    max_index_i[ci->count - 1] = 0;
  }

  /* Only populate max_index array for local actve cells. */
  if (active_cj) {
    /* Find the rightmost active particle in cell j that interacts with any
     * particle in cell i. */
    last_pj = -1;
    active_id = last_pj;
    while (last_pj < cj->count &&
           sort_j[last_pj + 1].d - hj_max - dx_max < di_max) {
      last_pj++;
      /* Store the index of the particle if it is active. */
      if (part_is_active_no_debug(&parts_j[sort_j[last_pj].i], max_active_bin))
        active_id = last_pj;
    }

    /* Set the last active pj in range of any particle in cell i. */
    last_pj = active_id;

    /* Find the maximum index into cell i for each particle in range in cell j.
     */
    if (last_pj >= 0) {

      /* Start from the last particle in cell i. */
      temp = ci->count - 1;

      const struct part *pj = &parts_j[sort_j[last_pj].i];
      const float last_dj =
          sort_j[last_pj].d - dx_max - pj->h * kernel_gamma + rshift;

      /* Loop through particles in cell i until they are not in range of pj. */
      while (temp > 0 && last_dj < sort_i[temp].d) temp--;

      max_index_j[last_pj] = temp;

      /* Populate max_index_j for remaining particles that are within range. */
      for (int i = last_pj - 1; i >= 0; i--) {
        temp = max_index_j[i + 1];
        pj = &parts_j[sort_j[i].i];
        const float dj = sort_j[i].d - dx_max - (pj->h * kernel_gamma) + rshift;

        while (temp > 0 && dj < sort_i[temp].d) temp--;

        max_index_j[i] = temp;
      }
    } else {
      /* Make sure that max index is set to last particle in ci.*/
      max_index_j[0] = ci->count - 1;
    }
  } else {
    /* Make sure that foreign cells are only read into the cache if the local
     * cell requires it.
     * Also ensure that it does not require any particles from ci. */
    last_pj = 0;
    max_index_j[0] = ci->count - 1;
  }

  *init_pi = first_pi;
  *init_pj = last_pj;
}

#if defined(WITH_VECTORIZATION)
/**
 * @brief Populates the arrays max_index_i and max_index_j with the maximum
 * indices of
 * particles into their neighbouring cells. Also finds the first pi that
 * interacts with any particle in cj and the last pj that interacts with any
 * particle in ci.
 *
 * @param ci #cell pointer to ci
 * @param cj #cell pointer to cj
 * @param sort_i #entry array for particle distance in ci
 * @param sort_j #entry array for particle distance in cj
 * @param dx_max maximum particle movement allowed in cell
 * @param rshift cutoff shift
 * @param hi_max_raw Maximal smoothing length in cell ci
 * @param hj_max_raw Maximal smoothing length in cell cj
 * @param h_max Maximal smoothing length in both cells scaled by kernel_gamma
 * @param di_max Maximal position on the axis that can interact in cell ci
 * @param dj_min Minimal position on the axis that can interact in cell ci
 * @param max_index_i array to hold the maximum distances of pi particles into
 * #cell cj
 * @param max_index_j array to hold the maximum distances of pj particles into
 * #cell cj
 * @param init_pi first pi to interact with a pj particle
 * @param init_pj last pj to interact with a pi particle
 * @param max_active_bin The largest time-bin active during this step.
 * @param active_ci Is any particle in cell ci active?
 * @param active_cj Is any particle in cell cj active?
 */
__attribute__((always_inline)) INLINE static void populate_max_index_force(
    const struct cell *ci, const struct cell *cj,
    const struct entry *restrict sort_i, const struct entry *restrict sort_j,
    const float dx_max, const float rshift, const double hi_max_raw,
    const double hj_max_raw, const double h_max, const double di_max,
    const double dj_min, int *max_index_i, int *max_index_j, int *init_pi,
    int *init_pj, const timebin_t max_active_bin, const int active_ci,
    const int active_cj) {

  const struct part *restrict parts_i = ci->parts;
  const struct part *restrict parts_j = cj->parts;

  int first_pi = 0, last_pj = cj->count - 1;
  int temp, active_id;

  /* Only populate max_index array for local actve cells. */
  if (active_ci) {

    /* Find the leftmost active particle in cell i that interacts with any
     * particle in cell j. */
    first_pi = ci->count;
    active_id = first_pi - 1;
    while (first_pi > 0 && sort_i[first_pi - 1].d + dx_max + h_max > dj_min) {
      first_pi--;
      /* Store the index of the particle if it is active. */
      if (part_is_active_no_debug(&parts_i[sort_i[first_pi].i], max_active_bin))
        active_id = first_pi;
    }

    /* Set the first active pi in range of any particle in cell j. */
    first_pi = active_id;

    /* Find the maximum index into cell j for each particle in range in cell i.
     */
    if (first_pi < ci->count) {

      /* Start from the first particle in cell j. */
      temp = 0;

      const struct part *pi = &parts_i[sort_i[first_pi].i];
      const float first_di = sort_i[first_pi].d +
                             max(pi->h, hj_max_raw) * kernel_gamma + dx_max -
                             rshift;

      /* Loop through particles in cell j until they are not in range of pi.
       * Make sure that temp stays between 0 and cj->count - 1.*/
      while (temp < cj->count - 1 && first_di > sort_j[temp].d) temp++;

      max_index_i[first_pi] = temp;

      /* Populate max_index_i for remaining particles that are within range. */
      for (int i = first_pi + 1; i < ci->count; i++) {
        temp = max_index_i[i - 1];
        pi = &parts_i[sort_i[i].i];

        const float di = sort_i[i].d + max(pi->h, hj_max_raw) * kernel_gamma +
                         dx_max - rshift;

        /* Make sure that temp stays between 0 and cj->count - 1.*/
        while (temp < cj->count - 1 && di > sort_j[temp].d) temp++;

        max_index_i[i] = temp;
      }
    } else {
      /* Make sure that max index is set to first particle in cj.*/
      max_index_i[ci->count - 1] = 0;
    }
  } else {
    /* Make sure that foreign cells are only read into the cache if the local
     * cell requires it.
     * Also ensure that it does not require any particles from cj. */
    first_pi = ci->count - 1;
    max_index_i[ci->count - 1] = 0;
  }

  /* Only populate max_index array for local actve cells. */
  if (active_cj) {
    /* Find the rightmost active particle in cell j that interacts with any
     * particle in cell i. */
    last_pj = -1;
    active_id = last_pj;
    while (last_pj < cj->count &&
           sort_j[last_pj + 1].d - h_max - dx_max < di_max) {
      last_pj++;
      /* Store the index of the particle if it is active. */
      if (part_is_active_no_debug(&parts_j[sort_j[last_pj].i], max_active_bin))
        active_id = last_pj;
    }

    /* Set the last active pj in range of any particle in cell i. */
    last_pj = active_id;

    /* Find the maximum index into cell i for each particle in range in cell j.
     */
    if (last_pj >= 0) {

      /* Start from the last particle in cell i. */
      temp = ci->count - 1;

      const struct part *pj = &parts_j[sort_j[last_pj].i];
      const float last_dj = sort_j[last_pj].d - dx_max -
                            max(pj->h, hi_max_raw) * kernel_gamma + rshift;

      /* Loop through particles in cell i until they are not in range of pj. */
      while (temp > 0 && last_dj < sort_i[temp].d) temp--;

      max_index_j[last_pj] = temp;

      /* Populate max_index_j for remaining particles that are within range. */
      for (int i = last_pj - 1; i >= 0; i--) {
        temp = max_index_j[i + 1];
        pj = &parts_j[sort_j[i].i];

        const float dj = sort_j[i].d - dx_max -
                         (max(pj->h, hi_max_raw) * kernel_gamma) + rshift;

        while (temp > 0 && dj < sort_i[temp].d) temp--;

        max_index_j[i] = temp;
      }
    } else {
      /* Make sure that max index is set to last particle in ci.*/
      max_index_j[0] = ci->count - 1;
    }
  } else {
    /* Make sure that foreign cells are only read into the cache if the local
     * cell requires it.
     * Also ensure that it does not require any particles from ci. */
    last_pj = 0;
    max_index_j[0] = ci->count - 1;
  }

  *init_pi = first_pi;
  *init_pj = last_pj;
}

/**
 * @brief Populates the array max_index_i with the maximum
 * index of
 * particles into the neighbouring cell. Also finds the first/last pj that
 * interacts with any particle in ci.
 *
 * @param count_i The number of particles in ci.
 * @param count_j The number of particles in cj.
 * @param parts_i The #part to interact with @c cj.
 * @param ind The list of indices of particles in @c ci to interact with.
 * @param total_ci_shift The shift vector to apply to the particles in ci.
 * @param dxj Maximum particle movement allowed in cell cj.
 * @param di_shift_correction The correction to di after the particles have been
 * shifted to the frame of cell ci.
 * @param runner_shift_x The runner_shift in the x direction.
 * @param runner_shift_y The runner_shift in the y direction.
 * @param runner_shift_z The runner_shift in the z direction.
 * @param sort_j #entry array for particle distance in cj
 * @param max_index_i array to hold the maximum distances of pi particles into
 * #cell cj
 * @param flipped Flag to check whether the cells have been flipped or not.
 * @return first_pj/last_pj first or last pj to interact with any particle in ci
 * depending whether the cells have been flipped or not.
 */
__attribute__((always_inline)) INLINE static int populate_max_index_subset(
    const int count_i, const int count_j, struct part *restrict parts_i,
    int *restrict ind, const double *total_ci_shift, const float dxj,
    const double di_shift_correction, const double runner_shift_x,
    const double runner_shift_y, const double runner_shift_z,
    const struct entry *restrict sort_j, int *max_index_i, const int flipped) {

  /* The cell is on the right so read the particles
   * into the cache from the start of the cell. */
  if (!flipped) {

    /* Find the rightmost particle in cell j that interacts with any
     * particle in cell i. */
    int last_pj = 0;

    for (int pid = 0; pid < count_i; pid++) {
      struct part *restrict pi = &parts_i[ind[pid]];
      const float pix = pi->x[0] - total_ci_shift[0];
      const float piy = pi->x[1] - total_ci_shift[1];
      const float piz = pi->x[2] - total_ci_shift[2];
      const float hi = pi->h;

      const double di = hi * kernel_gamma + dxj + pix * runner_shift_x +
                        piy * runner_shift_y + piz * runner_shift_z +
                        di_shift_correction;

      for (int pjd = last_pj; pjd < count_j && sort_j[pjd].d < di; pjd++)
        last_pj++;

      max_index_i[pid] = last_pj;
    }
    return last_pj;
  }
  /* The cell is on the left so read the particles
   * into the cache from the end of the cell. */
  else {

    int first_pj = count_j - 1;

    for (int pid = 0; pid < count_i; pid++) {
      struct part *restrict pi = &parts_i[ind[pid]];
      const float pix = pi->x[0] - total_ci_shift[0];
      const float piy = pi->x[1] - total_ci_shift[1];
      const float piz = pi->x[2] - total_ci_shift[2];
      const float hi = pi->h;

      const double di = -hi * kernel_gamma - dxj + pix * runner_shift_x +
                        piy * runner_shift_y + piz * runner_shift_z +
                        di_shift_correction;

      for (int pjd = first_pj; pjd > 0 && di < sort_j[pjd].d; pjd--) first_pj--;

      max_index_i[pid] = first_pj;
    }
    return first_pj;
  }
}

#endif

#endif /* WITH_VECTORIZATION && GADGET2_SPH */

/**
 * @brief Compute the cell self-interaction (non-symmetric) using vector
 * intrinsics with one particle pi at a time.
 *
 * @param r The #runner.
 * @param c The #cell.
 */
void runner_doself1_density_vec(struct runner *r, struct cell *restrict c) {

#if defined(WITH_VECTORIZATION) && (defined(GADGET2_SPH) || defined(MINIMAL_SPH))

  /* Get some local variables */
  const struct engine *e = r->e;

  TIMER_TIC;

  /* Anything to do here? */
  if (!cell_is_active_hydro(c, e)) return;

  if (!cell_are_part_drifted(c, e)) error("Interacting undrifted cell.");

  /* Get some local variables */
  const timebin_t max_active_bin = e->max_active_bin;
  struct part *restrict parts = c->parts;
  const int count = c->count;

#ifdef SWIFT_DEBUG_CHECKS
  for (int i = 0; i < count; i++) {
    /* Check that particles have been drifted to the current time */
    if (parts[i].ti_drift != e->ti_current)
      error("Particle pi not drifted to current time");
  }
#endif

  /* Get the particle cache from the runner and re-allocate
   * the cache if it is not big enough for the cell. */
  struct cache *restrict cell_cache = &r->ci_cache;
  if (cell_cache->count < count) cache_init(cell_cache, count);

  /* Read the particles from the cell and store them locally in the cache. */
  cache_read_particles(c, cell_cache, count);

  /* Create secondary cache to store particle interactions. */
  struct c2_cache int_cache;

  /* Loop over the particles in the cell. */
  for (int pid = 0; pid < count; pid++) {

    /* Get a pointer to the ith particle. */
    struct part *restrict pi = &parts[pid];

    /* Is the ith particle active? */
    if (!part_is_active_no_debug(pi, max_active_bin)) continue;

    /* Get the smoothing length, hi. */
    const float hi = cell_cache->h[pid];
    const float hig2 = hi * hi * kernel_gamma2;

    /* Fill particle pi vectors. */
    const vector v_pix = vector_set1(cell_cache->x[pid]);
    const vector v_piy = vector_set1(cell_cache->y[pid]);
    const vector v_piz = vector_set1(cell_cache->z[pid]);
    const vector v_hig2 = vector_set1(hig2);

    struct input_params_density params;
    populate_input_params_density_cache(cell_cache, pid, &params);
    
    struct update_cache_density sum_cache;
    update_cache_density_init(&sum_cache);

    /* The number of interactions for pi. */
    int icount = 0;

    /* Find all of particle pi's interacions and store needed values in the
     * secondary cache.*/
    for (int pjd = 0; pjd < count; pjd += (NUM_VEC_PROC * VEC_SIZE)) {

      /* Load 2 sets of vectors from the particle cache. */
      const vector v_pjx = vector_load(&cell_cache->x[pjd]);
      const vector v_pjy = vector_load(&cell_cache->y[pjd]);
      const vector v_pjz = vector_load(&cell_cache->z[pjd]);

      const vector v_pjx2 = vector_load(&cell_cache->x[pjd + VEC_SIZE]);
      const vector v_pjy2 = vector_load(&cell_cache->y[pjd + VEC_SIZE]);
      const vector v_pjz2 = vector_load(&cell_cache->z[pjd + VEC_SIZE]);

      /* Compute the pairwise distance. */
      vector v_dx, v_dy, v_dz, v_r2;
      vector v_dx_2, v_dy_2, v_dz_2, v_r2_2;

      v_dx.v = vec_sub(v_pix.v, v_pjx.v);
      v_dx_2.v = vec_sub(v_pix.v, v_pjx2.v);
      v_dy.v = vec_sub(v_piy.v, v_pjy.v);
      v_dy_2.v = vec_sub(v_piy.v, v_pjy2.v);
      v_dz.v = vec_sub(v_piz.v, v_pjz.v);
      v_dz_2.v = vec_sub(v_piz.v, v_pjz2.v);

      v_r2.v = vec_mul(v_dx.v, v_dx.v);
      v_r2_2.v = vec_mul(v_dx_2.v, v_dx_2.v);
      v_r2.v = vec_fma(v_dy.v, v_dy.v, v_r2.v);
      v_r2_2.v = vec_fma(v_dy_2.v, v_dy_2.v, v_r2_2.v);
      v_r2.v = vec_fma(v_dz.v, v_dz.v, v_r2.v);
      v_r2_2.v = vec_fma(v_dz_2.v, v_dz_2.v, v_r2_2.v);

      /* Form a mask from r2 < hig2 and r2 > 0.*/
      mask_t v_doi_mask, v_doi_mask_self_check, v_doi_mask2,
          v_doi_mask2_self_check;
      
      /* Form r2 > 0 mask and r2 < hig2 mask. */
      vec_create_mask(v_doi_mask_self_check, vec_cmp_gt(v_r2.v, vec_setzero()));
      vec_create_mask(v_doi_mask, vec_cmp_lt(v_r2.v, v_hig2.v));

      /* Form r2 > 0 mask and r2 < hig2 mask. */
      vec_create_mask(v_doi_mask2_self_check,
                      vec_cmp_gt(v_r2_2.v, vec_setzero()));
      vec_create_mask(v_doi_mask2, vec_cmp_lt(v_r2_2.v, v_hig2.v));

      /* Combine two masks and form integer masks. */
      const int doi_mask = vec_is_mask_true(v_doi_mask) &
                           vec_is_mask_true(v_doi_mask_self_check);
      const int doi_mask2 = vec_is_mask_true(v_doi_mask2) &
                            vec_is_mask_true(v_doi_mask2_self_check);

#ifdef DEBUG_INTERACTIONS_SPH
      for (int bit_index = 0; bit_index < VEC_SIZE; bit_index++) {
        if (doi_mask & (1 << bit_index)) {
          if (pi->num_ngb_density < MAX_NUM_OF_NEIGHBOURS)
            pi->ids_ngbs_density[pi->num_ngb_density] =
                parts[pjd + bit_index].id;
          ++pi->num_ngb_density;
        }

        if (doi_mask2 & (1 << bit_index)) {
          if (pi->num_ngb_density < MAX_NUM_OF_NEIGHBOURS)
            pi->ids_ngbs_density[pi->num_ngb_density] =
                parts[pjd + VEC_SIZE + bit_index].id;
          ++pi->num_ngb_density;
        }
      }
#endif

      /* If there are any interactions left pack interaction values into c2
       * cache. */
      if (doi_mask) {
        storeInteractions(doi_mask, pjd, &v_r2, &v_dx, &v_dy, &v_dz, cell_cache,
                          &int_cache, &icount, &sum_cache, &params);
      }
      if (doi_mask2) {
        storeInteractions(doi_mask2, pjd + VEC_SIZE, &v_r2_2, &v_dx_2, &v_dy_2,
                          &v_dz_2, cell_cache, &int_cache, &icount, &sum_cache, &params);
      }
    }

    /* Perform padded vector remainder interactions if any are present. */
    calcRemInteractions(&int_cache, &icount, &sum_cache, &params);

    /* Initialise masks to true in case remainder interactions have been
     * performed. */
    mask_t int_mask, int_mask2;
    vec_init_mask_true(int_mask);
    vec_init_mask_true(int_mask2);

    /* Perform interaction with 2 vectors. */
    for (int pjd = 0; pjd < icount; pjd += (NUM_VEC_PROC * VEC_SIZE)) {
      runner_iact_nonsym_2_vec_density(&int_cache, pjd, &params, &sum_cache, int_mask, int_mask2, 0);
    }

    /* Perform horizontal adds on vector sums and store result in particle pi. */
    update_density_particle(pi, &sum_cache);
    
    /* Reset interaction count. */
    icount = 0;
  } /* loop over all particles. */

  TIMER_TOC(timer_doself_density);

#else

  error("Incorrectly calling vectorized Gadget-2 functions!");

#endif /* WITH_VECTORIZATION */
}

/**
 * @brief Compute the interactions between a cell pair, but only for the
 *      given indices in ci. (Vectorised)
 *
 * @param r The #runner.
 * @param c The first #cell.
 * @param parts The #part to interact.
 * @param ind The list of indices of particles in @c c to interact with.
 * @param pi_count The number of particles in @c ind.
 */
void runner_doself_subset_density_vec(struct runner *r, struct cell *restrict c,
                                      struct part *restrict parts,
                                      int *restrict ind, int pi_count) {

#if defined(WITH_VECTORIZATION)

  const int count = c->count;

  TIMER_TIC;

  /* Get the particle cache from the runner and re-allocate
   * the cache if it is not big enough for the cell. */
  struct cache *restrict cell_cache = &r->ci_cache;

  if (cell_cache->count < count) cache_init(cell_cache, count);

  /* Read the particles from the cell and store them locally in the cache. */
  cache_read_particles(c, cell_cache, count);

  /* Create secondary cache to store particle interactions. */
  struct c2_cache int_cache;

  /* Loop over the subset of particles in the parts that need updating. */
  for (int pid = 0; pid < pi_count; pid++) {

    /* Get a pointer to the ith particle. */
    struct part *pi = &parts[ind[pid]];

#ifdef SWIFT_DEBUG_CHECKS
    const struct engine *e = r->e;
    if (!part_is_active(pi, e)) error("Inactive particle in subset function!");
#endif

    /* Get the smoothing length, hi. */
    const float hi = pi->h;
    const float hig2 = hi * hi * kernel_gamma2;

    /* Fill particle pi vectors. */
    const vector v_pix = vector_set1(pi->x[0] - c->loc[0]);
    const vector v_piy = vector_set1(pi->x[1] - c->loc[1]);
    const vector v_piz = vector_set1(pi->x[2] - c->loc[2]);
    const vector v_hig2 = vector_set1(hig2);

    struct input_params_density params;
    populate_input_params_density(pi, &params);

    /* Reset cumulative sums of update vectors. */
    struct update_cache_density sum_cache;
    update_cache_density_init(&sum_cache);
    
    /* Pad cache if there is a serial remainder. */
    int count_align = count;
    const int rem = count % (NUM_VEC_PROC * VEC_SIZE);
    if (rem != 0) {
      const int pad = (NUM_VEC_PROC * VEC_SIZE) - rem;

      count_align += pad;

      /* Set positions to the same as particle pi so when the r2 > 0 mask is
       * applied these extra contributions are masked out.*/
      for (int i = count; i < count_align; i++) {
        cell_cache->x[i] = v_pix.f[0];
        cell_cache->y[i] = v_piy.f[0];
        cell_cache->z[i] = v_piz.f[0];
      }
    }

    /* The number of interactions for pi and the padded version of it to
     * make it a multiple of VEC_SIZE. */
    int icount = 0;

    /* Find all of particle pi's interacions and store needed values in the
     * secondary cache.*/
    for (int pjd = 0; pjd < count; pjd += (NUM_VEC_PROC * VEC_SIZE)) {

      /* Load 2 sets of vectors from the particle cache. */
      const vector v_pjx = vector_load(&cell_cache->x[pjd]);
      const vector v_pjy = vector_load(&cell_cache->y[pjd]);
      const vector v_pjz = vector_load(&cell_cache->z[pjd]);

      const vector v_pjx2 = vector_load(&cell_cache->x[pjd + VEC_SIZE]);
      const vector v_pjy2 = vector_load(&cell_cache->y[pjd + VEC_SIZE]);
      const vector v_pjz2 = vector_load(&cell_cache->z[pjd + VEC_SIZE]);

      /* Compute the pairwise distance. */
      vector v_dx, v_dy, v_dz, v_r2;
      vector v_dx_2, v_dy_2, v_dz_2, v_r2_2;

      /* p_i - p_j */
      v_dx.v = vec_sub(v_pix.v, v_pjx.v);
      v_dx_2.v = vec_sub(v_pix.v, v_pjx2.v);
      v_dy.v = vec_sub(v_piy.v, v_pjy.v);
      v_dy_2.v = vec_sub(v_piy.v, v_pjy2.v);
      v_dz.v = vec_sub(v_piz.v, v_pjz.v);
      v_dz_2.v = vec_sub(v_piz.v, v_pjz2.v);

      /* r2 = dx^2 + dy^2 + dz^2 */
      v_r2.v = vec_mul(v_dx.v, v_dx.v);
      v_r2_2.v = vec_mul(v_dx_2.v, v_dx_2.v);
      v_r2.v = vec_fma(v_dy.v, v_dy.v, v_r2.v);
      v_r2_2.v = vec_fma(v_dy_2.v, v_dy_2.v, v_r2_2.v);
      v_r2.v = vec_fma(v_dz.v, v_dz.v, v_r2.v);
      v_r2_2.v = vec_fma(v_dz_2.v, v_dz_2.v, v_r2_2.v);

      /* Form a mask from r2 < hig2 and r2 > 0.*/
      mask_t v_doi_mask, v_doi_mask_self_check, v_doi_mask2,
          v_doi_mask2_self_check;

      /* Form r2 > 0 mask and r2 < hig2 mask. */
      vec_create_mask(v_doi_mask_self_check, vec_cmp_gt(v_r2.v, vec_setzero()));
      vec_create_mask(v_doi_mask, vec_cmp_lt(v_r2.v, v_hig2.v));

      /* Form r2 > 0 mask and r2 < hig2 mask. */
      vec_create_mask(v_doi_mask2_self_check,
                      vec_cmp_gt(v_r2_2.v, vec_setzero()));
      vec_create_mask(v_doi_mask2, vec_cmp_lt(v_r2_2.v, v_hig2.v));

      /* Combine two masks and form integer masks. */
      const int doi_mask = vec_is_mask_true(v_doi_mask) &
                           vec_is_mask_true(v_doi_mask_self_check);
      const int doi_mask2 = vec_is_mask_true(v_doi_mask2) &
                            vec_is_mask_true(v_doi_mask2_self_check);

#ifdef DEBUG_INTERACTIONS_SPH
      struct part *restrict parts_i = c->parts;
      for (int bit_index = 0; bit_index < VEC_SIZE; bit_index++) {
        if (doi_mask & (1 << bit_index)) {
          if (pi->num_ngb_density < MAX_NUM_OF_NEIGHBOURS)
            pi->ids_ngbs_density[pi->num_ngb_density] =
                parts_i[pjd + bit_index].id;
          ++pi->num_ngb_density;
        }

        if (doi_mask2 & (1 << bit_index)) {
          if (pi->num_ngb_density < MAX_NUM_OF_NEIGHBOURS)
            pi->ids_ngbs_density[pi->num_ngb_density] =
                parts_i[pjd + VEC_SIZE + bit_index].id;
          ++pi->num_ngb_density;
        }
      }
#endif

      /* If there are any interactions left pack interaction values into c2
       * cache. */
      if (doi_mask) {
        storeInteractions(doi_mask, pjd, &v_r2, &v_dx, &v_dy, &v_dz, cell_cache,
                          &int_cache, &icount, &sum_cache, &params);
      }
      if (doi_mask2) {
        storeInteractions(doi_mask2, pjd + VEC_SIZE, &v_r2_2, &v_dx_2, &v_dy_2,
                          &v_dz_2, cell_cache, &int_cache, &icount, &sum_cache,
                          &params);
      }
    }

    /* Perform padded vector remainder interactions if any are present. */
    calcRemInteractions(&int_cache, &icount, &sum_cache, &params);

    /* Initialise masks to true in case remainder interactions have been
     * performed. */
    mask_t int_mask, int_mask2;
    vec_init_mask_true(int_mask);
    vec_init_mask_true(int_mask2);

    /* Perform interaction with 2 vectors. */
    for (int pjd = 0; pjd < icount; pjd += (NUM_VEC_PROC * VEC_SIZE)) {
      runner_iact_nonsym_2_vec_density(
          &int_cache, pjd, &params, &sum_cache, int_mask, int_mask2, 0);
    }

    /* Perform horizontal adds on vector sums and store result in particle pi.*/
    update_density_particle(pi, &sum_cache);

    /* Reset interaction count. */
    icount = 0;
  } /* loop over all particles. */

  TIMER_TOC(timer_doself_subset);

#else

  error("Incorrectly calling vectorized Gadget-2 functions!");

#endif /* WITH_VECTORIZATION */
}

/**
 * @brief Compute the force cell self-interaction (non-symmetric) using vector
 * intrinsics with one particle pi at a time.
 *
 * @param r The #runner.
 * @param c The #cell.
 */
void runner_doself2_force_vec(struct runner *r, struct cell *restrict c) {

#if defined(WITH_VECTORIZATION)

  const struct engine *e = r->e;
  const timebin_t max_active_bin = e->max_active_bin;
  struct part *restrict parts = c->parts;
  const int count = c->count;

  TIMER_TIC;

  /* Early abort? */
  if (!cell_is_active_hydro(c, e)) return;

  if (!cell_are_part_drifted(c, e)) error("Interacting undrifted cell.");

#ifdef SWIFT_DEBUG_CHECKS
  for (int i = 0; i < count; i++) {
    /* Check that particles have been drifted to the current time */
    if (parts[i].ti_drift != e->ti_current)
      error("Particle pi not drifted to current time");
  }
#endif

  /* Get the particle cache from the runner and re-allocate
   * the cache if it is not big enough for the cell. */
  struct cache *restrict cell_cache = &r->ci_cache;

  if (cell_cache->count < count) cache_init(cell_cache, count);

  /* Read the particles from the cell and store them locally in the cache. */
  cache_read_force_particles(c, cell_cache, count);

  /* Loop over the particles in the cell. */
  for (int pid = 0; pid < count; pid++) {

    /* Get a pointer to the ith particle. */
    struct part *restrict pi = &parts[pid];

    /* Is the ith particle active? */
    if (!part_is_active_no_debug(pi, max_active_bin)) continue;

    const float hi = cell_cache->h[pid];
    const float hig2 = hi * hi * kernel_gamma2;

    /* Fill particle pi vectors. */
    const vector v_pix = vector_set1(cell_cache->x[pid]);
    const vector v_piy = vector_set1(cell_cache->y[pid]);
    const vector v_piz = vector_set1(cell_cache->z[pid]);
    const vector v_hig2 = vector_set1(hig2);

    struct input_params_force params;
    populate_input_params_force_cache(cell_cache, pid, &params);
    
    /* Reset cumulative sums of update vectors. */
    struct update_cache_force sum_cache;
    update_cache_force_init(&sum_cache);

    /* Find all of particle pi's interacions and store needed values in the
     * secondary cache.*/
    for (int pjd = 0; pjd < count; pjd += VEC_SIZE) {

      /* Load 1 set of vectors from the particle cache. */
      vector hjg2;
      const vector v_pjx = vector_load(&cell_cache->x[pjd]);
      const vector v_pjy = vector_load(&cell_cache->y[pjd]);
      const vector v_pjz = vector_load(&cell_cache->z[pjd]);
      const vector hj = vector_load(&cell_cache->h[pjd]);
      hjg2.v = vec_mul(vec_mul(hj.v, hj.v), kernel_gamma2_vec.v);

      /* Compute the pairwise distance. */
      vector v_dx, v_dy, v_dz, v_r2;
      v_dx.v = vec_sub(v_pix.v, v_pjx.v);
      v_dy.v = vec_sub(v_piy.v, v_pjy.v);
      v_dz.v = vec_sub(v_piz.v, v_pjz.v);

      v_r2.v = vec_mul(v_dx.v, v_dx.v);
      v_r2.v = vec_fma(v_dy.v, v_dy.v, v_r2.v);
      v_r2.v = vec_fma(v_dz.v, v_dz.v, v_r2.v);

      /* Form r2 > 0 mask, r2 < max(hig2,hjg2) mask. */
      mask_t v_doi_mask, v_doi_mask_self_check;

      /* Form r2 > 0 mask.*/
      vec_create_mask(v_doi_mask_self_check, vec_cmp_gt(v_r2.v, vec_setzero()));

      /* Form a mask from r2 < max(hig2,hjg2) mask. */
      vector v_h2;
      v_h2.v = vec_fmax(v_hig2.v, hjg2.v);
      vec_create_mask(v_doi_mask, vec_cmp_lt(v_r2.v, v_h2.v));

      /* Combine both masks. */
      vec_combine_masks(v_doi_mask, v_doi_mask_self_check);

#ifdef DEBUG_INTERACTIONS_SPH
      for (int bit_index = 0; bit_index < VEC_SIZE; bit_index++) {
        if (vec_is_mask_true(v_doi_mask) & (1 << bit_index)) {
          if (pi->num_ngb_force < MAX_NUM_OF_NEIGHBOURS)
            pi->ids_ngbs_force[pi->num_ngb_force] = parts[pjd + bit_index].id;
          ++pi->num_ngb_force;
        }
      }
#endif

      /* If there are any interactions perform them. */
      if (vec_is_mask_true(v_doi_mask)) {
        vector v_hj_inv = vec_reciprocal(hj);

        /* To stop floating point exceptions for when particle separations are
         * 0. */
        v_r2.v = vec_add(v_r2.v, vec_set1(FLT_MIN));

        runner_iact_nonsym_1_vec_force(
            &v_r2, &v_dx, &v_dy, &v_dz, &params, cell_cache, pjd,
            v_hj_inv, &sum_cache, v_doi_mask);
      }

    } /* Loop over all other particles. */

    /* Perform horizontal adds on vector sums and store result in pj. */
    update_force_particle(pi, &sum_cache);
    
  } /* loop over all particles. */

  TIMER_TOC(timer_doself_force);

#else

  error("Incorrectly calling vectorized Gadget-2 functions!");

#endif /* WITH_VECTORIZATION */
}

/**
 * @brief Compute the density interactions between a cell pair (non-symmetric)
 * using vector intrinsics.
 *
 * @param r The #runner.
 * @param ci The first #cell.
 * @param cj The second #cell.
 * @param sid The direction of the pair
 * @param shift The shift vector to apply to the particles in ci.
 */
void runner_dopair1_density_vec(struct runner *r, struct cell *ci,
                                struct cell *cj, const int sid,
                                const double *shift) {

#if defined(WITH_VECTORIZATION) && (defined(GADGET2_SPH) || defined(MINIMAL_SPH))

  const struct engine *restrict e = r->e;
  const timebin_t max_active_bin = e->max_active_bin;

  TIMER_TIC;

  /* Check whether cells are local to the node. */
  const int ci_local = (ci->nodeID == e->nodeID);
  const int cj_local = (cj->nodeID == e->nodeID);

  /* Get the cutoff shift. */
  double rshift = 0.0;
  for (int k = 0; k < 3; k++) rshift += shift[k] * runner_shift[sid][k];

  /* Pick-out the sorted lists. */
  const struct entry *restrict sort_i = ci->sort[sid];
  const struct entry *restrict sort_j = cj->sort[sid];

  /* Get some other useful values. */
  const int count_i = ci->count;
  const int count_j = cj->count;
  const double hi_max = ci->h_max * kernel_gamma - rshift;
  const double hj_max = cj->h_max * kernel_gamma;
  struct part *restrict parts_i = ci->parts;
  struct part *restrict parts_j = cj->parts;
  const double di_max = sort_i[count_i - 1].d - rshift;
  const double dj_min = sort_j[0].d;
  const float dx_max = (ci->dx_max_sort + cj->dx_max_sort);
  const int active_ci = cell_is_active_hydro(ci, e) && ci_local;
  const int active_cj = cell_is_active_hydro(cj, e) && cj_local;

#ifdef SWIFT_DEBUG_CHECKS
  /* Check that particles have been drifted to the current time */
  for (int pid = 0; pid < count_i; pid++)
    if (parts_i[pid].ti_drift != e->ti_current)
      error("Particle pi not drifted to current time");
  for (int pjd = 0; pjd < count_j; pjd++)
    if (parts_j[pjd].ti_drift != e->ti_current)
      error("Particle pj not drifted to current time");
#endif

  /* Count number of particles that are in range and active*/
  int numActive = 0;

  if (active_ci) {
    for (int pid = count_i - 1;
         pid >= 0 && sort_i[pid].d + hi_max + dx_max > dj_min; pid--) {
      const struct part *restrict pi = &parts_i[sort_i[pid].i];
      if (part_is_active_no_debug(pi, max_active_bin)) {
        numActive++;
        break;
      }
    }
  }

  if (!numActive && active_cj) {
    for (int pjd = 0; pjd < count_j && sort_j[pjd].d - hj_max - dx_max < di_max;
         pjd++) {
      const struct part *restrict pj = &parts_j[sort_j[pjd].i];
      if (part_is_active_no_debug(pj, max_active_bin)) {
        numActive++;
        break;
      }
    }
  }

  /* Return if there are no active particles within range */
  if (numActive == 0) return;

  /* Get both particle caches from the runner and re-allocate
   * them if they are not big enough for the cells. */
  struct cache *restrict ci_cache = &r->ci_cache;
  struct cache *restrict cj_cache = &r->cj_cache;
  if (ci_cache->count < count_i) cache_init(ci_cache, count_i);
  if (cj_cache->count < count_j) cache_init(cj_cache, count_j);

  /* Get a direct pointer to the index arrays */
  int first_pi, last_pj;
  swift_declare_aligned_ptr(int, max_index_i, r->ci_cache.max_index,
                            SWIFT_CACHE_ALIGNMENT);
  swift_declare_aligned_ptr(int, max_index_j, r->cj_cache.max_index,
                            SWIFT_CACHE_ALIGNMENT);

  /* Find particles maximum index into cj, max_index_i[] and ci, max_index_j[].
   * Also find the first pi that interacts with any particle in cj and the last
   * pj that interacts with any particle in ci. */
  populate_max_index_density(ci, cj, sort_i, sort_j, dx_max, rshift, hi_max,
                             hj_max, di_max, dj_min, max_index_i, max_index_j,
                             &first_pi, &last_pj, max_active_bin, active_ci,
                             active_cj);

  /* Limits of the outer loops. */
  const int first_pi_loop = first_pi;
  const int last_pj_loop_end = last_pj + 1;

  /* Take the max/min of both values calculated to work out how many particles
   * to read into the cache. */
  last_pj = max(last_pj, max_index_i[count_i - 1]);
  first_pi = min(first_pi, max_index_j[0]);

  /* Read the required particles into the two caches. */
  cache_read_two_partial_cells_sorted(ci, cj, ci_cache, cj_cache, sort_i,
                                      sort_j, shift, &first_pi, &last_pj);

  /* Get the number of particles read into the ci cache. */
  const int ci_cache_count = count_i - first_pi;

  if (active_ci) {

    /* Loop over the parts in ci until nothing is within range in cj. */
    for (int pid = count_i - 1; pid >= first_pi_loop; pid--) {

      /* Get a hold of the ith part in ci. */
      struct part *restrict pi = &parts_i[sort_i[pid].i];
      if (!part_is_active_no_debug(pi, max_active_bin)) continue;

      /* Set the cache index. */
      const int ci_cache_idx = pid - first_pi;

      /* Skip this particle if no particle in cj is within range of it. */
      const float hi = ci_cache->h[ci_cache_idx];
      const double di_test =
          sort_i[pid].d + hi * kernel_gamma + dx_max - rshift;
      if (di_test < dj_min) continue;

      /* Determine the exit iteration of the interaction loop. */
      const int exit_iteration_end = max_index_i[pid] + 1;

      /* Fill particle pi vectors. */
      const vector v_pix = vector_set1(ci_cache->x[ci_cache_idx]);
      const vector v_piy = vector_set1(ci_cache->y[ci_cache_idx]);
      const vector v_piz = vector_set1(ci_cache->z[ci_cache_idx]);
      
      const float hig2 = hi * hi * kernel_gamma2;
      const vector v_hig2 = vector_set1(hig2);

      struct input_params_density params;
      populate_input_params_density_cache(ci_cache, ci_cache_idx, &params);
      
      /* Reset cumulative sums of update vectors. */
      struct update_cache_density sum_cache;
      update_cache_density_init(&sum_cache);

      /* Loop over the parts in cj. Making sure to perform an iteration of the
       * loop even if exit_iteration_align is zero and there is only one
       * particle to interact with.*/
      for (int pjd = 0; pjd < exit_iteration_end; pjd += VEC_SIZE) {

        /* Get the cache index to the jth particle. */
        const int cj_cache_idx = pjd;

        vector v_dx, v_dy, v_dz, v_r2;

#ifdef SWIFT_DEBUG_CHECKS
        if (cj_cache_idx % VEC_SIZE != 0 || cj_cache_idx < 0 ||
            cj_cache_idx + (VEC_SIZE - 1) > (last_pj + 1 + VEC_SIZE)) {
          error("Unaligned read!!! cj_cache_idx=%d, last_pj=%d", cj_cache_idx,
                last_pj);
        }
#endif

        /* Load 1 set of vectors from the particle cache. */
        const vector v_pjx = vector_load(&cj_cache->x[cj_cache_idx]);
        const vector v_pjy = vector_load(&cj_cache->y[cj_cache_idx]);
        const vector v_pjz = vector_load(&cj_cache->z[cj_cache_idx]);

        /* Compute the pairwise distance. */
        v_dx.v = vec_sub(v_pix.v, v_pjx.v);
        v_dy.v = vec_sub(v_piy.v, v_pjy.v);
        v_dz.v = vec_sub(v_piz.v, v_pjz.v);

        v_r2.v = vec_mul(v_dx.v, v_dx.v);
        v_r2.v = vec_fma(v_dy.v, v_dy.v, v_r2.v);
        v_r2.v = vec_fma(v_dz.v, v_dz.v, v_r2.v);

        mask_t v_doi_mask;

        /* Form r2 < hig2 mask. */
        vec_create_mask(v_doi_mask, vec_cmp_lt(v_r2.v, v_hig2.v));

#ifdef DEBUG_INTERACTIONS_SPH
        for (int bit_index = 0; bit_index < VEC_SIZE; bit_index++) {
          if (vec_is_mask_true(v_doi_mask) & (1 << bit_index)) {
            if (pi->num_ngb_density < MAX_NUM_OF_NEIGHBOURS)
              pi->ids_ngbs_density[pi->num_ngb_density] =
                  parts_j[sort_j[pjd + bit_index].i].id;
            ++pi->num_ngb_density;
          }
        }
#endif

        /* If there are any interactions perform them. */
        if (vec_is_mask_true(v_doi_mask))
          runner_iact_nonsym_1_vec_density(
              &v_r2, &v_dx, &v_dy, &v_dz, &params,
              cj_cache, cj_cache_idx, &sum_cache, v_doi_mask);

      } /* loop over the parts in cj. */

      /* Perform horizontal adds on vector sums and store result in particle pi.*/
      update_density_particle(pi, &sum_cache);
      
    } /* loop over the parts in ci. */
  }

  if (active_cj) {

    /* Loop over the parts in cj until nothing is within range in ci. */
    for (int pjd = 0; pjd < last_pj_loop_end; pjd++) {

      /* Get a hold of the jth part in cj. */
      struct part *restrict pj = &parts_j[sort_j[pjd].i];
      if (!part_is_active_no_debug(pj, max_active_bin)) continue;

      /* Set the cache index. */
      const int cj_cache_idx = pjd;

      /* Skip this particle if no particle in ci is within range of it. */
      const float hj = cj_cache->h[cj_cache_idx];
      const double dj_test = sort_j[pjd].d - hj * kernel_gamma - dx_max;
      if (dj_test > di_max) continue;

      /* Determine the exit iteration of the interaction loop. */
      const int exit_iteration = max_index_j[pjd];

      /* Fill particle pi vectors. */
      const vector v_pjx = vector_set1(cj_cache->x[cj_cache_idx]);
      const vector v_pjy = vector_set1(cj_cache->y[cj_cache_idx]);
      const vector v_pjz = vector_set1(cj_cache->z[cj_cache_idx]);
      
      const float hjg2 = hj * hj * kernel_gamma2;
      const vector v_hjg2 = vector_set1(hjg2);

      struct input_params_density params;
      populate_input_params_density_cache(cj_cache, cj_cache_idx, &params);
      
      /* Reset cumulative sums of update vectors. */
      struct update_cache_density sum_cache;
      update_cache_density_init(&sum_cache);
      
      /* Convert exit iteration to cache indices. */
      int exit_iteration_align = exit_iteration - first_pi;

      /* Pad the exit iteration align so cache reads are aligned. */
      const int rem = exit_iteration_align % VEC_SIZE;
      if (exit_iteration_align < VEC_SIZE) {
        exit_iteration_align = 0;
      } else
        exit_iteration_align -= rem;

      /* Loop over the parts in ci. */
      for (int ci_cache_idx = exit_iteration_align;
           ci_cache_idx < ci_cache_count; ci_cache_idx += VEC_SIZE) {

#ifdef SWIFT_DEBUG_CHECKS
        if (ci_cache_idx % VEC_SIZE != 0 || ci_cache_idx < 0 ||
            ci_cache_idx + (VEC_SIZE - 1) > (count_i - first_pi + VEC_SIZE)) {
          error(
              "Unaligned read!!! ci_cache_idx=%d, first_pi=%d, "
              "count_i=%d",
              ci_cache_idx, first_pi, count_i);
        }
#endif

        vector v_dx, v_dy, v_dz, v_r2;

        /* Load 2 sets of vectors from the particle cache. */
        const vector v_pix = vector_load(&ci_cache->x[ci_cache_idx]);
        const vector v_piy = vector_load(&ci_cache->y[ci_cache_idx]);
        const vector v_piz = vector_load(&ci_cache->z[ci_cache_idx]);

        /* Compute the pairwise distance. */
        v_dx.v = vec_sub(v_pjx.v, v_pix.v);
        v_dy.v = vec_sub(v_pjy.v, v_piy.v);
        v_dz.v = vec_sub(v_pjz.v, v_piz.v);

        v_r2.v = vec_mul(v_dx.v, v_dx.v);
        v_r2.v = vec_fma(v_dy.v, v_dy.v, v_r2.v);
        v_r2.v = vec_fma(v_dz.v, v_dz.v, v_r2.v);

        mask_t v_doj_mask;

        /* Form r2 < hig2 mask. */
        vec_create_mask(v_doj_mask, vec_cmp_lt(v_r2.v, v_hjg2.v));

#ifdef DEBUG_INTERACTIONS_SPH
        for (int bit_index = 0; bit_index < VEC_SIZE; bit_index++) {
          if (vec_is_mask_true(v_doj_mask) & (1 << bit_index)) {
            if (pj->num_ngb_density < MAX_NUM_OF_NEIGHBOURS)
              pj->ids_ngbs_density[pj->num_ngb_density] =
                  parts_i[sort_i[ci_cache_idx + first_pi + bit_index].i].id;
            ++pj->num_ngb_density;
          }
        }
#endif

        /* If there are any interactions perform them. */
        if (vec_is_mask_true(v_doj_mask))
          runner_iact_nonsym_1_vec_density(
              &v_r2, &v_dx, &v_dy, &v_dz, &params,
              ci_cache, ci_cache_idx, &sum_cache, v_doj_mask);

      } /* loop over the parts in ci. */

      /* Perform horizontal adds on vector sums and store result in particle pj.*/
      update_density_particle(pj, &sum_cache);
    
    } /* loop over the parts in cj. */
  }

  TIMER_TOC(timer_dopair_density);

#else

  error("Incorrectly calling vectorized Gadget-2 functions!");

#endif /* WITH_VECTORIZATION */
}

/**
 * @brief Compute the interactions between a cell pair, but only for the
 *      given indices in ci. (Vectorised)
 *
 * @param r The #runner.
 * @param ci The first #cell.
 * @param parts_i The #part to interact with @c cj.
 * @param ind The list of indices of particles in @c ci to interact with.
 * @param count The number of particles in @c ind.
 * @param cj The second #cell.
 * @param sid The direction of the pair.
 * @param flipped Flag to check whether the cells have been flipped or not.
 * @param shift The shift vector to apply to the particles in ci.
 */
void runner_dopair_subset_density_vec(struct runner *r,
                                      struct cell *restrict ci,
                                      struct part *restrict parts_i,
                                      int *restrict ind, int count,
                                      struct cell *restrict cj, const int sid,
                                      const int flipped, const double *shift) {

#ifdef WITH_VECTORIZATION

  TIMER_TIC;

  const int count_j = cj->count;

  /* Pick-out the sorted lists. */
  const struct entry *restrict sort_j = cj->sort[sid];
  const float dxj = cj->dx_max_sort;

  /* Get both particle caches from the runner and re-allocate
   * them if they are not big enough for the cells. */
  struct cache *restrict cj_cache = &r->cj_cache;

  if (cj_cache->count < count_j) cache_init(cj_cache, count_j);

  /* Pull each runner_shift from memory. */
  const double runner_shift_x = runner_shift[sid][0];
  const double runner_shift_y = runner_shift[sid][1];
  const double runner_shift_z = runner_shift[sid][2];

  const double total_ci_shift[3] = {
      ci->loc[0] + shift[0], ci->loc[1] + shift[1], ci->loc[2] + shift[2]};

  /* Calculate the correction to di after the particles have been shifted to the
   * frame of cell ci. */
  const double di_shift_correction = ci->loc[0] * runner_shift_x +
                                     ci->loc[1] * runner_shift_y +
                                     ci->loc[2] * runner_shift_z;

  double rshift = 0.0;
  for (int k = 0; k < 3; k++) rshift += shift[k] * runner_shift[sid][k];

  int *restrict max_index_i SWIFT_CACHE_ALIGN;
  max_index_i = r->ci_cache.max_index;

  /* Parts are on the left? */
  if (!flipped) {

    int last_pj = populate_max_index_subset(
        count, count_j, parts_i, ind, total_ci_shift, dxj, di_shift_correction,
        runner_shift_x, runner_shift_y, runner_shift_z, sort_j, max_index_i, 0);

    /* Read the particles from the cell and store them locally in the cache. */
    cache_read_particles_subset(cj, cj_cache, sort_j, 0, &last_pj, ci->loc, 0);

    const double dj_min = sort_j[0].d;

    /* Loop over the parts_i. */
    for (int pid = 0; pid < count; pid++) {

      /* Get a hold of the ith part in ci. */
      struct part *restrict pi = &parts_i[ind[pid]];
      const float pix = pi->x[0] - total_ci_shift[0];
      const float piy = pi->x[1] - total_ci_shift[1];
      const float piz = pi->x[2] - total_ci_shift[2];
      const float hi = pi->h;

      /* Skip this particle if no particle in cj is within range of it. */
      const double di = hi * kernel_gamma + dxj + pix * runner_shift_x +
                        piy * runner_shift_y + piz * runner_shift_z +
                        di_shift_correction;
      if (di < dj_min) continue;

      /* Fill particle pi vectors. */
      const vector v_pix = vector_set1(pix);
      const vector v_piy = vector_set1(piy);
      const vector v_piz = vector_set1(piz);
      
      const float hig2 = hi * hi * kernel_gamma2;
      const vector v_hig2 = vector_set1(hig2);
      
      struct input_params_density params;
      populate_input_params_density(pi, &params);
      
      /* Reset cumulative sums of update vectors. */
      struct update_cache_density sum_cache;
      update_cache_density_init(&sum_cache);

      int exit_iteration_end = max_index_i[pid] + 1;

      /* Loop over the parts in cj. */
      for (int pjd = 0; pjd < exit_iteration_end; pjd += VEC_SIZE) {

        /* Get the cache index to the jth particle. */
        const int cj_cache_idx = pjd;

        vector v_dx, v_dy, v_dz, v_r2;

        /* Load 1 set of vectors from the particle cache. */
        const vector v_pjx = vector_load(&cj_cache->x[cj_cache_idx]);
        const vector v_pjy = vector_load(&cj_cache->y[cj_cache_idx]);
        const vector v_pjz = vector_load(&cj_cache->z[cj_cache_idx]);

        /* Compute the pairwise distance. */
        v_dx.v = vec_sub(v_pix.v, v_pjx.v);
        v_dy.v = vec_sub(v_piy.v, v_pjy.v);
        v_dz.v = vec_sub(v_piz.v, v_pjz.v);

        v_r2.v = vec_mul(v_dx.v, v_dx.v);
        v_r2.v = vec_fma(v_dy.v, v_dy.v, v_r2.v);
        v_r2.v = vec_fma(v_dz.v, v_dz.v, v_r2.v);

        /* Form r2 < hig2 mask. */
        mask_t v_doi_mask;
        vec_create_mask(v_doi_mask, vec_cmp_lt(v_r2.v, v_hig2.v));

#ifdef DEBUG_INTERACTIONS_SPH
        struct part *restrict parts_j = cj->parts;
        for (int bit_index = 0; bit_index < VEC_SIZE; bit_index++) {
          if (vec_is_mask_true(v_doi_mask) & (1 << bit_index)) {
            if (pi->num_ngb_density < MAX_NUM_OF_NEIGHBOURS) {
              pi->ids_ngbs_density[pi->num_ngb_density] =
                  parts_j[sort_j[pjd + bit_index].i].id;
            }
            ++pi->num_ngb_density;
          }
        }
#endif

        /* If there are any interactions perform them. */
        if (vec_is_mask_true(v_doi_mask))
          runner_iact_nonsym_1_vec_density(
              &v_r2, &v_dx, &v_dy, &v_dz, &params, cj_cache, cj_cache_idx,
              &sum_cache, v_doi_mask);

      } /* loop over the parts in cj. */

      /* Perform horizontal adds on vector sums and store result in particle pi.*/
      update_density_particle(pi, &sum_cache);
      
    } /* loop over the parts in ci. */
  }

  /* Parts are on the right. */
  else {

    int first_pj = populate_max_index_subset(
        count, count_j, parts_i, ind, total_ci_shift, dxj, di_shift_correction,
        runner_shift_x, runner_shift_y, runner_shift_z, sort_j, max_index_i, 1);

    /* Read the particles from the cell and store them locally in the cache. */
    cache_read_particles_subset(cj, cj_cache, sort_j, &first_pj, 0, ci->loc, 1);

    /* Get the number of particles read into the ci cache. */
    const int cj_cache_count = count_j - first_pj;

    const double dj_max = sort_j[count_j - 1].d;

    /* Loop over the parts_i. */
    for (int pid = 0; pid < count; pid++) {

      /* Get a hold of the ith part in ci. */
      struct part *restrict pi = &parts_i[ind[pid]];
      const float pix = pi->x[0] - total_ci_shift[0];
      const float piy = pi->x[1] - total_ci_shift[1];
      const float piz = pi->x[2] - total_ci_shift[2];
      const float hi = pi->h;

      /* Skip this particle if no particle in cj is within range of it. */
      const double di = -hi * kernel_gamma - dxj + pix * runner_shift_x +
                        piy * runner_shift_y + piz * runner_shift_z +
                        di_shift_correction;
      if (di > dj_max) continue;

      /* Fill particle pi vectors. */
      const vector v_pix = vector_set1(pix);
      const vector v_piy = vector_set1(piy);
      const vector v_piz = vector_set1(piz);
      
      const float hig2 = hi * hi * kernel_gamma2;
      const vector v_hig2 = vector_set1(hig2);

      struct input_params_density params;
      populate_input_params_density(pi, &params);

      /* Reset cumulative sums of update vectors. */
      struct update_cache_density sum_cache;
      update_cache_density_init(&sum_cache);

      /* Convert exit iteration to cache indices. */
      int exit_iteration_align = max_index_i[pid] - first_pj;

      /* Pad the exit iteration align so cache reads are aligned. */
      const int rem = exit_iteration_align % VEC_SIZE;
      if (exit_iteration_align < VEC_SIZE) {
        exit_iteration_align = 0;
      } else
        exit_iteration_align -= rem;

      /* Loop over the parts in cj. */
      for (int cj_cache_idx = exit_iteration_align;
           cj_cache_idx < cj_cache_count; cj_cache_idx += VEC_SIZE) {

        vector v_dx, v_dy, v_dz, v_r2;

        /* Load 1 set of vectors from the particle cache. */
        const vector v_pjx = vector_load(&cj_cache->x[cj_cache_idx]);
        const vector v_pjy = vector_load(&cj_cache->y[cj_cache_idx]);
        const vector v_pjz = vector_load(&cj_cache->z[cj_cache_idx]);

        /* Compute the pairwise distance. */
        v_dx.v = vec_sub(v_pix.v, v_pjx.v);
        v_dy.v = vec_sub(v_piy.v, v_pjy.v);
        v_dz.v = vec_sub(v_piz.v, v_pjz.v);

        v_r2.v = vec_mul(v_dx.v, v_dx.v);
        v_r2.v = vec_fma(v_dy.v, v_dy.v, v_r2.v);
        v_r2.v = vec_fma(v_dz.v, v_dz.v, v_r2.v);

        /* Form r2 < hig2 mask. */
        mask_t v_doi_mask;
        vec_create_mask(v_doi_mask, vec_cmp_lt(v_r2.v, v_hig2.v));

#ifdef DEBUG_INTERACTIONS_SPH
        struct part *restrict parts_j = cj->parts;
        for (int bit_index = 0; bit_index < VEC_SIZE; bit_index++) {
          if (vec_is_mask_true(v_doi_mask) & (1 << bit_index)) {
            if (pi->num_ngb_density < MAX_NUM_OF_NEIGHBOURS) {
              pi->ids_ngbs_density[pi->num_ngb_density] =
                  parts_j[sort_j[cj_cache_idx + first_pj + bit_index].i].id;
            }
            ++pi->num_ngb_density;
          }
        }
#endif

        /* If there are any interactions perform them. */
        if (vec_is_mask_true(v_doi_mask))
          runner_iact_nonsym_1_vec_density(
              &v_r2, &v_dx, &v_dy, &v_dz, &params, cj_cache, cj_cache_idx, 
              &sum_cache, v_doi_mask);

      } /* loop over the parts in cj. */

      /* Perform horizontal adds on vector sums and store result in particle pi.*/
      update_density_particle(pi, &sum_cache);
      
    } /* loop over the parts in ci. */
  }

  TIMER_TOC(timer_dopair_subset);
#endif /* WITH_VECTORIZATION */
}

/**
 * @brief Compute the force interactions between a cell pair (non-symmetric)
 * using vector intrinsics.
 *
 * @param r The #runner.
 * @param ci The first #cell.
 * @param cj The second #cell.
 * @param sid The direction of the pair
 * @param shift The shift vector to apply to the particles in ci.
 */
void runner_dopair2_force_vec(struct runner *r, struct cell *ci,
                              struct cell *cj, const int sid,
                              const double *shift) {

#if defined(WITH_VECTORIZATION) && (defined(GADGET2_SPH) || defined(MINIMAL_SPH))

  const struct engine *restrict e = r->e;
  const timebin_t max_active_bin = e->max_active_bin;

  TIMER_TIC;

  /* Check whether cells are local to the node. */
  const int ci_local = (ci->nodeID == e->nodeID);
  const int cj_local = (cj->nodeID == e->nodeID);

  /* Get the cutoff shift. */
  double rshift = 0.0;
  for (int k = 0; k < 3; k++) rshift += shift[k] * runner_shift[sid][k];

  /* Pick-out the sorted lists. */
  const struct entry *restrict sort_i = ci->sort[sid];
  const struct entry *restrict sort_j = cj->sort[sid];

  /* Get some other useful values. */
  const int count_i = ci->count;
  const int count_j = cj->count;
  const double hi_max = ci->h_max * kernel_gamma;
  const double hj_max = cj->h_max * kernel_gamma;
  const double hi_max_raw = ci->h_max;
  const double hj_max_raw = cj->h_max;
  struct part *restrict parts_i = ci->parts;
  struct part *restrict parts_j = cj->parts;
  const double di_max = sort_i[count_i - 1].d - rshift;
  const double dj_min = sort_j[0].d;
  const float dx_max = (ci->dx_max_sort + cj->dx_max_sort);
  const int active_ci = cell_is_active_hydro(ci, e) && ci_local;
  const int active_cj = cell_is_active_hydro(cj, e) && cj_local;

#ifdef SWIFT_DEBUG_CHECKS
  /* Check that particles have been drifted to the current time */
  for (int pid = 0; pid < count_i; pid++)
    if (parts_i[pid].ti_drift != e->ti_current)
      error("Particle pi not drifted to current time");
  for (int pjd = 0; pjd < count_j; pjd++)
    if (parts_j[pjd].ti_drift != e->ti_current)
      error("Particle pj not drifted to current time");
#endif

  /* Check if any particles are active and in range */
  int numActive = 0;

  /* Use the largest smoothing length to make sure that no interactions are
   * missed. */
  const double h_max = max(hi_max, hj_max);

  if (active_ci) {
    for (int pid = count_i - 1;
         pid >= 0 && sort_i[pid].d + h_max + dx_max > dj_min; pid--) {
      const struct part *restrict pi = &parts_i[sort_i[pid].i];
      if (part_is_active_no_debug(pi, max_active_bin)) {
        numActive++;
        break;
      }
    }
  }

  if (!numActive && active_cj) {
    for (int pjd = 0; pjd < count_j && sort_j[pjd].d - h_max - dx_max < di_max;
         pjd++) {
      const struct part *restrict pj = &parts_j[sort_j[pjd].i];
      if (part_is_active_no_debug(pj, max_active_bin)) {
        numActive++;
        break;
      }
    }
  }

  /* Return if no active particle in range */
  if (numActive == 0) return;

  /* Get both particle caches from the runner and re-allocate
   * them if they are not big enough for the cells. */
  struct cache *restrict ci_cache = &r->ci_cache;
  struct cache *restrict cj_cache = &r->cj_cache;
  if (ci_cache->count < count_i) cache_init(ci_cache, count_i);
  if (cj_cache->count < count_j) cache_init(cj_cache, count_j);

  /* Get a direct pointer to the index arrays */
  int first_pi, last_pj;
  swift_declare_aligned_ptr(int, max_index_i, r->ci_cache.max_index,
                            SWIFT_CACHE_ALIGNMENT);
  swift_declare_aligned_ptr(int, max_index_j, r->cj_cache.max_index,
                            SWIFT_CACHE_ALIGNMENT);

  /* Find particles maximum distance into cj, max_di[] and ci, max_dj[]. */
  /* Also find the first pi that interacts with any particle in cj and the last
   * pj that interacts with any particle in ci. */
  populate_max_index_force(ci, cj, sort_i, sort_j, dx_max, rshift, hi_max_raw,
                           hj_max_raw, h_max, di_max, dj_min, max_index_i,
                           max_index_j, &first_pi, &last_pj, max_active_bin,
                           active_ci, active_cj);

  /* Limits of the outer loops. */
  const int first_pi_loop = first_pi;
  const int last_pj_loop_end = last_pj + 1;

  /* Take the max/min of both values calculated to work out how many particles
   * to read into the cache. */
  last_pj = max(last_pj, max_index_i[count_i - 1]);
  first_pi = min(first_pi, max_index_j[0]);

  /* Read the required particles into the two caches. */
  cache_read_two_partial_cells_sorted_force(ci, cj, ci_cache, cj_cache, sort_i,
                                            sort_j, shift, &first_pi, &last_pj);

  /* Get the number of particles read into the ci cache. */
  const int ci_cache_count = count_i - first_pi;

  if (active_ci) {

    /* Loop over the parts in ci until nothing is within range in cj. */
    for (int pid = count_i - 1; pid >= first_pi_loop; pid--) {

      /* Get a hold of the ith part in ci. */
      struct part *restrict pi = &parts_i[sort_i[pid].i];
      if (!part_is_active(pi, e)) continue;

      /* Set the cache index. */
      const int ci_cache_idx = pid - first_pi;

      /* Skip this particle if no particle in cj is within range of it. */
      const float hi = ci_cache->h[ci_cache_idx];
      const double di_test =
          sort_i[pid].d + max(hi, hj_max_raw) * kernel_gamma + dx_max - rshift;
      if (di_test < dj_min) continue;

      /* Determine the exit iteration of the interaction loop. */
      const int exit_iteration_end = max_index_i[pid] + 1;

      const float hig2 = hi * hi * kernel_gamma2;
      
      /* Fill particle pi vectors. */
      const vector v_pix = vector_set1(ci_cache->x[ci_cache_idx]);
      const vector v_piy = vector_set1(ci_cache->y[ci_cache_idx]);
      const vector v_piz = vector_set1(ci_cache->z[ci_cache_idx]);
      const vector v_hig2 = vector_set1(hig2);
      
      struct input_params_force params;
      populate_input_params_force_cache(ci_cache, ci_cache_idx, &params);
      
      /* Reset cumulative sums of update vectors. */
      struct update_cache_force sum_cache;
      update_cache_force_init(&sum_cache);
      
      /* Loop over the parts in cj. Making sure to perform an iteration of the
       * loop even if exit_iteration_align is zero and there is only one
       * particle to interact with.*/
      for (int pjd = 0; pjd < exit_iteration_end; pjd += VEC_SIZE) {

        /* Get the cache index to the jth particle. */
        const int cj_cache_idx = pjd;

        vector v_dx, v_dy, v_dz, v_r2;
        vector v_hjg2;

#ifdef SWIFT_DEBUG_CHECKS
        if (cj_cache_idx % VEC_SIZE != 0 || cj_cache_idx < 0 ||
            cj_cache_idx + (VEC_SIZE - 1) > (last_pj + 1 + VEC_SIZE)) {
          error("Unaligned read!!! cj_cache_idx=%d, last_pj=%d", cj_cache_idx,
                last_pj);
        }
#endif

        /* Load 2 sets of vectors from the particle cache. */
        const vector v_pjx = vector_load(&cj_cache->x[cj_cache_idx]);
        const vector v_pjy = vector_load(&cj_cache->y[cj_cache_idx]);
        const vector v_pjz = vector_load(&cj_cache->z[cj_cache_idx]);
        const vector v_hj = vector_load(&cj_cache->h[cj_cache_idx]);
        v_hjg2.v = vec_mul(vec_mul(v_hj.v, v_hj.v), kernel_gamma2_vec.v);

        /* Compute the pairwise distance. */
        v_dx.v = vec_sub(v_pix.v, v_pjx.v);
        v_dy.v = vec_sub(v_piy.v, v_pjy.v);
        v_dz.v = vec_sub(v_piz.v, v_pjz.v);

        v_r2.v = vec_mul(v_dx.v, v_dx.v);
        v_r2.v = vec_fma(v_dy.v, v_dy.v, v_r2.v);
        v_r2.v = vec_fma(v_dz.v, v_dz.v, v_r2.v);

        mask_t v_doi_mask;

        /* Form a mask from r2 < hig2 mask and r2 < hjg2 mask. */
        vector v_h2;
        v_h2.v = vec_fmax(v_hig2.v, v_hjg2.v);
        vec_create_mask(v_doi_mask, vec_cmp_lt(v_r2.v, v_h2.v));

#ifdef DEBUG_INTERACTIONS_SPH
        for (int bit_index = 0; bit_index < VEC_SIZE; bit_index++) {
          if (vec_is_mask_true(v_doi_mask) & (1 << bit_index)) {
            if (pi->num_ngb_force < MAX_NUM_OF_NEIGHBOURS)
              pi->ids_ngbs_force[pi->num_ngb_force] =
                  parts_j[sort_j[pjd + bit_index].i].id;
            ++pi->num_ngb_force;
          }
        }
#endif

        /* If there are any interactions perform them. */
        if (vec_is_mask_true(v_doi_mask)) {
          vector v_hj_inv = vec_reciprocal(v_hj);

          runner_iact_nonsym_1_vec_force(
              &v_r2, &v_dx, &v_dy, &v_dz, &params,
              cj_cache, cj_cache_idx,
              v_hj_inv, &sum_cache,
              v_doi_mask);
        }

      } /* loop over the parts in cj. */

      /* Perform horizontal adds on vector sums and store result in pi. */
      update_force_particle(pi, &sum_cache);

    } /* loop over the parts in ci. */
  }

  if (active_cj) {

    /* Loop over the parts in cj until nothing is within range in ci. */
    for (int pjd = 0; pjd < last_pj_loop_end; pjd++) {

      /* Get a hold of the jth part in cj. */
      struct part *restrict pj = &parts_j[sort_j[pjd].i];
      if (!part_is_active(pj, e)) continue;

      /* Set the cache index. */
      const int cj_cache_idx = pjd;

      /* Skip this particle if no particle in ci is within range of it. */
      const float hj = cj_cache->h[cj_cache_idx];
      const double dj_test =
          sort_j[pjd].d - max(hj, hi_max_raw) * kernel_gamma - dx_max;
      if (dj_test > di_max) continue;

      /* Determine the exit iteration of the interaction loop. */
      const int exit_iteration = max_index_j[pjd];

      const float hjg2 = hj * hj * kernel_gamma2;
      
      /* Fill particle pi vectors. */
      const vector v_pjx = vector_set1(cj_cache->x[cj_cache_idx]);
      const vector v_pjy = vector_set1(cj_cache->y[cj_cache_idx]);
      const vector v_pjz = vector_set1(cj_cache->z[cj_cache_idx]);
      const vector v_hjg2 = vector_set1(hjg2);

      struct input_params_force params;
      populate_input_params_force_cache(cj_cache, cj_cache_idx, &params);
      
      /* Reset cumulative sums of update vectors. */
      struct update_cache_force sum_cache;
      update_cache_force_init(&sum_cache);
      
      /* Convert exit iteration to cache indices. */
      int exit_iteration_align = exit_iteration - first_pi;

      /* Pad the exit iteration align so cache reads are aligned. */
      const int rem = exit_iteration_align % VEC_SIZE;
      if (exit_iteration_align < VEC_SIZE) {
        exit_iteration_align = 0;
      } else
        exit_iteration_align -= rem;

      /* Loop over the parts in ci. */
      for (int ci_cache_idx = exit_iteration_align;
           ci_cache_idx < ci_cache_count; ci_cache_idx += VEC_SIZE) {

#ifdef SWIFT_DEBUG_CHECKS
        if (ci_cache_idx % VEC_SIZE != 0 || ci_cache_idx < 0) {
          error("Unaligned read!!! ci_cache_idx=%d", ci_cache_idx);
        }
#endif

        vector v_hig2;
        vector v_dx, v_dy, v_dz, v_r2;

        /* Load 2 sets of vectors from the particle cache. */
        const vector v_pix = vector_load(&ci_cache->x[ci_cache_idx]);
        const vector v_piy = vector_load(&ci_cache->y[ci_cache_idx]);
        const vector v_piz = vector_load(&ci_cache->z[ci_cache_idx]);
        const vector v_hi = vector_load(&ci_cache->h[ci_cache_idx]);
        v_hig2.v = vec_mul(vec_mul(v_hi.v, v_hi.v), kernel_gamma2_vec.v);

        /* Compute the pairwise distance. */
        v_dx.v = vec_sub(v_pjx.v, v_pix.v);
        v_dy.v = vec_sub(v_pjy.v, v_piy.v);
        v_dz.v = vec_sub(v_pjz.v, v_piz.v);

        v_r2.v = vec_mul(v_dx.v, v_dx.v);
        v_r2.v = vec_fma(v_dy.v, v_dy.v, v_r2.v);
        v_r2.v = vec_fma(v_dz.v, v_dz.v, v_r2.v);

        mask_t v_doj_mask;

        /* Form a mask from r2 < hig2 mask and r2 < hjg2 mask. */
        vector v_h2;
        v_h2.v = vec_fmax(v_hjg2.v, v_hig2.v);
        vec_create_mask(v_doj_mask, vec_cmp_lt(v_r2.v, v_h2.v));

#ifdef DEBUG_INTERACTIONS_SPH
        for (int bit_index = 0; bit_index < VEC_SIZE; bit_index++) {
          if (vec_is_mask_true(v_doj_mask) & (1 << bit_index)) {
            if (pj->num_ngb_force < MAX_NUM_OF_NEIGHBOURS)
              pj->ids_ngbs_force[pj->num_ngb_force] =
                  parts_i[sort_i[ci_cache_idx + first_pi + bit_index].i].id;
            ++pj->num_ngb_force;
          }
        }
#endif

        /* If there are any interactions perform them. */
        if (vec_is_mask_true(v_doj_mask)) {
          vector v_hi_inv = vec_reciprocal(v_hi);

          runner_iact_nonsym_1_vec_force(
              &v_r2, &v_dx, &v_dy, &v_dz, &params,
              ci_cache, ci_cache_idx, 
              v_hi_inv, &sum_cache,
              v_doj_mask);
        }
      } /* loop over the parts in ci. */

      /* Perform horizontal adds on vector sums and store result in pj. */
      update_force_particle(pj, &sum_cache);
      
    } /* loop over the parts in cj. */

    TIMER_TOC(timer_dopair_density);
  }

#else

  error("Incorrectly calling vectorized Gadget-2 functions!");

#endif /* WITH_VECTORIZATION */
}
