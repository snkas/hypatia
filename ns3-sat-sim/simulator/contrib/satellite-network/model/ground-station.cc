/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2020 ETH Zurich
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
 * Author: Jens Eirik Saethre  June 2019
 *         Andre Aguas         March 2020
 *         Simon               2020
 */


#include "ground-station.h"
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/mpi-interface.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE("GroundStation");

NS_OBJECT_ENSURE_REGISTERED(GroundStation);

TypeId
GroundStation::GetTypeId (void)
{
    static TypeId tid = TypeId("ns3::GroundStation").SetParent<Object>().SetGroupName("SatelliteNetwork");
    return tid;
}

GroundStation::GroundStation(
        uint32_t gid, std::string name,
        double latitude, double longitude, double elevation,
        Vector cartesian_position
)
    : m_gid(gid), m_name(name),
      m_latitude(latitude), m_longitude(longitude), m_elevation(elevation),
      m_cartesian_position(cartesian_position)
{
    NS_LOG_FUNCTION(this);
}

GroundStation::~GroundStation()
{
    NS_LOG_FUNCTION(this);
}

uint32_t
GroundStation::GetGid()
{
    return m_gid;
}

std::string
GroundStation::GetName()
{
    return m_name;
}

double
GroundStation::GetLatitude()
{
    return m_latitude;
}

double
GroundStation::GetLongitude()
{
    return m_longitude;
}

double
GroundStation::GetElevation()
{
    return m_elevation;
}

Vector
GroundStation::GetCartesianPosition()
{
    return m_cartesian_position;
}

std::string
GroundStation::ToString()
{
    std::stringstream info;
    info << "Ground station[";
    info << "gid=" << m_gid << ", ";
    info << "name=" << m_name << ", ";
    info << "latitude=" << m_latitude << ", ";
    info << "longitude=" << m_longitude << ", ";
    info << "elevation=" << m_elevation << ", ";
    info << "cartesian_position=" << m_cartesian_position;
    info << "]";
    return info.str();
}

} // namespace ns3

