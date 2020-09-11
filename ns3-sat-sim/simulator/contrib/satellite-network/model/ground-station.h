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


#ifndef GROUND_STATION_H
#define GROUND_STATION_H

#include "ns3/core-module.h"
#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/ipv4-interface-container.h"

namespace ns3 {

class GroundSatelliteLink;

class GroundStation : public Object
{
public:

    // Constructors
    static TypeId GetTypeId();
    GroundStation(
            uint32_t gid, std::string name,
            double latitude, double longitude, double elevation,
            Vector cartesian_position
    );
    ~GroundStation();

    // Accessors
    uint32_t GetGid();
    std::string GetName();
    double GetLatitude();
    double GetLongitude();
    double GetElevation();
    Vector GetCartesianPosition();
    std::string ToString();

private:
    uint32_t m_gid;        // Unique ground stations identifier
    std::string m_name;    // Name
    double m_latitude;     // Latitude
    double m_longitude;    // Longitude
    double m_elevation;    // Elevation
    Vector m_cartesian_position;  // Cartesian coordinate according to WGS72

};

} // namespace ns3

#endif // GROUND_STATION_H
