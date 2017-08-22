"""
###############################################################################
# This file is part of SWIFT.
# Copyright (c) 2017
#
# Josh Borrow (joshua.borrow@durham.ac.uk)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################
"""

import numpy as np
from scipy.special import erfinv


def inverse_gaussian(m_i, central_radius, standard_deviation):
    """
    The inverse of the Gaussian PDF used for generating the radial positions of
    the particles.

    @param: m_i | float / array-like
        - the m_i that are to be used to generate the radii of the particles

    @param: central_radius | float
        - central radius of the guassian

    @param: standard_deviation | float
        - standard deviation of the gaussian.

    ---------------------------------------------------------------------------

    @return: radius | float / array-like
        - the radius associated with m_i (see README for theory).
    """
    error_function = erfinv(2 * m_i - 1)
    radius = central_radius + standard_deviation * np.sqrt(2) * error_function

    return radius


def generate_m_i(n_particles):
    """
    Generate the m_i for each particle

    @param: n_particles | int
        - number of m_i to generate, or equivalently the number of particles.

    ---------------------------------------------------------------------------

    @return: m_i | numpy.array
        - the m_i that are used to generate the radii of each particle.
    """
    m_i = (np.arange(n_particles) + 0.5) / n_particles

    return m_i


def generate_theta_i(r_i, theta_initial=0.):
    """
    Generate the theta associated with the particles based on their radii.

    @param: r_i | float / array-like
        - the radii of the particles.

    @param: theta_initial | float | optional
        - initial radius to start the generation at.

    ---------------------------------------------------------------------------

    @return: theta_i | numpy.array
        - angles associated with the particles in the plane.
    """
    radii_fraction = r_i[:-1] / r_i[1:]
    d_theta_i = np.sqrt(2 * np.pi * (1 - radii_fraction))

    theta_i = np.empty_like(r_i)
    theta_i[0] = theta_initial

    for i in range(len(d_theta_i)):  # first is theta_initial
        theta_i[i+1] = theta_i[i] + d_theta_i[i]

    return theta_i


def convert_polar_to_cartesian(r_i, theta_i):
    """
    Calculate the cartesian co-ordinates (to be used to store in the GADGET
    file) from the polar co-ordinates generated by the script.abs

    @param: r_i | float / array-like
        - the radii of the particles

    @param: theta_i | float / array-like
        - the polar angle co-ordinate of the particles

    ---------------------------------------------------------------------------

    @return: x_i | float / array-like
        - the x co-ordinates of the particles

    @return: y_i | float / array-like
        - the y co-ordinates of the particles
    """
    x_i = r_i * np.cos(theta_i)
    y_i = r_i * np.sin(theta_i)

    return x_i, y_i


def generate_particles(n_particles, central_radius, standard_deviation):
    """
    A quick wrapper function that generates the x and y co-ordinates of
    particles in a keplerian ring.

    @param: n_particles | int
        - the number of particles in the ring
    
    @param: central_radius | float
        - the radius around which the particles are arranged

    @param: standard_deviation | float
        - the standard deviation of the gaussian which determines how the
          particles are arranged horizontally and vertically around the ring.

    ---------------------------------------------------------------------------

    @return: x_i | numpy.array
        - the x co-ordinates of the particles in the ring

    @return: y_i | numpy.array
        - the y co-ordinates of the particles in the ring
    """
    m_i = generate_m_i(n_particles)
    r_i = inverse_gaussian(m_i, central_radius, standard_deviation)
    theta_i = generate_theta_i(r_i)

    return convert_polar_to_cartesian(r_i, theta_i)


if __name__ == "__main__":
    # Check the particles are arrangd how we thought they were!
    import matplotlib.pyplot as plt

    x, y = generate_particles(10000, 10, 2.5)

    fig = plt.figure(figsize=(6, 6))
    ax = fig.add_subplot(111)

    ax.scatter(x, y, s=1)
    ax.set_xlim(-20, 20)
    ax.set_ylim(-20, 20)

    fig.show()
    input()  # keep the figure alive
