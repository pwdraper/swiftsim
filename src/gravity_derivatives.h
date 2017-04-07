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
#ifndef SWIFT_GRAVITY_DERIVATIVE_H
#define SWIFT_GRAVITY_DERIVATIVE_H

/**
 * @file gravity_derivatives.h
 * @brief Derivatives (up to 3rd order) of the gravitational potential.
 *
 * We use the notation of Dehnen, Computational Astrophysics and Cosmology,
 * 1, 1, pp. 24 (2014), arXiv:1405.2255
 */

/* Some standard headers. */
#include <math.h>

/* Local headers. */
#include "inline.h"

/*************************/
/* 0th order derivatives */
/*************************/

/**
 * @brief \f$ \phi(r_x, r_y, r_z) \f$.
 *
 * @param r_x x-coordinate of the distance vector (\f$ r_x \f$).
 * @param r_y y-coordinate of the distance vector (\f$ r_y \f$).
 * @param r_z z-coordinate of the distance vector (\f$ r_z \f$).
 * @param r_inv Inverse of the norm of the distance vector (\f$ |r|^{-1} \f$)
 */
__attribute__((always_inline)) INLINE static double D_000(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {

  return r_inv;
}

/*************************/
/* 1st order derivatives */
/*************************/

/**
 * @brief \f$ \frac{\partial\phi(r_x, r_y, r_z)}{\partial r_x} \f$.
 *
 * @param r_x x-coordinate of the distance vector (\f$ r_x \f$).
 * @param r_y y-coordinate of the distance vector (\f$ r_y \f$).
 * @param r_z z-coordinate of the distance vector (\f$ r_z \f$).
 * @param r_inv Inverse of the norm of the distance vector (\f$ |r|^{-1} \f$)
 */
__attribute__((always_inline)) INLINE static double D_100(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {

  return -r_x * r_inv * r_inv * r_inv;
}

/**
 * @brief \f$ \frac{\partial\phi(r_x, r_y, r_z)}{\partial r_x} \f$.
 *
 * @param r_x x-coordinate of the distance vector (\f$ r_x \f$).
 * @param r_y y-coordinate of the distance vector (\f$ r_y \f$).
 * @param r_z z-coordinate of the distance vector (\f$ r_z \f$).
 * @param r_inv Inverse of the norm of the distance vector (\f$ |r|^{-1} \f$)
 */
__attribute__((always_inline)) INLINE static double D_010(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {

  return -r_y * r_inv * r_inv * r_inv;
}

/**
 * @brief \f$ \frac{\partial\phi(r_x, r_y, r_z)}{\partial r_x} \f$.
 *
 * @param r_x x-coordinate of the distance vector (\f$ r_x \f$).
 * @param r_y y-coordinate of the distance vector (\f$ r_y \f$).
 * @param r_z z-coordinate of the distance vector (\f$ r_z \f$).
 * @param r_inv Inverse of the norm of the distance vector (\f$ |r|^{-1} \f$)
 */
__attribute__((always_inline)) INLINE static double D_001(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {

  return -r_z * r_inv * r_inv * r_inv;
}

/*************************/
/* 2nd order derivatives */
/*************************/

/**
 * @brief \f$ \frac{\partial^2\phi(r_x, r_y, r_z)}{\partial r_x^2} \f$.
 *
 * @param r_x x-coordinate of the distance vector (\f$ r_x \f$).
 * @param r_y y-coordinate of the distance vector (\f$ r_y \f$).
 * @param r_z z-coordinate of the distance vector (\f$ r_z \f$).
 * @param r_inv Inverse of the norm of the distance vector (\f$ |r|^{-1} \f$)
 */
__attribute__((always_inline)) INLINE static double D_200(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  const double r_inv2 = r_inv * r_inv;
  const double r_inv3 = r_inv * r_inv2;
  const double r_inv5 = r_inv3 * r_inv2;
  return 3. * r_x * r_x * r_inv5 - r_inv3;
}

/**
 * @brief \f$ \frac{\partial^2\phi(r_x, r_y, r_z)}{\partial r_y^2} \f$.
 *
 * @param r_x x-coordinate of the distance vector (\f$ r_x \f$).
 * @param r_y y-coordinate of the distance vector (\f$ r_y \f$).
 * @param r_z z-coordinate of the distance vector (\f$ r_z \f$).
 * @param r_inv Inverse of the norm of the distance vector (\f$ |r|^{-1} \f$)
 */
__attribute__((always_inline)) INLINE static double D_020(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  const double r_inv2 = r_inv * r_inv;
  const double r_inv3 = r_inv * r_inv2;
  const double r_inv5 = r_inv3 * r_inv2;
  return 3. * r_y * r_y * r_inv5 - r_inv3;
}

/**
 * @brief \f$ \frac{\partial^2\phi(r_x, r_y, r_z)}{\partial r_z^2} \f$.
 *
 * @param r_x x-coordinate of the distance vector (\f$ r_x \f$).
 * @param r_y y-coordinate of the distance vector (\f$ r_y \f$).
 * @param r_z z-coordinate of the distance vector (\f$ r_z \f$).
 * @param r_inv Inverse of the norm of the distance vector (\f$ |r|^{-1} \f$)
 */
__attribute__((always_inline)) INLINE static double D_002(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  const double r_inv2 = r_inv * r_inv;
  const double r_inv3 = r_inv * r_inv2;
  const double r_inv5 = r_inv3 * r_inv2;
  return 3. * r_z * r_z * r_inv5 - r_inv3;
}

/**
 * @brief \f$ \frac{\partial^2\phi(r_x, r_y, r_z)}{\partial r_x\partial r_y}
 * \f$.
 *
 * @param r_x x-coordinate of the distance vector (\f$ r_x \f$).
 * @param r_y y-coordinate of the distance vector (\f$ r_y \f$).
 * @param r_z z-coordinate of the distance vector (\f$ r_z \f$).
 * @param r_inv Inverse of the norm of the distance vector (\f$ |r|^{-1} \f$)
 */
__attribute__((always_inline)) INLINE static double D_110(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  const double r_inv2 = r_inv * r_inv;
  const double r_inv5 = r_inv2 * r_inv2 * r_inv;
  return 3. * r_x * r_y * r_inv5;
}

/**
 * @brief \f$ \frac{\partial^2\phi(r_x, r_y, r_z)}{\partial r_x\partial r_z}
 * \f$.
 *
 * @param r_x x-coordinate of the distance vector (\f$ r_x \f$).
 * @param r_y y-coordinate of the distance vector (\f$ r_y \f$).
 * @param r_z z-coordinate of the distance vector (\f$ r_z \f$).
 * @param r_inv Inverse of the norm of the distance vector (\f$ |r|^{-1} \f$)
 */
__attribute__((always_inline)) INLINE static double D_101(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  const double r_inv2 = r_inv * r_inv;
  const double r_inv5 = r_inv2 * r_inv2 * r_inv;
  return 3. * r_x * r_z * r_inv5;
}

/**
 * @brief \f$ \frac{\partial^2\phi(r_x, r_y, r_z)}{\partial r_y\partial r_z}
 * \f$.
 *
 * @param r_x x-coordinate of the distance vector (\f$ r_x \f$).
 * @param r_y y-coordinate of the distance vector (\f$ r_y \f$).
 * @param r_z z-coordinate of the distance vector (\f$ r_z \f$).
 * @param r_inv Inverse of the norm of the distance vector (\f$ |r|^{-1} \f$)
 */
__attribute__((always_inline)) INLINE static double D_011(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  const double r_inv2 = r_inv * r_inv;
  const double r_inv5 = r_inv2 * r_inv2 * r_inv;
  return 3. * r_y * r_z * r_inv5;
}

/*************************/
/* 3rd order derivatives */
/*************************/

/**
 * @brief \f$ \frac{\partial^3\phi(r_x, r_y, r_z)}{\partial r_x^3} \f$.
 *
 * @param r_x x-coordinate of the distance vector (\f$ r_x \f$).
 * @param r_y y-coordinate of the distance vector (\f$ r_y \f$).
 * @param r_z z-coordinate of the distance vector (\f$ r_z \f$).
 * @param r_inv Inverse of the norm of the distance vector (\f$ |r|^{-1} \f$)
 */
__attribute__((always_inline)) INLINE static double D_300(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  const double r_inv2 = r_inv * r_inv;
  const double r_inv5 = r_inv2 * r_inv2 * r_inv;
  const double r_inv7 = r_inv5 * r_inv2;
  return -15. * r_x * r_x * r_x * r_inv7 + 9. * r_x * r_inv5;
}

/**
 * @brief \f$ \frac{\partial^3\phi(r_x, r_y, r_z)}{\partial r_y^3} \f$.
 *
 * @param r_x x-coordinate of the distance vector (\f$ r_x \f$).
 * @param r_y y-coordinate of the distance vector (\f$ r_y \f$).
 * @param r_z z-coordinate of the distance vector (\f$ r_z \f$).
 * @param r_inv Inverse of the norm of the distance vector (\f$ |r|^{-1} \f$)
 */
__attribute__((always_inline)) INLINE static double D_030(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  const double r_inv2 = r_inv * r_inv;
  const double r_inv5 = r_inv2 * r_inv2 * r_inv;
  const double r_inv7 = r_inv5 * r_inv2;
  return -15. * r_y * r_y * r_y * r_inv7 + 9. * r_y * r_inv5;
}

/**
 * @brief \f$ \frac{\partial^3\phi(r_x, r_y, r_z)}{\partial r_z^3} \f$.
 *
 * @param r_x x-coordinate of the distance vector (\f$ r_x \f$).
 * @param r_y y-coordinate of the distance vector (\f$ r_y \f$).
 * @param r_z z-coordinate of the distance vector (\f$ r_z \f$).
 * @param r_inv Inverse of the norm of the distance vector (\f$ |r|^{-1} \f$)
 */
__attribute__((always_inline)) INLINE static double D_003(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  const double r_inv2 = r_inv * r_inv;
  const double r_inv5 = r_inv2 * r_inv2 * r_inv;
  const double r_inv7 = r_inv5 * r_inv2;
  return -15. * r_z * r_z * r_z * r_inv7 + 9. * r_z * r_inv5;
}

/**
 * @brief \f$ \frac{\partial^3\phi(r_x, r_y, r_z)}{\partial r_x^2\partial r_y}
 * \f$.
 *
 * @param r_x x-coordinate of the distance vector (\f$ r_x \f$).
 * @param r_y y-coordinate of the distance vector (\f$ r_y \f$).
 * @param r_z z-coordinate of the distance vector (\f$ r_z \f$).
 * @param r_inv Inverse of the norm of the distance vector (\f$ |r|^{-1} \f$)
 */
__attribute__((always_inline)) INLINE static double D_210(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  const double r_inv2 = r_inv * r_inv;
  const double r_inv5 = r_inv2 * r_inv2 * r_inv;
  const double r_inv7 = r_inv5 * r_inv2;
  return -15. * r_x * r_x * r_y * r_inv7 + 3. * r_y * r_inv5;
}

/**
 * @brief \f$ \frac{\partial^3\phi(r_x, r_y, r_z)}{\partial r_x^2\partial r_z}
 * \f$.
 *
 * @param r_x x-coordinate of the distance vector (\f$ r_x \f$).
 * @param r_y y-coordinate of the distance vector (\f$ r_y \f$).
 * @param r_z z-coordinate of the distance vector (\f$ r_z \f$).
 * @param r_inv Inverse of the norm of the distance vector (\f$ |r|^{-1} \f$)
 */
__attribute__((always_inline)) INLINE static double D_201(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  const double r_inv2 = r_inv * r_inv;
  const double r_inv5 = r_inv2 * r_inv2 * r_inv;
  const double r_inv7 = r_inv5 * r_inv2;
  return -15. * r_x * r_x * r_z * r_inv7 + 3. * r_z * r_inv5;
}

/**
 * @brief \f$ \frac{\partial^3\phi(r_x, r_y, r_z)}{\partial r_x\partial r_y^2}
 * \f$.
 *
 * @param r_x x-coordinate of the distance vector (\f$ r_x \f$).
 * @param r_y y-coordinate of the distance vector (\f$ r_y \f$).
 * @param r_z z-coordinate of the distance vector (\f$ r_z \f$).
 * @param r_inv Inverse of the norm of the distance vector (\f$ |r|^{-1} \f$)
 */
__attribute__((always_inline)) INLINE static double D_120(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  const double r_inv2 = r_inv * r_inv;
  const double r_inv5 = r_inv2 * r_inv2 * r_inv;
  const double r_inv7 = r_inv5 * r_inv2;
  return -15. * r_x * r_y * r_y * r_inv7 + 3. * r_x * r_inv5;
}

/**
 * @brief \f$ \frac{\partial^3\phi(r_x, r_y, r_z)}{\partial r_y^2\partial r_z}
 * \f$.
 *
 * @param r_x x-coordinate of the distance vector (\f$ r_x \f$).
 * @param r_y y-coordinate of the distance vector (\f$ r_y \f$).
 * @param r_z z-coordinate of the distance vector (\f$ r_z \f$).
 * @param r_inv Inverse of the norm of the distance vector (\f$ |r|^{-1} \f$)
 */
__attribute__((always_inline)) INLINE static double D_021(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  const double r_inv2 = r_inv * r_inv;
  const double r_inv5 = r_inv2 * r_inv2 * r_inv;
  const double r_inv7 = r_inv5 * r_inv2;
  return -15. * r_z * r_y * r_y * r_inv7 + 3. * r_z * r_inv5;
}

/**
 * @brief \f$ \frac{\partial^3\phi(r_x, r_y, r_z)}{\partial r_x\partial r_z^2}
 * \f$.
 *
 * @param r_x x-coordinate of the distance vector (\f$ r_x \f$).
 * @param r_y y-coordinate of the distance vector (\f$ r_y \f$).
 * @param r_z z-coordinate of the distance vector (\f$ r_z \f$).
 * @param r_inv Inverse of the norm of the distance vector (\f$ |r|^{-1} \f$)
 */
__attribute__((always_inline)) INLINE static double D_102(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  const double r_inv2 = r_inv * r_inv;
  const double r_inv5 = r_inv2 * r_inv2 * r_inv;
  const double r_inv7 = r_inv5 * r_inv2;
  return -15. * r_x * r_z * r_z * r_inv7 + 3. * r_x * r_inv5;
}

/**
 * @brief \f$ \frac{\partial^3\phi(r_x, r_y, r_z)}{\partial r_y\partial r_z^2}
 * \f$.
 *
 * @param r_x x-coordinate of the distance vector (\f$ r_x \f$).
 * @param r_y y-coordinate of the distance vector (\f$ r_y \f$).
 * @param r_z z-coordinate of the distance vector (\f$ r_z \f$).
 * @param r_inv Inverse of the norm of the distance vector (\f$ |r|^{-1} \f$)
 */
__attribute__((always_inline)) INLINE static double D_012(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  const double r_inv2 = r_inv * r_inv;
  const double r_inv5 = r_inv2 * r_inv2 * r_inv;
  const double r_inv7 = r_inv5 * r_inv2;
  return -15. * r_y * r_z * r_z * r_inv7 + 3. * r_y * r_inv5;
}

/**
 * @brief \f$ \frac{\partial^3\phi(r_x, r_y, r_z)}{\partial r_z\partial
 * r_y\partial r_z} \f$.
 *
 * @param r_x x-coordinate of the distance vector (\f$ r_x \f$).
 * @param r_y y-coordinate of the distance vector (\f$ r_y \f$).
 * @param r_z z-coordinate of the distance vector (\f$ r_z \f$).
 * @param r_inv Inverse of the norm of the distance vector (\f$ |r|^{-1} \f$)
 */
__attribute__((always_inline)) INLINE static double D_111(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  const double r_inv3 = r_inv * r_inv * r_inv;
  const double r_inv7 = r_inv3 * r_inv3 * r_inv;
  return -15. * r_x * r_y * r_z * r_inv7;
}

/*********************************/
/* 4th order gravity derivatives */
/*********************************/

/**
 * @brief Compute \f$ \frac{\partial^4}{ \partial_z^4 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_004(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return +105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * (r_z * r_z * r_z * r_z) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0 *
             (r_z * r_z) +
         3. * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0;
  /* 5 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^4}{ \partial_y^1 \partial_z^3 }\phi(x, y,
 * z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_013(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return +105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * (r_y * r_z * r_z * r_z) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0 *
             (r_y * r_z);
  /* 11 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^4}{ \partial_y^2 \partial_z^2 }\phi(x, y,
 * z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_022(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return +105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * (r_y * r_y * r_z * r_z) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             (r_y * r_y) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             (r_z * r_z) +
         3. * r_inv * r_inv * r_inv * r_inv * r_inv;
  /* 11 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^4}{ \partial_y^3 \partial_z^1 }\phi(x, y,
 * z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_031(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return +105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * (r_y * r_y * r_y * r_z) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0 *
             (r_y * r_z);
  /* 11 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^4}{ \partial_y^4 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_040(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return +105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * (r_y * r_y * r_y * r_y) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0 *
             (r_y * r_y) +
         3. * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0;
  /* 5 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^4}{ \partial_x^1 \partial_z^3 }\phi(x, y,
 * z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_103(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return +105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * (r_x * r_z * r_z * r_z) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0 *
             (r_x * r_z);
  /* 11 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^4}{ \partial_x^1 \partial_y^1 \partial_z^2
 * }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_112(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return +105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * (r_x * r_y * r_z * r_z) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             (r_x * r_y);
  /* 13 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^4}{ \partial_x^1 \partial_y^2 \partial_z^1
 * }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_121(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return +105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * (r_x * r_y * r_y * r_z) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             (r_x * r_z);
  /* 13 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^4}{ \partial_x^1 \partial_y^3 }\phi(x, y,
 * z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_130(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return +105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * (r_x * r_y * r_y * r_y) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0 *
             (r_x * r_y);
  /* 11 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^4}{ \partial_x^2 \partial_z^2 }\phi(x, y,
 * z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_202(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return +105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * (r_x * r_x * r_z * r_z) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             (r_x * r_x) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             (r_z * r_z) +
         3. * r_inv * r_inv * r_inv * r_inv * r_inv;
  /* 11 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^4}{ \partial_x^2 \partial_y^1 \partial_z^1
 * }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_211(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return +105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * (r_x * r_x * r_y * r_z) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             (r_y * r_z);
  /* 13 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^4}{ \partial_x^2 \partial_y^2 }\phi(x, y,
 * z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_220(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return +105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * (r_x * r_x * r_y * r_y) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             (r_x * r_x) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             (r_y * r_y) +
         3. * r_inv * r_inv * r_inv * r_inv * r_inv;
  /* 11 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^4}{ \partial_x^3 \partial_z^1 }\phi(x, y,
 * z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_301(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return +105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * (r_x * r_x * r_x * r_z) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0 *
             (r_x * r_z);
  /* 11 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^4}{ \partial_x^3 \partial_y^1 }\phi(x, y,
 * z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_310(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return +105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * (r_x * r_x * r_x * r_y) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0 *
             (r_x * r_y);
  /* 11 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^4}{ \partial_x^4 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_400(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return +105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * (r_x * r_x * r_x * r_x) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0 *
             (r_x * r_x) +
         3. * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0;
  /* 5 zero-valued terms not written out */
}

/*********************************/
/* 5th order gravity derivatives */
/*********************************/

/**
 * @brief Compute \f$ \frac{\partial^5}{ \partial_z^5 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_005(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return -945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * r_inv * r_inv * (r_z * r_z * r_z * r_z * r_z) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * 10.0 * (r_z * r_z * r_z) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0 *
             (r_z);
  /* 26 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^5}{ \partial_y^1 \partial_z^4 }\phi(x, y,
 * z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_014(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return -945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * r_inv * r_inv * (r_y * r_z * r_z * r_z * r_z) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * 6.0 * (r_y * r_z * r_z) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0 *
             (r_y);
  /* 42 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^5}{ \partial_y^2 \partial_z^3 }\phi(x, y,
 * z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_023(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return -945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * r_inv * r_inv * (r_y * r_y * r_z * r_z * r_z) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * 3.0 * (r_y * r_y * r_z) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * (r_z * r_z * r_z) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0 *
             (r_z);
  /* 44 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^5}{ \partial_y^3 \partial_z^2 }\phi(x, y,
 * z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_032(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return -945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * r_inv * r_inv * (r_y * r_y * r_y * r_z * r_z) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * (r_y * r_y * r_y) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * 3.0 * (r_y * r_z * r_z) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0 *
             (r_y);
  /* 44 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^5}{ \partial_y^4 \partial_z^1 }\phi(x, y,
 * z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_041(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return -945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * r_inv * r_inv * (r_y * r_y * r_y * r_y * r_z) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * 6.0 * (r_y * r_y * r_z) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0 *
             (r_z);
  /* 42 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^5}{ \partial_y^5 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_050(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return -945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * r_inv * r_inv * (r_y * r_y * r_y * r_y * r_y) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * 10.0 * (r_y * r_y * r_y) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0 *
             (r_y);
  /* 26 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^5}{ \partial_x^1 \partial_z^4 }\phi(x, y,
 * z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_104(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return -945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * r_inv * r_inv * (r_x * r_z * r_z * r_z * r_z) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * 6.0 * (r_x * r_z * r_z) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0 *
             (r_x);
  /* 42 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^5}{ \partial_x^1 \partial_y^1 \partial_z^3
 * }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_113(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return -945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * r_inv * r_inv * (r_x * r_y * r_z * r_z * r_z) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * 3.0 * (r_x * r_y * r_z);
  /* 48 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^5}{ \partial_x^1 \partial_y^2 \partial_z^2
 * }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_122(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return -945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * r_inv * r_inv * (r_x * r_y * r_y * r_z * r_z) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * (r_x * r_y * r_y) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * (r_x * r_z * r_z) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * (r_x);
  /* 48 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^5}{ \partial_x^1 \partial_y^3 \partial_z^1
 * }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_131(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return -945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * r_inv * r_inv * (r_x * r_y * r_y * r_y * r_z) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * 3.0 * (r_x * r_y * r_z);
  /* 48 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^5}{ \partial_x^1 \partial_y^4 }\phi(x, y,
 * z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_140(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return -945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * r_inv * r_inv * (r_x * r_y * r_y * r_y * r_y) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * 6.0 * (r_x * r_y * r_y) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0 *
             (r_x);
  /* 42 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^5}{ \partial_x^2 \partial_z^3 }\phi(x, y,
 * z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_203(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return -945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * r_inv * r_inv * (r_x * r_x * r_z * r_z * r_z) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * 3.0 * (r_x * r_x * r_z) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * (r_z * r_z * r_z) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0 *
             (r_z);
  /* 44 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^5}{ \partial_x^2 \partial_y^1 \partial_z^2
 * }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_212(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return -945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * r_inv * r_inv * (r_x * r_x * r_y * r_z * r_z) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * (r_x * r_x * r_y) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * (r_y * r_z * r_z) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * (r_y);
  /* 48 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^5}{ \partial_x^2 \partial_y^2 \partial_z^1
 * }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_221(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return -945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * r_inv * r_inv * (r_x * r_x * r_y * r_y * r_z) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * (r_x * r_x * r_z) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * (r_y * r_y * r_z) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * (r_z);
  /* 48 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^5}{ \partial_x^2 \partial_y^3 }\phi(x, y,
 * z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_230(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return -945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * r_inv * r_inv * (r_x * r_x * r_y * r_y * r_y) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * 3.0 * (r_x * r_x * r_y) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * (r_y * r_y * r_y) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0 *
             (r_y);
  /* 44 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^5}{ \partial_x^3 \partial_z^2 }\phi(x, y,
 * z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_302(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return -945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * r_inv * r_inv * (r_x * r_x * r_x * r_z * r_z) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * (r_x * r_x * r_x) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * 3.0 * (r_x * r_z * r_z) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0 *
             (r_x);
  /* 44 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^5}{ \partial_x^3 \partial_y^1 \partial_z^1
 * }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_311(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return -945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * r_inv * r_inv * (r_x * r_x * r_x * r_y * r_z) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * 3.0 * (r_x * r_y * r_z);
  /* 48 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^5}{ \partial_x^3 \partial_y^2 }\phi(x, y,
 * z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_320(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return -945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * r_inv * r_inv * (r_x * r_x * r_x * r_y * r_y) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * (r_x * r_x * r_x) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * 3.0 * (r_x * r_y * r_y) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0 *
             (r_x);
  /* 44 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^5}{ \partial_x^4 \partial_z^1 }\phi(x, y,
 * z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_401(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return -945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * r_inv * r_inv * (r_x * r_x * r_x * r_x * r_z) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * 6.0 * (r_x * r_x * r_z) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0 *
             (r_z);
  /* 42 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^5}{ \partial_x^4 \partial_y^1 }\phi(x, y,
 * z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_410(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return -945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * r_inv * r_inv * (r_x * r_x * r_x * r_x * r_y) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * 6.0 * (r_x * r_x * r_y) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0 *
             (r_y);
  /* 42 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^5}{ \partial_x^5 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_500(double r_x,
                                                          double r_y,
                                                          double r_z,
                                                          double r_inv) {
  return -945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * r_inv * r_inv * (r_x * r_x * r_x * r_x * r_x) +
         105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv *
             r_inv * 10.0 * (r_x * r_x * r_x) -
         15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0 *
             (r_x);
  /* 26 zero-valued terms not written out */
}
/*********************************/
/* 6th order gravity derivatives */
/*********************************/

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_z^6 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_006(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_z * r_z * r_z * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_z * r_z * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 45.0   * ( r_z * r_z ) - 15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0  ;
/* 127 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_y^1 \partial_z^5 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_015(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_y * r_z * r_z * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 10.0   * ( r_y * r_z * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_y * r_z ) ;
/* 177 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_y^2 \partial_z^4 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_024(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_y * r_y * r_z * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_y * r_y * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_y * r_y ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_z * r_z * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_z * r_z ) - 15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0  ;
/* 183 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_y^3 \partial_z^3 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_033(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_y * r_y * r_y * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_y * r_y * r_y * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_y * r_z * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 9.0   * ( r_y * r_z ) ;
/* 187 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_y^4 \partial_z^2 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_042(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_y * r_y * r_y * r_y * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_y * r_y * r_y * r_y ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_y * r_y * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_y * r_y ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_z * r_z ) - 15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0  ;
/* 183 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_y^5 \partial_z^1 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_051(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_y * r_y * r_y * r_y * r_y * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 10.0   * ( r_y * r_y * r_y * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_y * r_z ) ;
/* 177 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_y^6 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_060(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_y * r_y * r_y * r_y * r_y * r_y ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_y * r_y * r_y * r_y ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 45.0   * ( r_y * r_y ) - 15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0  ;
/* 127 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_x^1 \partial_z^5 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_105(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_z * r_z * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 10.0   * ( r_x * r_z * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_x * r_z ) ;
/* 177 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_x^1 \partial_y^1 \partial_z^4 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_114(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_y * r_z * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_x * r_y * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_y ) ;
/* 193 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_x^1 \partial_y^2 \partial_z^3 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_123(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_y * r_y * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_y * r_y * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_z * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_z ) ;
/* 195 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_x^1 \partial_y^3 \partial_z^2 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_132(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_y * r_y * r_y * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_y * r_y * r_y ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_y * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_y ) ;
/* 195 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_x^1 \partial_y^4 \partial_z^1 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_141(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_y * r_y * r_y * r_y * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_x * r_y * r_y * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_z ) ;
/* 193 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_x^1 \partial_y^5 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_150(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_y * r_y * r_y * r_y * r_y ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 10.0   * ( r_x * r_y * r_y * r_y ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_x * r_y ) ;
/* 177 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_x^2 \partial_z^4 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_204(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_z * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_x * r_x * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_x ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_z * r_z * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_z * r_z ) - 15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0  ;
/* 183 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_x^2 \partial_y^1 \partial_z^3 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_213(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_y * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_x * r_y * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_y * r_z * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_y * r_z ) ;
/* 195 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_x^2 \partial_y^2 \partial_z^2 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_222(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_y * r_y * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_y * r_y ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_y * r_y * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_y * r_y ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_z * r_z ) - 15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv ;
/* 195 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_x^2 \partial_y^3 \partial_z^1 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_231(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_y * r_y * r_y * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_x * r_y * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_y * r_y * r_y * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_y * r_z ) ;
/* 195 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_x^2 \partial_y^4 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_240(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_y * r_y * r_y * r_y ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_x * r_x * r_y * r_y ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_x ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_y * r_y * r_y * r_y ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_y * r_y ) - 15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0  ;
/* 183 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_x^3 \partial_z^3 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_303(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_x * r_x * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_z * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 9.0   * ( r_x * r_z ) ;
/* 187 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_x^3 \partial_y^1 \partial_z^2 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_312(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_y * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_y ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_y * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_y ) ;
/* 195 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_x^3 \partial_y^2 \partial_z^1 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_321(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_y * r_y * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_y * r_y * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_z ) ;
/* 195 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_x^3 \partial_y^3 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_330(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_y * r_y * r_y ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_x * r_x * r_y ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_y * r_y * r_y ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 9.0   * ( r_x * r_y ) ;
/* 187 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_x^4 \partial_z^2 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_402(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_x * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_x ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_x * r_x * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_x * r_x ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_z * r_z ) - 15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0  ;
/* 183 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_x^4 \partial_y^1 \partial_z^1 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_411(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_x * r_y * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_x * r_x * r_y * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_y * r_z ) ;
/* 193 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_x^4 \partial_y^2 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_420(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_x * r_y * r_y ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_x ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_x * r_x * r_y * r_y ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_x * r_x ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_y * r_y ) - 15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0  ;
/* 183 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_x^5 \partial_z^1 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_501(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_x * r_x * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 10.0   * ( r_x * r_x * r_x * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_x * r_z ) ;
/* 177 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_x^5 \partial_y^1 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_510(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_x * r_x * r_y ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 10.0   * ( r_x * r_x * r_x * r_y ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_x * r_y ) ;
/* 177 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^6}{ \partial_x^6 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_600(double r_x, double r_y, double r_z, double r_inv) {
return + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_x * r_x * r_x ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_x * r_x * r_x * r_x ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 45.0   * ( r_x * r_x ) - 15. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0  ;
/* 127 zero-valued terms not written out */
}

/*********************************/
/* 7th order gravity derivatives */
/*********************************/

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_z^7 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_007(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_z * r_z * r_z * r_z * r_z * r_z * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 21.0   * ( r_z * r_z * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 105.0   * ( r_z * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 105.0   * ( r_z ) ;
/* 645 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_y^1 \partial_z^6 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_016(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_y * r_z * r_z * r_z * r_z * r_z * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_y * r_z * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 45.0   * ( r_y * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_y ) ;
/* 801 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_y^2 \partial_z^5 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_025(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_y * r_y * r_z * r_z * r_z * r_z * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 10.0   * ( r_y * r_y * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_y * r_y * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_z * r_z * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 10.0   * ( r_z * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_z ) ;
/* 825 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_y^3 \partial_z^4 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_034(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_y * r_y * r_y * r_z * r_z * r_z * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_y * r_y * r_y * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_y * r_y * r_y ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_y * r_z * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 18.0   * ( r_y * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 9.0   * ( r_y ) ;
/* 837 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_y^4 \partial_z^3 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_043(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_y * r_y * r_y * r_y * r_z * r_z * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_y * r_y * r_y * r_y * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_y * r_y * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 18.0   * ( r_y * r_y * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_z * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 9.0   * ( r_z ) ;
/* 837 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_y^5 \partial_z^2 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_052(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_y * r_y * r_y * r_y * r_y * r_z * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_y * r_y * r_y * r_y * r_y ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 10.0   * ( r_y * r_y * r_y * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 10.0   * ( r_y * r_y * r_y ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_y * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_y ) ;
/* 825 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_y^6 \partial_z^1 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_061(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_y * r_y * r_y * r_y * r_y * r_y * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_y * r_y * r_y * r_y * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 45.0   * ( r_y * r_y * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_z ) ;
/* 801 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_y^7 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_070(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_y * r_y * r_y * r_y * r_y * r_y * r_y ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 21.0   * ( r_y * r_y * r_y * r_y * r_y ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 105.0   * ( r_y * r_y * r_y ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 105.0   * ( r_y ) ;
/* 645 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^1 \partial_z^6 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_106(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_z * r_z * r_z * r_z * r_z * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_x * r_z * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 45.0   * ( r_x * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_x ) ;
/* 801 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^1 \partial_y^1 \partial_z^5 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_115(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_y * r_z * r_z * r_z * r_z * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 10.0   * ( r_x * r_y * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_x * r_y * r_z ) ;
/* 851 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^1 \partial_y^2 \partial_z^4 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_124(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_y * r_y * r_z * r_z * r_z * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_x * r_y * r_y * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_y * r_y ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_z * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_x * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x ) ;
/* 857 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^1 \partial_y^3 \partial_z^3 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_133(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_y * r_y * r_y * r_z * r_z * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_y * r_y * r_y * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_y * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 9.0   * ( r_x * r_y * r_z ) ;
/* 861 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^1 \partial_y^4 \partial_z^2 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_142(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_y * r_y * r_y * r_y * r_z * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_y * r_y * r_y * r_y ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_x * r_y * r_y * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_x * r_y * r_y ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x ) ;
/* 857 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^1 \partial_y^5 \partial_z^1 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_151(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_y * r_y * r_y * r_y * r_y * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 10.0   * ( r_x * r_y * r_y * r_y * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_x * r_y * r_z ) ;
/* 851 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^1 \partial_y^6 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_160(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_y * r_y * r_y * r_y * r_y * r_y ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_x * r_y * r_y * r_y * r_y ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 45.0   * ( r_x * r_y * r_y ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_x ) ;
/* 801 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^2 \partial_z^5 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_205(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_z * r_z * r_z * r_z * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 10.0   * ( r_x * r_x * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_x * r_x * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_z * r_z * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 10.0   * ( r_z * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_z ) ;
/* 825 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^2 \partial_y^1 \partial_z^4 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_214(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_y * r_z * r_z * r_z * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_x * r_x * r_y * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_x * r_y ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_y * r_z * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_y * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_y ) ;
/* 857 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^2 \partial_y^2 \partial_z^3 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_223(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_y * r_y * r_z * r_z * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_x * r_y * r_y * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_x * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_y * r_y * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_y * r_y * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_z * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_z ) ;
/* 861 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^2 \partial_y^3 \partial_z^2 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_232(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_y * r_y * r_y * r_z * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_y * r_y * r_y ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_x * r_y * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_x * r_y ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_y * r_y * r_y * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_y * r_y * r_y ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_y * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_y ) ;
/* 861 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^2 \partial_y^4 \partial_z^1 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_241(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_y * r_y * r_y * r_y * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_x * r_x * r_y * r_y * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_x * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_y * r_y * r_y * r_y * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_y * r_y * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_z ) ;
/* 857 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^2 \partial_y^5 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_250(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_y * r_y * r_y * r_y * r_y ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 10.0   * ( r_x * r_x * r_y * r_y * r_y ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_x * r_x * r_y ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_y * r_y * r_y * r_y * r_y ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 10.0   * ( r_y * r_y * r_y ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_y ) ;
/* 825 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^3 \partial_z^4 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_304(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_z * r_z * r_z * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_x * r_x * r_x * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_x * r_x ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_z * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 18.0   * ( r_x * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 9.0   * ( r_x ) ;
/* 837 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^3 \partial_y^1 \partial_z^3 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_313(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_y * r_z * r_z * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_x * r_x * r_y * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_y * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 9.0   * ( r_x * r_y * r_z ) ;
/* 861 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^3 \partial_y^2 \partial_z^2 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_322(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_y * r_y * r_z * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_y * r_y ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_y * r_y * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_y * r_y ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x ) ;
/* 861 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^3 \partial_y^3 \partial_z^1 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_331(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_y * r_y * r_y * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_x * r_x * r_y * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_y * r_y * r_y * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 9.0   * ( r_x * r_y * r_z ) ;
/* 861 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^3 \partial_y^4 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_340(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_y * r_y * r_y * r_y ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_x * r_x * r_x * r_y * r_y ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_x * r_x ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_y * r_y * r_y * r_y ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 18.0   * ( r_x * r_y * r_y ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 9.0   * ( r_x ) ;
/* 837 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^4 \partial_z^3 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_403(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_x * r_z * r_z * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_x * r_x * r_x * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_x * r_x * r_z * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 18.0   * ( r_x * r_x * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_z * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 9.0   * ( r_z ) ;
/* 837 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^4 \partial_y^1 \partial_z^2 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_412(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_x * r_y * r_z * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_x * r_y ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_x * r_x * r_y * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_x * r_x * r_y ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_y * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_y ) ;
/* 857 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^4 \partial_y^2 \partial_z^1 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_421(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_x * r_y * r_y * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_x * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_x * r_x * r_y * r_y * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_x * r_x * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_y * r_y * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_z ) ;
/* 857 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^4 \partial_y^3 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_430(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_x * r_y * r_y * r_y ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_x * r_x * r_x * r_x * r_y ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 6.0   * ( r_x * r_x * r_y * r_y * r_y ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 18.0   * ( r_x * r_x * r_y ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 3.0   * ( r_y * r_y * r_y ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 9.0   * ( r_y ) ;
/* 837 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^5 \partial_z^2 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_502(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_x * r_x * r_z * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_x * r_x ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 10.0   * ( r_x * r_x * r_x * r_z * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 10.0   * ( r_x * r_x * r_x ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_x * r_z * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_x ) ;
/* 825 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^5 \partial_y^1 \partial_z^1 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_511(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_x * r_x * r_y * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 10.0   * ( r_x * r_x * r_x * r_y * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_x * r_y * r_z ) ;
/* 851 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^5 \partial_y^2 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_520(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_x * r_x * r_y * r_y ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_x * r_x ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 10.0   * ( r_x * r_x * r_x * r_y * r_y ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 10.0   * ( r_x * r_x * r_x ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_x * r_y * r_y ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_x ) ;
/* 825 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^6 \partial_z^1 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_601(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_x * r_x * r_x * r_z ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_x * r_x * r_x * r_x * r_z ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 45.0   * ( r_x * r_x * r_z ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_z ) ;
/* 801 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^6 \partial_y^1 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_610(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_x * r_x * r_x * r_y ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_x * r_x * r_x * r_x * r_y ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 45.0   * ( r_x * r_x * r_y ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 15.0   * ( r_y ) ;
/* 801 zero-valued terms not written out */
}

/**
 * @brief Compute \f$ \frac{\partial^7}{ \partial_x^7 }\phi(x, y, z} \f$.
 *
 * Note that r_inv = 1./sqrt(r_x^2 + r_y^2 + r_z^2)
 */
__attribute__((always_inline)) INLINE static double D_700(double r_x, double r_y, double r_z, double r_inv) {
return - 135135. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv  * ( r_x * r_x * r_x * r_x * r_x * r_x * r_x ) + 10395. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 21.0   * ( r_x * r_x * r_x * r_x * r_x ) - 945. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 105.0   * ( r_x * r_x * r_x ) + 105. * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * r_inv * 105.0   * ( r_x ) ;
/* 645 zero-valued terms not written out */
}

#endif /* SWIFT_GRAVITY_DERIVATIVE_H */
