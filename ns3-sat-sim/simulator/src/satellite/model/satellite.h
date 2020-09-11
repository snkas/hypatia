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

#ifndef SATELLITE_H
#define SATELLITE_H

#include <stdint.h>
#include <string>
#include <utility>

#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/type-id.h"
#include "ns3/vector.h"

#include "julian-date.h"
#include "sgp4ext.h"
#include "sgp4io.h"
#include "sgp4unit.h"

namespace ns3 {

/**
 * \ingroup satellite
 * @brief Satellite object using the simplified perturbations models.
 *
 * This class relies on the code released by David Vallado et al. that
 * implements the simplified perturbations models, usually known collectively as
 * SGP4, to calculate the orbital position and velocity of the satellite. The
 * authors provide a detailed report, the source code (in several programming
 * languages), and additional information regarding the models and their
 * implementation at https://celestrak.com/publications/AIAA/2006-6753/. The
 * source code remains as is (spg4* files) except for minor changes that were
 * carried out to ensure that recent C++ compilers issued no warnings, e.g.,
 * due to uninitialized variables and implicit type conversions.
 *
 * The SGP4/SDP4 models expect the input to be in NORAD's Two-Line Element (TLE)
 * format, a data format used to encode a list of orbital elements of an
 * Earth-orbiting object at a given point in time (the epoch). A description of
 * this format can be found at https://www.space-track.org/documentation#/tle or
 * at https://en.wikipedia.org/wiki/Two-line_element_set. Using these orbital
 * elements the SGP4/SDP4 models are able to predict with some accuracy the
 * satellite's position and velocity in past or future times. The accuracy is
 * ~1 km at TLE epoch and grows at a rate of 1 to 3 km per day (see Vallado's et
 * al. report). Joint Space Operations Center (JSpOC) tracks all detectable
 * objects orbiting the Earth and provides that information in TLE format at
 * https://www.space-track.org upon registration. For maximum accuracy, download
 * the TLE data which has an epoch closest to the one intended.
 *
 * The output of SGP4/SDP4 models is in the TEME (true equator, mean equinox)
 * coordinate system, an Earth-centered inertial (ECI) frame. Therefore, these
 * coordinates need to be converted into an Earth-centered Earth-fixed (ECEF)
 * frame to be usable within ns-3: International Terrestrial Reference Frame
 * (ITRF). The conversion itself requires Earth Orientation Parameters (EOP)
 * that are provided by the JulianDate class.
 */
class Satellite : public Object {
public:
  /// World Geodetic System (WGS) constants to be used by SGP4/SDP4 models.
  static const gravconsttype WGeoSys;
  /// Satellite's name field size defined by TLE data format.
  static const uint32_t TleSatNameWidth;
  /// Satellite's information line size defined by TLE data format.
  static const uint32_t TleSatInfoWidth;

  /**
   * @brief Get the type ID.
   * @return the object TypeId.
   */
  static TypeId GetTypeId (void);

  /**
   * @brief Default constructor.
   */
  Satellite (void);

  /**
   * @brief Retrieve the satellite's number (NORAD SAT_ID).
   * @return the satellite's number (NORAD ID) if it has already been
   *         initialized or 0 otherwise.
   */
  uint32_t GetSatelliteNumber (void) const;

  /**
   * @brief Retrieve satellite's name.
   * @return the satellite's name or an empty string if has not yet been set.
   */
  std::string GetName (void) const;

  /**
   * @brief Retrieve the TLE information used to initialize this satellite.
   * @return an std::pair with the two lines used to initialize this satellite,
   *         or an std::pair with two empty strings if it has not yet been set.
   */
  std::pair<std::string, std::string> GetTleInfo (void) const;

  /**
   * @brief Retrieve the TLE epoch time.
   * @return the TLE epoch time or 0h, 1 January 1992 if the satellite has not
   *         yet been initialized.
   */
  JulianDate GetTleEpoch (void) const;

  /**
   * @brief Get the prediction for the satellite's position at a given time.
   * @param t When.
   * @return an ns3::Vector3D object containing the satellite's position,
   *         in meters, on ITRF coordinate frame.
   */
  Vector3D GetPosition (const JulianDate &t) const;

  /**
   * @brief Get the prediction for the satellite's velocity at a given time.
   * @param t When.
   * @return an ns3::Vector3D object containing the satellite's velocity,
   *         in m/s, on ITRF coordinate frame.
   */
  Vector3D GetVelocity (const JulianDate &t) const;

  /**
   * @brief Get the predicted satellite's geographic position at a given time.
   * @param t When.
   * @return an ns3::Vector3D object containing the satellite's position:
   *         x = latitude (in degrees), y = longitude (in degrees), and
   *         z = altitude (in meters) on WGS84 format.
   */
  Vector3D GetGeographicPosition (const JulianDate &t) const;

  /**
   * @brief Get the satellite's orbital period.
   * @return an ns3::Time object containing the satellite's orbital period.
   */
  Time GetOrbitalPeriod (void) const;

  /**
   * @brief Set satellite's name.
   * @param name Satellite's name.
   */
  void SetName (const std::string &name);

  /**
   * @brief Set satellite's TLE information required for its initialization.
   * @param line1 First line of the TLE data format.
   * @param line2 Second line of the TLE data format.
   * @return a boolean indicating whether the initialization succeeded.
   */
  bool SetTleInfo (const std::string &line1, const std::string &line2);

  /**
   * @brief Extract the satellite's name from a string.
   * @param name String containing the satellite's name.
   * @return the satellite's name containing, at most, the maximum number of
   *         characters allowed by the TLE data format.
   */
  static std::string ExtractTleSatName (const std::string &name);

  /**
   * @brief Extract a TLE line from a string.
   * @param info String containing the TLE line.
   * @return the TLE line containing exactly the number of characters required
   *         by the TLE data format.
   */
  static std::string ExtractTleSatInfo (const std::string &info);

private:
  /// row of a Matrix
  struct Row {
    double r[3];

    double& operator[] (uint32_t i) { return r[i]; }
    const double& operator[] (uint32_t i) const { return r[i]; }
  };

  /// Matrix data structure to make coordinate conversion code clearer and
  /// less verbose
  struct Matrix {
  public:
    Matrix (void) { }
    Matrix (
      double c00, double c01, double c02,
      double c10, double c11, double c12,
      double c20, double c21, double c22
    );

    Row& operator[] (uint32_t i) { return m[i]; }
    const Row& operator[] (uint32_t i) const { return m[i]; }

    Vector3D operator* (const Vector3D &v) const;

    Matrix Transpose (void) const;

  private:
    Row m[3];
  };

  /**
   * @brief Check if the satellite has already been initialized.
   * @return a boolean indicating whether the satellite is initialized.
   */
  bool IsInitialized (void) const;

  /**
   * @brief Retrieve the matrix for converting from PEF to ITRF at a given time.
   * @param t When.
   * @return the PEF-ITRF conversion matrix.
   */
  static Matrix PefToItrf (const JulianDate &t);

  /**
   * @brief Retrieve the matrix for converting from TEME to PEF at a given time.
   * @param t When.
   * @return the TEME-PEF conversion matrix.
   */
  static Matrix TemeToPef (const JulianDate &t);

  /**
   * @brief Retrieve the satellite's position vector in ITRF coordinates.
   * @param t When.
   * @return the satellite's position vector in ITRF coordinates (meters).
   */
  static Vector3D rTemeTorItrf (const Vector3D &rteme, const JulianDate &t);

  /**
   * @brief Retrieve the satellite's velocity vector in ITRF coordinates.
   * @param t When.
   * @return the satellite's velocity vector in ITRF coordinates (m/s).
   */
  static Vector3D rvTemeTovItrf (
    const Vector3D &rteme, const Vector3D &vteme, const JulianDate& t
  );

  std::string m_name;                               //!< satellite's name.
  std::string m_tle1, m_tle2;                       //!< satellite's TLE data.
  mutable elsetrec m_sgp4_record;                   //!< SGP4/SDP4 record.
};

}

#endif /* SATELLITE_H */
