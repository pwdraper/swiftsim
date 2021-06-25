/*******************************************************************************
 * This file is part of SWIFT.
 * Coypright (c) 2021 Mladen Ivkovic (mladen.ivkovic@hotmail.com)
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

#ifndef SWIFT_GEAR_RT_FLUX_H
#define SWIFT_GEAR_RT_FLUX_H

#if defined(RT_RIEMANN_SOLVER_GLF)
#include "rt_riemann_GLF.h"
#elif defined(RT_RIEMANN_SOLVER_HLL)
#include "rt_riemann_HLL.h"
#else
#error "No valid choice of RT Riemann solver has been selected"
#endif

/**
 * @file src/rt/GEAR/rt_flux.h
 * @brief Functions related to compute the interparticle flux term of the
 * conservation law. This is its own file so we can switch between Riemann
 * solvers more easily.
 */


/**
 * @brief Reset the fluxes for the given particle.
 *
 * @param p Particle.
 */
__attribute__((always_inline)) INLINE static void rt_part_reset_fluxes(
    struct part* restrict p) {

  for (int g = 0; g < RT_NGROUPS; g++) {
    p->rt_data.flux[g].energy = 0.f;
    p->rt_data.flux[g].flux[0] = 0.f;
    p->rt_data.flux[g].flux[1] = 0.f;
    p->rt_data.flux[g].flux[2] = 0.f;
  }
}

/**
 * @brief Compute the flux between a left state Qleft and a right
 * state Qright along the direction of the unit vector n_unit
 * through a surface of size Anorm.
 *
 * @param QL left state
 * @param QR right state
 * @param n_unit unit vector of the direction of the surface
 * @param Anorm size of the surface through which the flux goes
 * @param totflux the resulting flux
 */
__attribute__((always_inline)) INLINE static void rt_compute_flux(
    const float QL[4], const float QR[4], const float n_unit[3],
    const float Anorm, float fluxes[4]) {

  /* if (QL[0] <= 0.f || QR[0] <= 0.f) */
  /*   message("------------------------------ Caught negative energies %.3e %.3e %.3e %.3e |  %.3e %.3e %.3e %.3e", QL[0], QL[1], QL[2], QL[3], QR[0], QR[1], QR[2], QR[3]); */
  if (QL[0] <= 0.f && QR[0] <= 0.f) {
    fluxes[0] = 0.f;
    fluxes[1] = 0.f;
    fluxes[2] = 0.f;
    fluxes[3] = 0.f;
    return;
  }

  float Fhalf[4][3]; /* flux at interface */
  rt_riemann_solve_for_flux(QL, QR, Fhalf);

  if (fluxes[0] != fluxes[0])
    message("----- Caught fluxes NAN %.3e | %.3e %.3e %.3e %.3e |  %.3e %.3e %.3e %.3e", fluxes[0], QL[0], QL[1], QL[2], QL[3], QR[0], QR[1], QR[2], QR[3]);

  /* now project the total flux along the direction of the surface */
  fluxes[0] = Fhalf[0][0] * n_unit[0] + Fhalf[0][1] * n_unit[1] + Fhalf[0][2] * n_unit[2];
  fluxes[1] = Fhalf[1][0] * n_unit[0] + Fhalf[1][1] * n_unit[1] + Fhalf[1][2] * n_unit[2];
  fluxes[2] = Fhalf[2][0] * n_unit[0] + Fhalf[2][1] * n_unit[1] + Fhalf[2][2] * n_unit[2];
  fluxes[3] = Fhalf[3][0] * n_unit[0] + Fhalf[3][1] * n_unit[1] + Fhalf[3][2] * n_unit[2];

  /* get the actual flux */
  fluxes[0] *= Anorm;
  fluxes[1] *= Anorm;
  fluxes[2] *= Anorm;
  fluxes[3] *= Anorm;
}
#endif /* SWIFT_GEAR_RT_FLUX_H */
