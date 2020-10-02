/*
 * Copyright (c) 2019 ETH Zurich
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

#ifndef TOPOLOGY_SATELLITE_NETWORK_H
#define TOPOLOGY_SATELLITE_NETWORK_H

#include <utility>
#include "ns3/core-module.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/topology.h"
#include "ns3/exp-util.h"
#include "ns3/basic-simulation.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/command-line.h"
#include "ns3/traffic-control-helper.h"
#include "ns3/ground-station.h"
#include "ns3/satellite-position-helper.h"
#include "ns3/satellite-position-mobility-model.h"
#include "ns3/mobility-helper.h"
#include "ns3/string.h"
#include "ns3/type-id.h"
#include "ns3/vector.h"
#include "ns3/satellite-position-helper.h"
#include "ns3/point-to-point-laser-helper.h"
#include "ns3/gsl-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-static-routing.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/wifi-net-device.h"
#include "ns3/point-to-point-laser-net-device.h"
#include "ns3/ipv4.h"

namespace ns3 {

    class TopologySatelliteNetwork : public Topology
    {
    public:

        // Constructors
        static TypeId GetTypeId (void);
        TopologySatelliteNetwork(Ptr<BasicSimulation> basicSimulation, const Ipv4RoutingHelper& ipv4RoutingHelper);

        // Inherited accessors
        const NodeContainer& GetNodes();
        int64_t GetNumNodes();
        bool IsValidEndpoint(int64_t node_id);
        const std::set<int64_t>& GetEndpoints();

        // Additional accessors
        uint32_t GetNumSatellites();
        uint32_t GetNumGroundStations();
        const NodeContainer& GetSatelliteNodes();
        const NodeContainer& GetGroundStationNodes();
        const std::vector<Ptr<GroundStation>>& GetGroundStations();
        const std::vector<Ptr<Satellite>>& GetSatellites();
        const Ptr<Satellite> GetSatellite(uint32_t sat_id);
        uint32_t NodeToGroundStationId(uint32_t node_id);
        bool IsSatelliteId(uint32_t node_id);
        bool IsGroundStationId(uint32_t node_id);

        // Post-processing
        void CollectUtilizationStatistics();

    private:

        // Build functions
        void ReadConfig();
        void Build(const Ipv4RoutingHelper& ipv4RoutingHelper);
        void ReadGroundStations();
        void ReadSatellites();
        void InstallInternetStacks(const Ipv4RoutingHelper& ipv4RoutingHelper);
        void ReadISLs();
        void CreateGSLs();

        // Helper
        void EnsureValidNodeId(uint32_t node_id);

        // Routing
        Ipv4AddressHelper m_ipv4_helper;
        void PopulateArpCaches();

        // Input
        Ptr<BasicSimulation> m_basicSimulation;       //<! Basic simulation instance
        std::string m_satellite_network_dir;          //<! Directory containing satellite network information
        std::string m_satellite_network_routes_dir;   //<! Directory containing the routes over time of the network
        bool m_satellite_network_force_static;        //<! True to disable satellite movement and basically run
                                                      //   it static at t=0 (like a static network)

        // Generated state
        NodeContainer m_allNodes;                           //!< All nodes
        NodeContainer m_groundStationNodes;                 //!< Ground station nodes
        NodeContainer m_satelliteNodes;                     //!< Satellite nodes
        std::vector<Ptr<GroundStation> > m_groundStations;  //!< Ground stations
        std::vector<Ptr<Satellite>> m_satellites;           //<! Satellites
        std::set<int64_t> m_endpoints;                      //<! Endpoint ids = ground station ids

        // ISL devices
        NetDeviceContainer m_islNetDevices;
        std::vector<std::pair<int32_t, int32_t>> m_islFromTo;

        // Values
        double m_isl_data_rate_megabit_per_s;
        double m_gsl_data_rate_megabit_per_s;
        int64_t m_isl_max_queue_size_pkts;
        int64_t m_gsl_max_queue_size_pkts;
        bool m_enable_isl_utilization_tracking;
        int64_t m_isl_utilization_tracking_interval_ns;

    };

}

#endif //TOPOLOGY_SATELLITE_NETWORK_H
