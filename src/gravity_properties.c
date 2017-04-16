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

/* This object's header. */
#include "gravity_properties.h"

/* Standard headers */
#include <float.h>
#include <math.h>

/* Local headers. */
#include "adiabatic_index.h"
#include "common_io.h"
#include "dimension.h"
#include "error.h"
#include "gravity.h"
#include "kernel_gravity.h"

#define gravity_props_default_a_smooth 1.25f
#define gravity_props_default_r_cut 4.5f

void gravity_props_init(struct gravity_props *p,
                        const struct swift_params *params) {

  /* Tree-PM parameters */
  p->a_smooth = parser_get_opt_param_float(params, "Gravity:a_smooth",
                                           gravity_props_default_a_smooth);
  p->r_cut = parser_get_opt_param_float(params, "Gravity:r_cut",
                                        gravity_props_default_r_cut);

  /* Time integration */
  p->eta = parser_get_param_float(params, "Gravity:eta");

  /* Softening lengths */
  p->epsilon = parser_get_param_double(params, "Gravity:epsilon");
  p->epsilon2 = p->epsilon * p->epsilon;
  p->epsilon_inv = 1. / p->epsilon;
}

void gravity_props_print(const struct gravity_props *p) {

  message("Self-gravity scheme: FMM-MM");

  message("Self-gravity time integration: eta=%.4f", p->eta);

  message("Self-gravity softening: epsilon=%.4f", p->epsilon);

  if (p->a_smooth != gravity_props_default_a_smooth)
    message("Self-gravity smoothing-scale: a_smooth=%f", p->a_smooth);

  if (p->r_cut != gravity_props_default_r_cut)
    message("Self-gravity MM cut-off: r_cut=%f", p->r_cut);
}

#if defined(HAVE_HDF5)
void gravity_props_print_snapshot(hid_t h_grpgrav,
                                  const struct gravity_props *p) {

  io_write_attribute_f(h_grpgrav, "Time integration eta", p->eta);
  io_write_attribute_f(h_grpgrav, "Softening", p->epsilon);
  io_write_attribute_f(h_grpgrav, "MM a_smooth", p->a_smooth);
  io_write_attribute_f(h_grpgrav, "MM r_cut", p->r_cut);
}
#endif
