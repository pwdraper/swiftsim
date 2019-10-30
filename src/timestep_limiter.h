/*******************************************************************************
 * This file is part of SWIFT.
 * Copyright (c) 2016 Matthieu Schaller (matthieu.schaller@durham.ac.uk)
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
#ifndef SWIFT_TIMESTEP_LIMITER_H
#define SWIFT_TIMESTEP_LIMITER_H

/* Config parameters. */
#include "../config.h"

/* Local headers. */
#include "kick.h"

/**
 * @brief Wakes up a particle by rewinding it's kick1 back in time and applying
 * a new one such that the particle becomes active again in the next time-step.
 *
 * @param p The #part to update.
 * @param xp Its #xpart companion.
 * @param e The #engine (to extract time-line information).
 */
__attribute__((always_inline)) INLINE static integertime_t timestep_limit_part(
    struct part *restrict p, struct xpart *restrict xp,
    const struct engine *e) {

  const struct cosmology *cosmo = e->cosmology;
  const int with_cosmology = e->policy & engine_policy_cosmology;
  const double time_base = e->time_base;
  const integertime_t ti_current = e->ti_current;

  if (p->id == ICHECK)
    message("LIMITER time_bin=%d wakeup=%d", p->time_bin, p->wakeup);

  if (part_is_active(p, e)) {

    // message("Limiting active particle!");

    /* First case, the particle was active so we only need to update the length
       of its next time-step */

    /* New time-bin of this particle */
    p->time_bin = -p->wakeup + 2;

    /* Mark the particle as being rady to be time integrated */
    p->wakeup = time_bin_not_awake;

    return get_integer_timestep(p->time_bin);

  } else {

    // message("Limiting inactive particle!");

    /* Second case, the particle was inactive so we need to interrupt its
       time-step, undo the "kick" operator and assign a new time-step size */

    /* The timebins to play with */
    const timebin_t old_bin = p->time_bin;
    const timebin_t new_bin = -p->wakeup + 2;

    /* Current start and end time of this particle */
    const integertime_t ti_beg_old =
        get_integer_time_begin(ti_current, old_bin);
    const integertime_t ti_end_old = get_integer_time_end(ti_current, old_bin);

    /* Length of the old and new time-step */
    const integertime_t dti_old = ti_end_old - ti_beg_old;
    const integertime_t dti_new = get_integer_timestep(new_bin);

    /* Let's now search for the starting point of the new step */
    int k = 0;
    while (ti_beg_old + k * dti_new <= ti_current) k++;

    const integertime_t ti_beg_new = ti_beg_old + (k - 1) * dti_new;

    if (p->id == ICHECK) {
      message("ti_kick=%lld (%d)", p->ti_kick, get_time_bin(p->ti_kick) + 1);
      message("ti_beg_new = %lld", ti_beg_new);
      message("ti_beg_old = %lld", ti_beg_new);
    }

#ifdef SWIFT_DEBUG_CHECKS
    /* Some basic safety checks */
    if (ti_beg_old >= e->ti_current)
      error(
          "Incorrect value for old time-step beginning ti_current=%lld, "
          "ti_beg_old=%lld",
          e->ti_current, ti_beg_old);

    if (ti_end_old <= e->ti_current)
      error(
          "Incorrect value for old time-step end ti_current=%lld, "
          "ti_end_old=%lld",
          e->ti_current, ti_end_old);

    if (ti_beg_new < ti_beg_old)
      error("New beg of time-step before the old one");

    if (dti_new > dti_old) error("New time-step larger than old one");
#endif

    double dt_kick_grav = 0., dt_kick_hydro = 0., dt_kick_therm = 0.,
           dt_kick_corr = 0.;

    /* if (with_cosmology) { */

    /* } else { */
    /*   dt_kick_hydro = (ti_beg_new - ti_beg_old) * time_base; */
    /*   dt_kick_grav = (ti_beg_new - ti_beg_old) * time_base; */
    /*   dt_kick_therm = (ti_beg_new - ti_beg_old) * time_base; */
    /*   dt_kick_corr = (ti_beg_new - ti_beg_old) * time_base; */
    /* } */

    /* kick_part(p, xp, dt_kick_hydro, dt_kick_grav, dt_kick_therm,
     * dt_kick_corr, */
    /*           e->cosmology, e->hydro_properties, e->entropy_floor, */
    /*           ti_beg_new, ti_beg_old); */

    /* Now we need to reverse the kick1... (the dt are negative here) */
    if (with_cosmology) {
      dt_kick_hydro = -cosmology_get_hydro_kick_factor(
          cosmo, ti_beg_old, ti_beg_old + dti_old / 2);
      dt_kick_grav = -cosmology_get_grav_kick_factor(cosmo, ti_beg_old,
                                                     ti_beg_old + dti_old / 2);
      dt_kick_therm = -cosmology_get_therm_kick_factor(
          cosmo, ti_beg_old, ti_beg_old + dti_old / 2);
      dt_kick_corr = -cosmology_get_corr_kick_factor(cosmo, ti_beg_old,
                                                     ti_beg_old + dti_old / 2);
    } else {
      dt_kick_hydro = -(dti_old / 2) * time_base;
      dt_kick_grav = -(dti_old / 2) * time_base;
      dt_kick_therm = -(dti_old / 2) * time_base;
      dt_kick_corr = -(dti_old / 2) * time_base;
    }

    kick_part(p, xp, dt_kick_hydro, dt_kick_grav, dt_kick_therm, dt_kick_corr,
              e->cosmology, e->hydro_properties, e->entropy_floor,
              ti_beg_old + dti_old / 2, ti_beg_old);

    if (p->id == ICHECK) {
      message("ti_kick=%lld (%d)", p->ti_kick, get_time_bin(p->ti_kick) + 1);
    }

    /* ...and apply the new one (dt is positiive) */
    if (with_cosmology) {
      dt_kick_hydro = cosmology_get_hydro_kick_factor(cosmo, ti_beg_new,
                                                      ti_beg_new + dti_new / 2);
      dt_kick_grav = cosmology_get_grav_kick_factor(cosmo, ti_beg_new,
                                                    ti_beg_new + dti_new / 2);
      dt_kick_therm = cosmology_get_therm_kick_factor(cosmo, ti_beg_new,
                                                      ti_beg_new + dti_new / 2);
      dt_kick_corr = cosmology_get_corr_kick_factor(cosmo, ti_beg_new,
                                                    ti_beg_new + dti_new / 2);
    } else {
      dt_kick_hydro = (ti_beg_new - ti_beg_old) * time_base;
      dt_kick_grav = (ti_beg_new - ti_beg_old) * time_base;
      dt_kick_therm = (ti_beg_new - ti_beg_old) * time_base;
      dt_kick_corr = (ti_beg_new - ti_beg_old) * time_base;
    }

    kick_part(p, xp, dt_kick_hydro, dt_kick_grav, dt_kick_therm, dt_kick_corr,
              e->cosmology, e->hydro_properties, e->entropy_floor, ti_beg_old,
              ti_beg_new);

    if (p->id == ICHECK) {
      message("ti_kick=%lld (%d)", p->ti_kick, get_time_bin(p->ti_kick) + 1);
    }

    if (new_bin > e->max_active_bin) {

      if (with_cosmology) {
        dt_kick_hydro = cosmology_get_hydro_kick_factor(
            cosmo, ti_beg_new, ti_beg_new + dti_new / 2);
        dt_kick_grav = cosmology_get_grav_kick_factor(cosmo, ti_beg_new,
                                                      ti_beg_new + dti_new / 2);
        dt_kick_therm = cosmology_get_therm_kick_factor(
            cosmo, ti_beg_new, ti_beg_new + dti_new / 2);
        dt_kick_corr = cosmology_get_corr_kick_factor(cosmo, ti_beg_new,
                                                      ti_beg_new + dti_new / 2);
      } else {
        dt_kick_hydro = (dti_new / 2) * time_base;
        dt_kick_grav = (dti_new / 2) * time_base;
        dt_kick_therm = (dti_new / 2) * time_base;
        dt_kick_corr = (dti_new / 2) * time_base;
      }

      kick_part(p, xp, dt_kick_hydro, dt_kick_grav, dt_kick_therm, dt_kick_corr,
                e->cosmology, e->hydro_properties, e->entropy_floor, ti_beg_new,
                ti_beg_new + dti_new / 2);
    }

    /* New time-bin of this particle */
    p->time_bin = new_bin;

    /* Mark the particle as being ready to be time integrated */
    p->wakeup = time_bin_not_awake;

    if (p->id == ICHECK) message("new time bin=%d", p->time_bin);

    return get_integer_timestep(new_bin);
  }
}

#endif /* SWIFT_TIMESTEP_LIMITER_H */
