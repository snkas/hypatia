/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 INESC TEC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Pedro Silva  <pmms@inesctec.pt>
 *
 */

#ifndef SATELLITE_VECTOR_EXTENSIONS_H
#define SATELLITE_VECTOR_EXTENSIONS_H

#include "ns3/vector.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * @brief extensions to ns3::Vector3D to support additional operations
 *
 * This file defines vector addition, vector difference, the product between
 * a vector and a scalar, the cross product, the inner product, the magnitude,
 * and the square of the magnitude.
 */

/**
 * @brief Addition of Vector3D objects.
 * @param v1 The first vector.
 * @param v2 The second vector.
 * @return a vector that is the sum of v1 and v2.
 */
Vector3D operator+ (const Vector3D &v1, const Vector3D &v2);

/**
 * @brief Subtraction of Vector3D objects.
 * @param v1 The first vector.
 * @param v2 The second vector.
 * @return a vector that is the difference between v1 and v2.
 */
Vector3D operator- (const Vector3D &v1, const Vector3D &v2);

/**
 * @brief Multiplication between a Vector3D object and a scalar.
 * @param vector The vector.
 * @param scalar The scalar.
 * @return a vector that is the product of the vector by the scalar.
 */
Vector3D operator* (const Vector3D &vector, double scalar);

/**
 * @brief Multiplication between a scalar and a Vector3D object.
 * @param scalar The scalar.
 * @param vector The vector.
 * @return a vector that is the product of the vector by the scalar.
 */
Vector3D operator* (double scalar, const Vector3D &vector);

/**
 * @brief Cross product of two Vector3D objects.
 * @param v1 The first vector.
 * @param v2 The second vector.
 * @returns a vector that is the cross product of the two vectors.
 */
Vector3D CrossProduct (const Vector3D &v1, const Vector3D &v2);

/**
 * @brief Dot product of two Vector3D objects.
 * @param v1 The first vector.
 * @param v2 The second vector.
 * @returns a scalar that is the dot product of the two vectors.
 */
double DotProduct (const Vector3D &v1, const Vector3D &v2);

/**
 * @brief Magnitude of a Vector3D object.
 * @param vector The vector.
 * @returns a scalar that is the magnitude of the vector.
 */
double Magnitude (const Vector3D &vector);

/**
 * @brief The square of the magnitude of a Vector3D object.
 * @param vector The vector.
 * @returns a scalar that is the square of the magnitude of the vector.
 */
double MagnitudeSquared (const Vector3D &vector);

}

#endif /* SATELLITE_VECTOR_EXTENSIONS_H */
