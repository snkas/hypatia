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
 *         Andre Aguas  March 2020
 *
 */

#include "satellite-position-mobility-model.h"

#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include "ns3/ptr.h"
#include "ns3/satellite.h"
#include "ns3/type-id.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SatellitePositionMobilityModel");

NS_OBJECT_ENSURE_REGISTERED (SatellitePositionMobilityModel);

TypeId
SatellitePositionMobilityModel::GetTypeId (void) {
  static TypeId tid = TypeId ("ns3::SatellitePositionMobilityModel")
    .SetParent<MobilityModel> ()
    .SetGroupName ("Mobility")
    .AddConstructor<SatellitePositionMobilityModel> ()
    .AddAttribute("SatellitePositionHelper",
                  "The satellite position helper that holds the satellite reference of this node",
                  SatellitePositionHelperValue(SatellitePositionHelper()),
                  MakeSatellitePositionHelperAccessor (&SatellitePositionMobilityModel::m_helper),
                  MakeSatellitePositionHelperChecker())
  ;

  return tid;
}

SatellitePositionMobilityModel::SatellitePositionMobilityModel (void) { }
SatellitePositionMobilityModel::~SatellitePositionMobilityModel (void) { }

std::string
SatellitePositionMobilityModel::GetSatelliteName (void) const
{
  return m_helper.GetSatelliteName ();
}

Ptr<Satellite>
SatellitePositionMobilityModel::GetSatellite (void) const
{
  return m_helper.GetSatellite ();
}

JulianDate
SatellitePositionMobilityModel::GetStartTime (void) const
{
  return m_helper.GetStartTime ();
}

void
SatellitePositionMobilityModel::SetSatellite (Ptr<Satellite> sat)
{
  m_helper.SetSatellite (sat);
}

void
SatellitePositionMobilityModel::SetStartTime (const JulianDate &t)
{
  m_helper.SetStartTime (t);
}

Vector3D
SatellitePositionMobilityModel::DoGetPosition (void) const
{
  return m_helper.GetPosition ();
}

void
SatellitePositionMobilityModel::DoSetPosition (const Vector3D &position)
{
  // position is not settable
  // NotifyCourseChange ();
}

Vector3D
SatellitePositionMobilityModel::DoGetVelocity (void) const
{
  return m_helper.GetVelocity ();
}

}
