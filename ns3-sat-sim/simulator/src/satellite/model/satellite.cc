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

#include "satellite.h"

#include <algorithm>
#include <cstring>
#include <stdint.h>
#include <string>
#include <utility>

#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/type-id.h"
#include "ns3/vector.h"

#include "vector-extensions.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Satellite");

NS_OBJECT_ENSURE_REGISTERED (Satellite);

const gravconsttype Satellite::WGeoSys = wgs72;   // recommended for SGP4/SDP4
const uint32_t Satellite::TleSatNameWidth = 24;
const uint32_t Satellite::TleSatInfoWidth = 69;

TypeId
Satellite::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Satellite")
    .SetParent<Object> ()
    .SetGroupName ("Satellite")
    .AddConstructor<Satellite> ();

  return tid;
}

Satellite::Satellite (void) :
  m_name (""), m_tle1 (""), m_tle2 ("")
{
  NS_LOG_FUNCTION_NOARGS ();

  m_sgp4_record.jdsatepoch = 0;
}

uint32_t
Satellite::GetSatelliteNumber (void) const
{
  return (IsInitialized () ? static_cast<uint32_t> (m_sgp4_record.satnum) : 0);
}

std::string
Satellite::GetName (void) const
{
  return m_name;
}

std::pair<std::string, std::string>
Satellite::GetTleInfo (void) const
{
  return std::make_pair (m_tle1, m_tle2);
}

JulianDate
Satellite::GetTleEpoch (void) const {
  if (IsInitialized ())
    return JulianDate (m_sgp4_record.jdsatepoch);

  return JulianDate ();
}

Vector3D
Satellite::GetPosition (const JulianDate &t) const
{
  double r[3], v[3];
  //double delta = (t - GetTleEpoch ())*1440;
  double delta = (t - GetTleEpoch ()).GetMinutes();

  if (!IsInitialized ())
    return Vector3D ();

  sgp4 (WGeoSys, m_sgp4_record, delta, r, v);

  if (m_sgp4_record.error != 0)
    return Vector3D ();

  // vector r is in km so it needs to be converted to meters
  return rTemeTorItrf (Vector3D (r[0], r[1], r[2]), t)*1000;
}

Vector3D
Satellite::GetVelocity (const JulianDate &t) const
{
  double r[3], v[3];
  //double delta = (t - GetTleEpoch ())*1440;
  double delta = (t - GetTleEpoch ()).GetMinutes();

  if (!IsInitialized ())
    return Vector3D ();

  sgp4 (WGeoSys, m_sgp4_record, delta, r, v);

  if (m_sgp4_record.error != 0)
    return Vector3D ();

  // velocity vector is in km/s so it needs to be converted to m/s
  return 1000*rvTemeTovItrf (
    Vector3D (r[0], r[1], r[2]), Vector3D (v[0], v[1], v[2]), t
  );
}

/*
 * This function uses the WGS84 constants as defined by the National
 * Geospatial-Intelligence Agency (NGA) on the report published on 2014-07-08
 * entitled WORLD GEODETIC SYSTEM 1984 - Its Definition and Relationships with
 * Local Geodetic Systems. The report is available at
 * http://earth-info.nga.mil/GandG/publications/NGA_STND_0036_1_0_0_WGS84/
 * NGA.STND.0036_1.0.0_WGS84.pdf [the URL was split to fit page limits].
 *
 * The datum conversion is described at
 * http://www.nalresearch.com/files/Standard%20Modems/
 * A3LA-XG/A3LA-XG%20SW%20Version%201.0.0/GPS%20Technical%20Documents/
 * GPS.G1-X-00006%20%28Datum%20Transformations%29.pdf
 * [the URL was split to fit page limits]
 */
Vector3D
Satellite::GetGeographicPosition (const JulianDate &t) const
{
  const double a = 6378137.0;                   // equatorial radius (m)
  const double b = 6356752.31424518;            // polar radius (m)
  const double fes = 6.694379990141e-03;        // first eccentricity squared
  const double ses = 6.739496742276e-03;        // second eccentricity squared
  Vector3D r = GetPosition (t);
  const double p = sqrt (r.x*r.x + r.y*r.y);
  const double th = atan2 (a*r.z, b*p);
  const double sinth = sin (th);
  const double costh = cos (th);
  double N, sinLat;
  Vector3D g;

  g.y = atan2 (r.y, r.x);
  g.x = atan2 (r.z + ses*b*sinth*sinth*sinth, p - fes*a*costh*costh*costh);

  sinLat = sin (g.x);
  N = a / sqrt (1 - fes*sinLat*sinLat);

  g.z = p / cos (g.x) - N;
  g.x = g.x * 180.0/M_PI;
  g.y = g.y * 180.0/M_PI;

  return g;
}

Time
Satellite::GetOrbitalPeriod (void) const
{
  if (!IsInitialized ())
    return MilliSeconds (0);

  // rad/min -> min [rad/(rad/min) = rad*min/rad = min]
  return MilliSeconds (60000*2*M_PI/m_sgp4_record.no);
}

void
Satellite::SetName (const std::string &name)
{
  NS_ASSERT_MSG (!name.empty (), "Name cannot be empty!");

  m_name = name.substr (0, name.find_last_not_of (" ") + 1);
}

bool
Satellite::SetTleInfo (const std::string &line1, const std::string &line2)
{
  double start, stop, delta;
  char l1[TleSatInfoWidth+1], l2[TleSatInfoWidth+1];
  double r[3], v[3];

  NS_ASSERT_MSG (
    line1.size () == TleSatInfoWidth && line2.size () == TleSatInfoWidth,
    "Two-Line Element info lines must be of length" << TleSatInfoWidth << "!"
  );

  m_tle1 = std::string (line1.c_str ());
  m_tle2 = std::string (line2.c_str ());

  memcpy (l1, line1.c_str (), sizeof (l1));
  memcpy (l2, line2.c_str (), sizeof (l2));

  // 'c' => catalog mode run
  // 'e' => epoch time (relative to TLE lines)
  // 'i' => improved mode of operation
  twoline2rv (
    l1, l2, 'c', 'e', 'i', WGeoSys, start, stop, delta, m_sgp4_record
  );

  // call propagator to check if it has been properly initialized
  sgp4 (WGeoSys, m_sgp4_record, 0, r, v);

  // return true if no errors occurred
  return (m_sgp4_record.error == 0);
}

std::string
Satellite::ExtractTleSatName (const std::string &name)
{
  NS_ASSERT_MSG (
    name.size () <= TleSatNameWidth,
    "Name cannot have a length greater than " << TleSatNameWidth << "!"
  );

  return name.substr (
    0, std::min (name.size (), static_cast<size_t> (TleSatNameWidth))
  );
}

std::string
Satellite::ExtractTleSatInfo (const std::string &info)
{
  NS_ASSERT_MSG (
    info.size () == TleSatInfoWidth,
    "Two-Line Element info lines must be of length" << TleSatInfoWidth << "!"
  );

  return info.substr (0, TleSatInfoWidth);
}

bool
Satellite::IsInitialized (void) const
{
  return ((m_sgp4_record.jdsatepoch > 0) && (m_tle1 != "") && (m_tle2 != ""));
}

Satellite::Matrix
Satellite::PefToItrf (const JulianDate &t)
{
  std::pair<double, double> eop = t.GetPolarMotion ();

  const double &xp = eop.first, &yp = eop.second;
  const double cosxp = cos (xp), cosyp = cos (yp);
  const double sinxp = sin (xp), sinyp = sin (yp);

  // [from AIAA-2006-6753 Report, Page 32, Appendix C - TEME Coordinate System]
  //
  // Matrix(ITRF<->PEF) = ROT1(yp)*ROT2(xp) [using c for cos, and s for sin]
  //
  // | 1    0     0   |*| c(xp) 0 -s(xp) |=|    c(xp)       0      -s(xp)   |
  // | 0  c(yp) s(yp) | |   0   1    0   | | s(yp)*s(xp)  c(yp) s(yp)*c(xp) |
  // | 0 -s(yp) c(yp) | | s(xp) 0  c(xp) | | c(yp)*s(xp) -s(yp) c(yp)*c(xp) |
  //

  // we return the transpose because it is what's needed
  return Matrix(
     cosxp, sinyp*sinxp, cosyp*sinxp,
       0,      cosyp,      -sinyp,
    -sinxp, sinyp*cosxp, cosyp*cosxp
  );
}

Satellite::Matrix
Satellite::TemeToPef (const JulianDate &t)
{
  const double gmst = t.GetGmst ();
  const double cosg = cos (gmst), sing = sin (gmst);

  // [from AIAA-2006-6753 Report, Page 32, Appendix C - TEME Coordinate System]
  //
  // rPEF = ROT3(gmst)*rTEME
  //
  // |  cos(gmst) sin(gmst) 0 |
  // | -sin(gmst) cos(gmst) 0 |
  // |      0         0     1 |
  //

  return Matrix(
     cosg, sing, 0,
    -sing, cosg, 0,
       0,    0,  1
  );
}

Vector3D
Satellite::rTemeTorItrf (const Vector3D &rteme, const JulianDate &t)
{
  Matrix pmt = PefToItrf (t);                   // PEF->ITRF matrix transposed
  Matrix tmt = TemeToPef (t);                   // TEME->PEF matrix

  return pmt*(tmt*rteme);
}

Vector3D
Satellite::rvTemeTovItrf(
  const Vector3D &rteme, const Vector3D &vteme, const JulianDate &t
)
{
  Matrix pmt = PefToItrf (t);                   // PEF->ITRF matrix transposed
  Matrix tmt = TemeToPef (t);                   // TEME->PEF matrix
  Vector3D w (0.0, 0.0, t.GetOmegaEarth ());

  return pmt*((tmt*vteme) - CrossProduct (w, tmt*rteme));
}

Satellite::Matrix::Matrix (
  double c00, double c01, double c02,
  double c10, double c11, double c12,
  double c20, double c21, double c22
)
{
  m[0][0] = c00;
  m[0][1] = c01;
  m[0][2] = c02;
  m[1][0] = c10;
  m[1][1] = c11;
  m[1][2] = c12;
  m[2][0] = c20;
  m[2][1] = c21;
  m[2][2] = c22;
}

Vector3D
Satellite::Matrix::operator* (const Vector3D& v) const
{
  return Vector3D (
    m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z,
    m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z,
    m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z
  );
}

Satellite::Matrix
Satellite::Matrix::Transpose (void) const
{
  return Matrix(
    m[0][0], m[1][0], m[2][0],
    m[0][1], m[1][1], m[2][1],
    m[0][2], m[1][2], m[2][2]
  );
}

}
