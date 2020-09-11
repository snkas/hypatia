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

#include "vector-extensions.h"

#include <cmath>

#include "ns3/vector.h"

namespace ns3 {

Vector3D
operator+ (const Vector3D &v1, const Vector3D &v2)
{
  return Vector3D (v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

Vector3D
operator- (const Vector3D &v1, const Vector3D &v2)
{
  return Vector3D (v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

Vector3D
operator* (const Vector3D &vector, double scalar)
{
  return Vector3D (vector.x * scalar, vector.y * scalar, vector.z * scalar);
}

Vector3D
operator* (double scalar, const Vector3D &vector)
{
  return vector * scalar;
}

Vector3D
CrossProduct (const Vector3D &v1, const Vector3D &v2)
{
  return Vector3D (
    v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x
  );
}

double
DotProduct (const Vector3D &v1, const Vector3D &v2)
{
  return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
}

double Magnitude (const Vector3D &vector)
{
  return std::sqrt (DotProduct (vector, vector));
}

double MagnitudeSquared (const Vector3D &vector) {
  return DotProduct (vector, vector);
}

}
