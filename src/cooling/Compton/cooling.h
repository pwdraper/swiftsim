/*******************************************************************************
 * This file is part of SWIFT.
 * Copyright (c) 2018 Matthieu Schaller (matthieu.schaller@durham.ac.uk)
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
#ifndef SWIFT_COOLING_COMPTON_H
#define SWIFT_COOLING_COMPTON_H

/* Config parameters. */
#include "../config.h"

/* Some standard headers. */
#include <float.h>
#include <math.h>

/* Local includes. */
#include "const.h"
#include "error.h"
#include "hydro.h"
#include "parser.h"
#include "part.h"
#include "physical_constants.h"
#include "units.h"

/**
 * @brief Calculates du/dt in CGS units for a particle.
 *
 * The cooling rate is \f$\frac{du}{dt} = -\Lambda \frac{n_H^2}{\rho} \f$ and
 * the returned value is in physical [erg * g^-1 * s^-1].
 *
 * @param cosmo The current cosmological model.
 * @param hydro_props The properties of the hydro scheme.
 * @param cooling The #cooling_function_data used in the run.
 * @param p Pointer to the particle data.
 * @return The change in energy per unit mass due to cooling for this particle
 * in cgs units [erg * g^-1 * s^-1].
 */
__attribute__((always_inline)) INLINE static double cooling_rate_cgs(
    const struct cosmology* cosmo, const struct hydro_props* hydro_props,
    const struct cooling_function_data* cooling, const struct part* p) {

  /* Get particle density */
  const double rho = hydro_get_physical_density(p, cosmo);
  const double rho_cgs = rho * cooling->conv_factor_density_to_cgs;

  /* Get Hydrogen mass fraction */
  const double X_H = hydro_props->hydrogen_mass_fraction;

  /* Hydrogen number density (X_H * rho / m_p) */
  const double n_H_cgs = X_H * rho_cgs * cooling->proton_mass_cgs_inv;

  /* Calculate du_dt (Lambda * n_H^2 / rho) */
  const double du_dt_cgs = -cooling->lambda_cgs * n_H_cgs * n_H_cgs / rho_cgs;

  return du_dt_cgs;
}

/**
 * @brief Apply the cooling function to a particle.
 *
 * @param phys_const The physical constants in internal units.
 * @param us The internal system of units.
 * @param cosmo The current cosmological model.
 * @param hydro_props The properties of the hydro scheme.
 * @param cooling The #cooling_function_data used in the run.
 * @param p Pointer to the particle data.
 * @param xp Pointer to the particle' extended data.
 * @param dt The time-step of this particle.
 * @param dt_therm The time-step operator used for thermal quantities.
 */
__attribute__((always_inline)) INLINE static void cooling_cool_part(
    const struct phys_const* restrict phys_const,
    const struct unit_system* restrict us,
    const struct cosmology* restrict cosmo,
    const struct hydro_props* hydro_props,
    const struct cooling_function_data* restrict cooling,
    struct part* restrict p, struct xpart* restrict xp, const float dt,
    const float dt_therm) {

  /* Nothing to do here? */
  if (dt == 0.) return;

  /* Internal energy floor */
  const float u_floor = hydro_props->minimal_internal_energy;

  /* Current energy */
  const float u_old = hydro_get_physical_internal_energy(p, xp, cosmo);

  /* Current du_dt in physical coordinates (internal units) */
  const float hydro_du_dt = hydro_get_physical_internal_energy_dt(p, cosmo);

  /* Calculate cooling du_dt (in cgs units) */
  const double cooling_du_dt_cgs =
      cooling_rate_cgs(cosmo, hydro_props, cooling, p);

  /* Convert to internal units */
  float cooling_du_dt =
      cooling_du_dt_cgs * cooling->conv_factor_energy_rate_from_cgs;

  /* Add cosmological term */
  cooling_du_dt *= cosmo->a * cosmo->a;

  float total_du_dt = hydro_du_dt + cooling_du_dt;

  /* We now need to check that we are not going to go below any of the limits */

  /* First, check whether we may end up below the minimal energy after
   * this step 1/2 kick + another 1/2 kick that could potentially be for
   * a time-step twice as big. We hence check for 1.5 delta_t. */
  if (u_old + total_du_dt * 1.5 * dt_therm < u_floor) {
    total_du_dt = (u_floor - u_old) / (1.5f * dt_therm);
  }

  /* Second, check whether the energy used in the prediction could get negative.
   * We need to check for the 1/2 dt kick followed by a full time-step drift
   * that could potentially be for a time-step twice as big. We hence check
   * for 2.5 delta_t but this time against 0 energy not the minimum */
  if (u_old + total_du_dt * 2.5 * dt_therm < 0.) {
    total_du_dt = -u_old / ((2.5f + 0.0001f) * dt_therm);
  }

  /* Update the internal energy time derivative */
  hydro_set_physical_internal_energy_dt(p, cosmo, total_du_dt);

  /* Store the radiated energy (assuming dt will not change) */
  xp->cooling_data.radiated_energy +=
      -hydro_get_mass(p) * (total_du_dt - hydro_du_dt) * dt_therm;
}

/**
 * @brief Computes the time-step due to cooling for this particle.
 *
 * We compute a time-step \f$ \alpha \frac{u}{du/dt} \f$ in physical
 * coordinates. \f$\alpha\f$ is a parameter of the cooling function.
 *
 * @param cooling The #cooling_function_data used in the run.
 * @param phys_const The physical constants in internal units.
 * @param cosmo The current cosmological model.
 * @param hydro_props The properties of the hydro scheme.
 * @param us The internal system of units.
 * @param p Pointer to the particle data.
 * @param xp Pointer to the extended data of the particle.
 */
__attribute__((always_inline)) INLINE static float cooling_timestep(
    const struct cooling_function_data* restrict cooling,
    const struct phys_const* restrict phys_const,
    const struct cosmology* restrict cosmo,
    const struct unit_system* restrict us,
    const struct hydro_props* hydro_props, const struct part* restrict p,
    const struct xpart* restrict xp) {

  /* Start with the case where there is no limit */
  if (cooling->cooling_tstep_mult == FLT_MAX) return FLT_MAX;

  /* Get current internal energy and cooling rate */
  const float u = hydro_get_physical_internal_energy(p, xp, cosmo);
  const double cooling_du_dt_cgs =
      cooling_rate_cgs(cosmo, hydro_props, cooling, p);

  /* Convert to internal units */
  const float cooling_du_dt =
      cooling_du_dt_cgs * cooling->conv_factor_energy_rate_from_cgs;

  /* If we are close to (or below) the limit, we ignore the condition */
  if (u < 1.01f * hydro_props->minimal_internal_energy || cooling_du_dt == 0.f)
    return FLT_MAX;
  else
    return cooling->cooling_tstep_mult * u / fabsf(cooling_du_dt);
}

/**
 * @brief Sets the cooling properties of the (x-)particles to a valid start
 * state.
 *
 * Nothing to do here. Just set the radiated energy counter to 0.
 *
 * @param phys_const The physical constants in internal units.
 * @param cooling The properties of the cooling function.
 * @param us The internal system of units.
 * @param cosmo The current cosmological model.
 * @param p Pointer to the particle data.
 * @param xp Pointer to the extended particle data.
 */
__attribute__((always_inline)) INLINE static void cooling_first_init_part(
    const struct phys_const* restrict phys_const,
    const struct unit_system* restrict us,
    const struct cosmology* restrict cosmo,
    const struct cooling_function_data* restrict cooling,
    const struct part* restrict p, struct xpart* restrict xp) {

  xp->cooling_data.radiated_energy = 0.f;
}

/**
 * @brief Returns the total radiated energy by this particle.
 *
 * @param xp The extended particle data
 */
__attribute__((always_inline)) INLINE static float cooling_get_radiated_energy(
    const struct xpart* restrict xp) {

  return xp->cooling_data.radiated_energy;
}

/**
 * @brief Initialises the cooling properties.
 *
 * @param parameter_file The parsed parameter file.
 * @param us The current internal system of units.
 * @param phys_const The physical constants in internal units.
 * @param cooling The cooling properties to initialize
 */
static INLINE void cooling_init_backend(struct swift_params* parameter_file,
                                        const struct unit_system* us,
                                        const struct phys_const* phys_const,
                                        struct cooling_function_data* cooling) {

  /* Read in the cooling parameters */
  cooling->lambda_cgs =
      parser_get_param_double(parameter_file, "LambdaCooling:lambda_cgs");
  cooling->cooling_tstep_mult = parser_get_opt_param_float(
      parameter_file, "LambdaCooling:cooling_tstep_mult", FLT_MAX);

  /* Some useful conversion values */
  cooling->conv_factor_density_to_cgs =
      units_cgs_conversion_factor(us, UNIT_CONV_DENSITY);
  cooling->conv_factor_energy_rate_from_cgs =
      units_cgs_conversion_factor(us, UNIT_CONV_TIME) /
      units_cgs_conversion_factor(us, UNIT_CONV_ENERGY_PER_UNIT_MASS);

  /* Useful constants */
  cooling->proton_mass_cgs_inv =
      1. / (phys_const->const_proton_mass *
            units_cgs_conversion_factor(us, UNIT_CONV_MASS));
}

/**
 * @brief Prints the properties of the cooling model to stdout.
 *
 * @param cooling The properties of the cooling function.
 */
static INLINE void cooling_print_backend(
    const struct cooling_function_data* cooling) {

  message(
      "Cooling function is 'Constant lambda' with Lambda=%g [erg * s^-1 * "
      "cm^-3]",
      cooling->lambda_cgs);

  if (cooling->cooling_tstep_mult == FLT_MAX)
    message("Cooling function time-step size is unlimited");
  else
    message("Cooling function time-step size limited to %f of u/(du/dt)",
            cooling->cooling_tstep_mult);
}

#endif /* SWIFT_COOLING_COMPTON_H */