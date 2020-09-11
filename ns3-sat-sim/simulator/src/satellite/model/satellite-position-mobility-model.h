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

#ifndef SATELLITE_POSITION_MOBILITY_MODEL_H
#define SATELLITE_POSITION_MOBILITY_MODEL_H

#include <string>

#include "ns3/julian-date.h"
#include "ns3/mobility-model.h"
#include "ns3/ptr.h"
#include "ns3/satellite.h"
#include "ns3/type-id.h"

#include "satellite-position-helper.h"

namespace ns3 {

/**
 * \ingroup mobility
 * @brief Satellite mobility model based on SGP4/SDP4 models.
 *
 * This mobility model relies on an underlying Satellite object to provide the
 * position and velocity vectors. Given that the Satellite object expects time
 * parameters to be absolute, it is also required to set the time instant to be
 * considered as simulation start so that relative simulation time can be mapped
 * into an absolute time.
 *
 * The DoSetPosition function has no effect because a satellite orbit cannot be
 * specified solely by a 3D position. When setting up Satellite objects, bear in
 * mind that it provides maximum accuracy at TLE epoch.
 */
class SatellitePositionMobilityModel : public MobilityModel {
public:
  /**
   * @brief Get the type ID.
   * @return the object TypeId.
   */
  static TypeId GetTypeId (void);

  /**
   * @brief Default constructor.
   */
  SatellitePositionMobilityModel (void);

  /**
   * @brief Destructor.
   */
  virtual ~SatellitePositionMobilityModel (void);

  /**
   * @brief Retrieve satellite's name.
   * @return the satellite's name or an empty string if has not yet been set.
   */
  std::string GetSatelliteName (void) const;

  /**
   * @brief Get the underlying Satellite object.
   * @return a pointer to the underlying Satellite object.
   */
  Ptr<Satellite> GetSatellite (void) const;

  /**
   * @brief Get the time instant considered as the simulation start.
   * @return a JulianDate object with the time considered as simulation start.
   */
  JulianDate GetStartTime (void) const;

  /**
   * @brief Set the underlying Satellite object.
   * @param sat a pointer to the Satellite object to be used.
   */
  void SetSatellite (Ptr<Satellite> sat);

  /**
   * @brief Set the time instant considered as the simulation start.
   * @param t the time instant to be considered as simulation start.
   */
  void SetStartTime (const JulianDate &t);

private:
  virtual Vector DoGetPosition (void) const;
  virtual void DoSetPosition (const Vector &position);
  virtual Vector DoGetVelocity (void) const;

  SatellitePositionHelper m_helper;     //!< helper for orbital computations
};

} // namespace ns3

#endif /* SATELLITE_POSITION_MOBILITY_MODEL_H */
