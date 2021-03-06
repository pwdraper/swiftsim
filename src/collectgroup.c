/*******************************************************************************
 * This file is part of SWIFT.
 * Copyright (c) 2017 Peter W. Draper (p.w.draper@durham.ac.uk)
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

/* MPI headers. */
#ifdef WITH_MPI
#include <mpi.h>
#endif

/* This object's header. */
#include "collectgroup.h"

/* Local headers. */
#include "engine.h"
#include "error.h"

#ifdef WITH_MPI

/* Local collections for MPI reduces. */
struct mpicollectgroup1 {
  long long updates, g_updates, s_updates;
  integertime_t ti_hydro_end_min;
  integertime_t ti_gravity_end_min;
  int forcerebuild;
};

/* Forward declarations. */
static void mpicollect_create_MPI_type(void);

/**
 * @brief MPI datatype for the #mpicollectgroup1 structure.
 */
static MPI_Datatype mpicollectgroup1_type;

/**
 * @brief MPI operator to reduce #mpicollectgroup1 structures.
 */
static MPI_Op mpicollectgroup1_reduce_op;

#endif

/**
 * @brief Perform any once only initialisations. Must be called once.
 */
void collectgroup_init(void) {

#ifdef WITH_MPI
  /* Initialise the MPI types. */
  mpicollect_create_MPI_type();
#endif
}

/**
 * @brief Apply the collectgroup1 values to the engine by copying all the
 * values to the engine fields.
 *
 * @param grp1 The #collectgroup1
 * @param e The #engine
 */
void collectgroup1_apply(struct collectgroup1 *grp1, struct engine *e) {
  e->ti_hydro_end_min = grp1->ti_hydro_end_min;
  e->ti_hydro_end_max = grp1->ti_hydro_end_max;
  e->ti_hydro_beg_max = grp1->ti_hydro_beg_max;
  e->ti_gravity_end_min = grp1->ti_gravity_end_min;
  e->ti_gravity_end_max = grp1->ti_gravity_end_max;
  e->ti_gravity_beg_max = grp1->ti_gravity_beg_max;
  e->ti_end_min = min(e->ti_hydro_end_min, e->ti_gravity_end_min);
  e->ti_end_max = max(e->ti_hydro_end_max, e->ti_gravity_end_max);
  e->ti_beg_max = max(e->ti_hydro_beg_max, e->ti_gravity_beg_max);
  e->updates = grp1->updates;
  e->g_updates = grp1->g_updates;
  e->s_updates = grp1->s_updates;
  e->forcerebuild = grp1->forcerebuild;
}

/**
 * @brief Initialises a collectgroup1 struct ready for processing.
 *
 * @param grp1 The #collectgroup1 to initialise
 * @param updates the number of updated hydro particles on this node this step.
 * @param g_updates the number of updated gravity particles on this node this
 * step.
 * @param s_updates the number of updated star particles on this node this step.
 * @param ti_hydro_end_min the minimum end time for next hydro time step after
 * this step.
 * @param ti_hydro_end_max the maximum end time for next hydro time step after
 * this step.
 * @param ti_hydro_beg_max the maximum begin time for next hydro time step after
 * this step.
 * @param ti_gravity_end_min the minimum end time for next gravity time step
 * after this step.
 * @param ti_gravity_end_max the maximum end time for next gravity time step
 * after this step.
 * @param ti_gravity_beg_max the maximum begin time for next gravity time step
 * after this step.
 * @param forcerebuild whether a rebuild is required after this step.
 */
void collectgroup1_init(struct collectgroup1 *grp1, size_t updates,
                        size_t g_updates, size_t s_updates,
                        integertime_t ti_hydro_end_min,
                        integertime_t ti_hydro_end_max,
                        integertime_t ti_hydro_beg_max,
                        integertime_t ti_gravity_end_min,
                        integertime_t ti_gravity_end_max,
                        integertime_t ti_gravity_beg_max, int forcerebuild) {
  grp1->updates = updates;
  grp1->g_updates = g_updates;
  grp1->s_updates = s_updates;
  grp1->ti_hydro_end_min = ti_hydro_end_min;
  grp1->ti_hydro_end_max = ti_hydro_end_max;
  grp1->ti_hydro_beg_max = ti_hydro_beg_max;
  grp1->ti_gravity_end_min = ti_gravity_end_min;
  grp1->ti_gravity_end_max = ti_gravity_end_max;
  grp1->ti_gravity_beg_max = ti_gravity_beg_max;
  grp1->forcerebuild = forcerebuild;
}

/**
 * @brief Do any processing necessary to the group before it can be used.
 *
 * This may involve an MPI reduction across all nodes.
 *
 * @param grp1 the #collectgroup1 struct already initialised by a call
 *             to collectgroup1_init.
 */
void collectgroup1_reduce(struct collectgroup1 *grp1) {

#ifdef WITH_MPI

  /* Populate an MPI group struct and reduce this across all nodes. */
  struct mpicollectgroup1 mpigrp11;
  mpigrp11.updates = grp1->updates;
  mpigrp11.g_updates = grp1->g_updates;
  mpigrp11.s_updates = grp1->s_updates;
  mpigrp11.ti_hydro_end_min = grp1->ti_hydro_end_min;
  mpigrp11.ti_gravity_end_min = grp1->ti_gravity_end_min;
  mpigrp11.forcerebuild = grp1->forcerebuild;

  struct mpicollectgroup1 mpigrp12;
  if (MPI_Allreduce(&mpigrp11, &mpigrp12, 1, mpicollectgroup1_type,
                    mpicollectgroup1_reduce_op, MPI_COMM_WORLD) != MPI_SUCCESS)
    error("Failed to reduce mpicollection1.");

  /* And update. */
  grp1->updates = mpigrp12.updates;
  grp1->g_updates = mpigrp12.g_updates;
  grp1->s_updates = mpigrp12.s_updates;
  grp1->ti_hydro_end_min = mpigrp12.ti_hydro_end_min;
  grp1->ti_gravity_end_min = mpigrp12.ti_gravity_end_min;
  grp1->forcerebuild = mpigrp12.forcerebuild;

#endif
}

#ifdef WITH_MPI
/**
 * @brief Do the reduction of two structs.
 *
 * @param mpigrp11 the first struct, this is updated on exit.
 * @param mpigrp12 the second struct
 */
static void doreduce1(struct mpicollectgroup1 *mpigrp11,
                      const struct mpicollectgroup1 *mpigrp12) {

  /* Do what is needed for each part of the collection. */
  /* Sum of updates. */
  mpigrp11->updates += mpigrp12->updates;
  mpigrp11->g_updates += mpigrp12->g_updates;
  mpigrp11->s_updates += mpigrp12->s_updates;

  /* Minimum end time. */
  mpigrp11->ti_hydro_end_min =
      min(mpigrp11->ti_hydro_end_min, mpigrp12->ti_hydro_end_min);
  mpigrp11->ti_gravity_end_min =
      min(mpigrp11->ti_gravity_end_min, mpigrp12->ti_gravity_end_min);

  /* Everyone must agree to not rebuild. */
  if (mpigrp11->forcerebuild || mpigrp12->forcerebuild)
    mpigrp11->forcerebuild = 1;
}

/**
 * @brief MPI reduce operator for #mpicollectgroup1 structures.
 */
static void mpicollectgroup1_reduce(void *in, void *inout, int *len,
                                    MPI_Datatype *datatype) {

  for (int i = 0; i < *len; ++i)
    doreduce1(&((struct mpicollectgroup1 *)inout)[0],
              &((const struct mpicollectgroup1 *)in)[i]);
}

/**
 * @brief Registers any MPI collection types and reduction functions.
 */
static void mpicollect_create_MPI_type(void) {

  if (MPI_Type_contiguous(sizeof(struct mpicollectgroup1), MPI_BYTE,
                          &mpicollectgroup1_type) != MPI_SUCCESS ||
      MPI_Type_commit(&mpicollectgroup1_type) != MPI_SUCCESS) {
    error("Failed to create MPI type for mpicollection1.");
  }

  /* Create the reduction operation */
  MPI_Op_create(mpicollectgroup1_reduce, 1, &mpicollectgroup1_reduce_op);
}
#endif
