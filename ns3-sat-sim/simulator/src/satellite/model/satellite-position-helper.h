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
 * Author: Pedro Silva <pmms@inesctec.pt>
 *
 */


#ifndef SATELLITE_POSITION_HELPER_MODEL_H
#define SATELLITE_POSITION_HELPER_MODEL_H

#include "ns3/ptr.h"
#include "ns3/vector.h"
#include "ns3/attribute.h"
#include "ns3/attribute-helper.h"

#include "ns3/satellite.h"

namespace ns3 {

/**
 * \ingroup mobility
 *
 * @brief Utility class used to interface between SatellitePositionMobilityModel
 *        and Satellite classes.
 */
class SatellitePositionHelper {
public:
  /**
   * @brief Default constructor.
   */
  SatellitePositionHelper (void);

  /**
   * @brief Create object and set satellite.
   * @param sat satellite object with SGP4/SDP4 propagation models built-in.
   */
  SatellitePositionHelper (Ptr<Satellite> sat);

  /**
   * @brief Create object, and set satellite and simulation's start time.
   * @param sat satellite object with SGP4/SDP4 propagation models built-in.
   * @param t the time instant to be considered as simulation start.
   */
  SatellitePositionHelper (Ptr<Satellite> sat, const JulianDate &t);

  /**
   * @brief Get current orbital position vector (x, y, z).
   * @return orbital position vector.
   */
  Vector3D GetPosition (void) const;

  /**
   * @brief Get orbital velocity.
   * @return orbital velocity vector.
   */
  Vector3D GetVelocity (void) const;

  /**
   * @brief Get satellite's name.
   * @return satellite's name or an empty string if the satellite object is not
   *         yet set.
   */
  std::string GetSatelliteName (void) const;

  /**
   * @brief Get the underlying satellite object.
   * @return the underlying satellite object.
   */
  Ptr<Satellite> GetSatellite (void) const;

  /**
   * @brief Get the time set to be considered as simulation's start time.
   * @return the absolute time set to be considered as simulation's start time.
   */
  JulianDate GetStartTime (void) const;

  /**
   * @brief Set the underlying satellite object.
   * @param satellite object with SGP4/SDP4 propagation models built-in.
   */
  void SetSatellite (Ptr<Satellite> sat);

  /**
   * @brief Set simulation's absolute start time.
   * @param t time instant to be consider as simulation's start.
   */
  void SetStartTime(const JulianDate &t);

private:
  Ptr<Satellite> m_sat;               //!< pointer to the Satellite object.
  JulianDate m_start;                         //!< simulation's absolute start time.
};

/**
 * \brief Stream insertion operator.
 *
 * \param os the stream
 * \param size the satellite position helper
 * \returns a reference to the stream
 */
std::ostream &operator << (std::ostream &os, const SatellitePositionHelper &satellitePositionHelper);

/**
 * \brief Stream extraction operator.
 *
 * \param is the stream
 * \param size the satellite position helper 
 * \returns a reference to the stream
 */
std::istream &operator >> (std::istream &is, SatellitePositionHelper &satellitePositionHelper);

ATTRIBUTE_HELPER_HEADER (SatellitePositionHelper);

} // namespace ns3

#endif
