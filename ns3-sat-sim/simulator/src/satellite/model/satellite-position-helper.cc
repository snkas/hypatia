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


#include "satellite-position-helper.h"

#include "ns3/simulator.h"
#include "ns3/nstime.h"

namespace ns3 {

ATTRIBUTE_HELPER_CPP (SatellitePositionHelper);

SatellitePositionHelper::SatellitePositionHelper (void)
{
  SetStartTime (JulianDate ());
}

SatellitePositionHelper::SatellitePositionHelper (Ptr<Satellite> sat)
{
  SetSatellite (sat);
  SetStartTime (sat->GetTleEpoch ());
}

SatellitePositionHelper::SatellitePositionHelper(
  Ptr<Satellite> sat, const JulianDate &t
)
{
  SetSatellite (sat);
  SetStartTime (t);
}

Vector3D
SatellitePositionHelper::GetPosition (void) const
{
  if (!m_sat)
    return Vector3D (0,0,0);

  JulianDate cur = m_start + Simulator::Now ();

  return m_sat->GetPosition (cur);
}

Vector3D
SatellitePositionHelper::GetVelocity (void) const
{
  if (!m_sat)
    return Vector3D (0,0,0);

  JulianDate cur = m_start + Simulator::Now ();

  return m_sat->GetVelocity (cur);
}

Ptr<Satellite>
SatellitePositionHelper::GetSatellite (void) const
{
  return m_sat;
}

JulianDate
SatellitePositionHelper::GetStartTime (void) const
{
  return m_start;
}

std::string
SatellitePositionHelper::GetSatelliteName (void) const
{
  return (!m_sat ? m_sat->GetName () : "");
}

void
SatellitePositionHelper::SetSatellite (Ptr<Satellite> sat)
{
  m_sat = sat;
}

void
SatellitePositionHelper::SetStartTime (const JulianDate &t)
{
  m_start = t;
}

std::ostream
&operator << (std::ostream &os, const SatellitePositionHelper &satellitePositionHelper)
{
  if (satellitePositionHelper.GetSatellite()) {
    os << satellitePositionHelper.GetSatellite()->GetTleInfo().first  << "|" <<
          satellitePositionHelper.GetSatellite()->GetTleInfo().second << "|" <<
          satellitePositionHelper.GetStartTime().ToString();
  }
  else {
    os << "satellite not yet initialized";
  }
  return os;
}
/* Initialize a queue size from an input stream */
std::istream &operator >> (std::istream &is, SatellitePositionHelper &satellitePositionHelper)
{
  std::string str;
  is >> str;
  std::istringstream tokenStream(str);
  std::vector<std::string> tokens;
  std::string token;
  while (std::getline(tokenStream, token, '|'))
  {
    tokens.push_back(token);
  }

  std::string tle1, tle2, time;
  time = tokens.back(); tokens.pop_back();
  tle2 = tokens.back(); tokens.pop_back();
  tle1 = tokens.back(); tokens.pop_back();

  Ptr<Satellite> satellite = CreateObject<Satellite>();
  satellite->SetTleInfo(tle1, tle2);
  satellitePositionHelper.SetSatellite(satellite);

  JulianDate startTime = JulianDate();
  startTime.SetDate(time, DateTime::TimeSystem::UTC);
  satellitePositionHelper.SetStartTime(startTime);

  return is;
}

} // namespace ns3
