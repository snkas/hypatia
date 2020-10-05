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

#include "topology-satellite-network.h"

namespace ns3 {

    NS_OBJECT_ENSURE_REGISTERED (TopologySatelliteNetwork);
    TypeId TopologySatelliteNetwork::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::TopologySatelliteNetwork")
                .SetParent<Object> ()
                .SetGroupName("SatelliteNetwork")
        ;
        return tid;
    }

    TopologySatelliteNetwork::TopologySatelliteNetwork(Ptr<BasicSimulation> basicSimulation, const Ipv4RoutingHelper& ipv4RoutingHelper) {
        m_basicSimulation = basicSimulation;
        ReadConfig();
        Build(ipv4RoutingHelper);
    }

    void TopologySatelliteNetwork::ReadConfig() {
        m_satellite_network_dir = m_basicSimulation->GetRunDir() + "/" + m_basicSimulation->GetConfigParamOrFail("satellite_network_dir");
        m_satellite_network_routes_dir =  m_basicSimulation->GetRunDir() + "/" + m_basicSimulation->GetConfigParamOrFail("satellite_network_routes_dir");
        m_satellite_network_force_static = parse_boolean(m_basicSimulation->GetConfigParamOrDefault("satellite_network_force_static", "false"));
    }

    void
    TopologySatelliteNetwork::Build(const Ipv4RoutingHelper& ipv4RoutingHelper) {
        std::cout << "SATELLITE NETWORK" << std::endl;

        // Initialize satellites
        ReadSatellites();
        std::cout << "  > Number of satellites........ " << m_satelliteNodes.GetN() << std::endl;

        // Initialize ground stations
        ReadGroundStations();
        std::cout << "  > Number of ground stations... " << m_groundStationNodes.GetN() << std::endl;

        // Only ground stations are valid endpoints
        for (uint32_t i = 0; i < m_groundStations.size(); i++) {
            m_endpoints.insert(m_satelliteNodes.GetN() + i);
        }

        // All nodes
        m_allNodes.Add(m_satelliteNodes);
        m_allNodes.Add(m_groundStationNodes);
        std::cout << "  > Number of nodes............. " << m_allNodes.GetN() << std::endl;

        // Install internet stacks on all nodes
        InstallInternetStacks(ipv4RoutingHelper);
        std::cout << "  > Installed Internet stacks" << std::endl;

        // IP helper
        m_ipv4_helper.SetBase ("10.0.0.0", "255.255.255.0");

        // Link settings
        m_isl_data_rate_megabit_per_s = parse_positive_double(m_basicSimulation->GetConfigParamOrFail("isl_data_rate_megabit_per_s"));
        m_gsl_data_rate_megabit_per_s = parse_positive_double(m_basicSimulation->GetConfigParamOrFail("gsl_data_rate_megabit_per_s"));
        m_isl_max_queue_size_pkts = parse_positive_int64(m_basicSimulation->GetConfigParamOrFail("isl_max_queue_size_pkts"));
        m_gsl_max_queue_size_pkts = parse_positive_int64(m_basicSimulation->GetConfigParamOrFail("gsl_max_queue_size_pkts"));

        // Utilization tracking settings
        m_enable_isl_utilization_tracking = parse_boolean(m_basicSimulation->GetConfigParamOrFail("enable_isl_utilization_tracking"));
        if (m_enable_isl_utilization_tracking) {
            m_isl_utilization_tracking_interval_ns = parse_positive_int64(m_basicSimulation->GetConfigParamOrFail("isl_utilization_tracking_interval_ns"));
        }

        // Create ISLs
        std::cout << "  > Reading and creating ISLs" << std::endl;
        ReadISLs();

        // Create GSLs
        std::cout << "  > Creating GSLs" << std::endl;
        CreateGSLs();

        // ARP caches
        std::cout << "  > Populating ARP caches" << std::endl;
        PopulateArpCaches();

        std::cout << std::endl;

    }

    void
    TopologySatelliteNetwork::ReadSatellites()
    {

        // Open file
        std::ifstream fs;
        fs.open(m_satellite_network_dir + "/tles.txt");
        NS_ABORT_MSG_UNLESS(fs.is_open(), "File tles.txt could not be opened");

        // First line:
        // <orbits> <satellites per orbit>
        std::string orbits_and_n_sats_per_orbit;
        std::getline(fs, orbits_and_n_sats_per_orbit);
        std::vector<std::string> res = split_string(orbits_and_n_sats_per_orbit, " ", 2);
        int64_t num_orbits = parse_positive_int64(res[0]);
        int64_t satellites_per_orbit = parse_positive_int64(res[1]);

        // Create the nodes
        m_satelliteNodes.Create(num_orbits * satellites_per_orbit);

        // Associate satellite mobility model with each node
        int64_t counter = 0;
        std::string name, tle1, tle2;
        while (std::getline(fs, name)) {
            std::getline(fs, tle1);
            std::getline(fs, tle2);

            // Format:
            // <name>
            // <TLE line 1>
            // <TLE line 2>

            // Create satellite
            Ptr<Satellite> satellite = CreateObject<Satellite>();
            satellite->SetName(name);
            satellite->SetTleInfo(tle1, tle2);

            // Decide the mobility model of the satellite
            MobilityHelper mobility;
            if (m_satellite_network_force_static) {

                // Static at the start of the epoch
                mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
                mobility.Install(m_satelliteNodes.Get(counter));
                Ptr<MobilityModel> mobModel = m_satelliteNodes.Get(counter)->GetObject<MobilityModel>();
                mobModel->SetPosition(satellite->GetPosition(satellite->GetTleEpoch()));

            } else {

                // Dynamic
                mobility.SetMobilityModel(
                        "ns3::SatellitePositionMobilityModel",
                        "SatellitePositionHelper",
                        SatellitePositionHelperValue(SatellitePositionHelper(satellite))
                );
                mobility.Install(m_satelliteNodes.Get(counter));

            }

            // Add to all satellites present
            m_satellites.push_back(satellite);

            counter++;
        }

        // Check that exactly that number of satellites has been read in
        if (counter != num_orbits * satellites_per_orbit) {
            throw std::runtime_error("Number of satellites defined in the TLEs does not match");
        }

        fs.close();
    }

    void
    TopologySatelliteNetwork::ReadGroundStations()
    {

        // Create a new file stream to open the file
        std::ifstream fs;
        fs.open(m_satellite_network_dir + "/ground_stations.txt");
        NS_ABORT_MSG_UNLESS(fs.is_open(), "File ground_stations.txt could not be opened");

        // Read ground station from each line
        std::string line;
        while (std::getline(fs, line)) {

            std::vector<std::string> res = split_string(line, ",", 8);

            // All eight values
            uint32_t gid = parse_positive_int64(res[0]);
            std::string name = res[1];
            double latitude = parse_double(res[2]);
            double longitude = parse_double(res[3]);
            double elevation = parse_double(res[4]);
            double cartesian_x = parse_double(res[5]);
            double cartesian_y = parse_double(res[6]);
            double cartesian_z = parse_double(res[7]);
            Vector cartesian_position(cartesian_x, cartesian_y, cartesian_z);

            // Create ground station data holder
            Ptr<GroundStation> gs = CreateObject<GroundStation>(
                    gid, name, latitude, longitude, elevation, cartesian_position
            );
            m_groundStations.push_back(gs);

            // Create the node
            m_groundStationNodes.Create(1);
            if (m_groundStationNodes.GetN() != gid + 1) {
                throw std::runtime_error("GID is not incremented each line");
            }

            // Install the constant mobility model on the node
            MobilityHelper mobility;
            mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
            mobility.Install(m_groundStationNodes.Get(gid));
            Ptr<MobilityModel> mobilityModel = m_groundStationNodes.Get(gid)->GetObject<MobilityModel>();
            mobilityModel->SetPosition(cartesian_position);

        }

        fs.close();
    }

    void
    TopologySatelliteNetwork::InstallInternetStacks(const Ipv4RoutingHelper& ipv4RoutingHelper) {
        InternetStackHelper internet;
        internet.SetRoutingHelper(ipv4RoutingHelper);
        internet.Install(m_allNodes);
    }

    void
    TopologySatelliteNetwork::ReadISLs()
    {

        // Link helper
        PointToPointLaserHelper p2p_laser_helper;
        std::string max_queue_size_str = format_string("%" PRId64 "p", m_isl_max_queue_size_pkts);
        p2p_laser_helper.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue(QueueSize(max_queue_size_str)));
        p2p_laser_helper.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (std::to_string(m_isl_data_rate_megabit_per_s) + "Mbps")));
        std::cout << "    >> ISL data rate........ " << m_isl_data_rate_megabit_per_s << " Mbit/s" << std::endl;
        std::cout << "    >> ISL max queue size... " << m_isl_max_queue_size_pkts << " packets" << std::endl;

        // Traffic control helper
        TrafficControlHelper tch_isl;
        tch_isl.SetRootQueueDisc("ns3::FifoQueueDisc", "MaxSize", QueueSizeValue(QueueSize("1p"))); // Will be removed later any case

        // Open file
        std::ifstream fs;
        fs.open(m_satellite_network_dir + "/isls.txt");
        NS_ABORT_MSG_UNLESS(fs.is_open(), "File isls.txt could not be opened");

        // Read ISL pair from each line
        std::string line;
        int counter = 0;
        while (std::getline(fs, line)) {
            std::vector<std::string> res = split_string(line, " ", 2);

            // Retrieve satellite identifiers
            int32_t sat0_id = parse_positive_int64(res.at(0));
            int32_t sat1_id = parse_positive_int64(res.at(1));
            Ptr<Satellite> sat0 = m_satellites.at(sat0_id);
            Ptr<Satellite> sat1 = m_satellites.at(sat1_id);

            // Install a p2p laser link between these two satellites
            NodeContainer c;
            c.Add(m_satelliteNodes.Get(sat0_id));
            c.Add(m_satelliteNodes.Get(sat1_id));
            NetDeviceContainer netDevices = p2p_laser_helper.Install(c);

            // Install traffic control helper
            tch_isl.Install(netDevices.Get(0));
            tch_isl.Install(netDevices.Get(1));

            // Assign some IP address (nothing smart, no aggregation, just some IP address)
            m_ipv4_helper.Assign(netDevices);
            m_ipv4_helper.NewNetwork();

            // Remove the traffic control layer (must be done here, else the Ipv4 helper will assign a default one)
            TrafficControlHelper tch_uninstaller;
            tch_uninstaller.Uninstall(netDevices.Get(0));
            tch_uninstaller.Uninstall(netDevices.Get(1));

            // Utilization tracking
            if (m_enable_isl_utilization_tracking) {
                netDevices.Get(0)->GetObject<PointToPointLaserNetDevice>()->EnableUtilizationTracking(m_isl_utilization_tracking_interval_ns);
                netDevices.Get(1)->GetObject<PointToPointLaserNetDevice>()->EnableUtilizationTracking(m_isl_utilization_tracking_interval_ns);

                m_islNetDevices.Add(netDevices.Get(0));
                m_islFromTo.push_back(std::make_pair(sat0_id, sat1_id));
                m_islNetDevices.Add(netDevices.Get(1));
                m_islFromTo.push_back(std::make_pair(sat1_id, sat0_id));
            }

            counter += 1;
        }
        fs.close();

        // Completed
        std::cout << "    >> Created " << std::to_string(counter) << " ISL(s)" << std::endl;

    }

    void
    TopologySatelliteNetwork::CreateGSLs() {

        // Link helper
        GSLHelper gsl_helper;
        std::string max_queue_size_str = format_string("%" PRId64 "p", m_gsl_max_queue_size_pkts);
        gsl_helper.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue(QueueSize(max_queue_size_str)));
        gsl_helper.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (std::to_string(m_gsl_data_rate_megabit_per_s) + "Mbps")));
        std::cout << "    >> GSL data rate........ " << m_gsl_data_rate_megabit_per_s << " Mbit/s" << std::endl;
        std::cout << "    >> GSL max queue size... " << m_gsl_max_queue_size_pkts << " packets" << std::endl;

        // Traffic control helper
        TrafficControlHelper tch_gsl;
        tch_gsl.SetRootQueueDisc("ns3::FifoQueueDisc", "MaxSize", QueueSizeValue(QueueSize("1p")));  // Will be removed later any case

        // Check that the file exists
        std::string filename = m_satellite_network_dir + "/gsl_interfaces_info.txt";
        if (!file_exists(filename)) {
            throw std::runtime_error(format_string("File %s does not exist.", filename.c_str()));
        }

        // Read file contents
        std::string line;
        std::ifstream fstate_file(filename);
        std::vector<std::tuple<int32_t, double>> node_gsl_if_info;
        uint32_t total_num_gsl_ifs = 0;
        if (fstate_file) {
            size_t line_counter = 0;
            while (getline(fstate_file, line)) {
                std::vector<std::string> comma_split = split_string(line, ",", 3);
                int64_t node_id = parse_positive_int64(comma_split[0]);
                int64_t num_ifs = parse_positive_int64(comma_split[1]);
                double agg_bandwidth = parse_positive_double(comma_split[2]);
                if ((size_t) node_id != line_counter) {
                    throw std::runtime_error("Node id must be incremented each line in GSL interfaces info");
                }
                node_gsl_if_info.push_back(std::make_tuple((int32_t) num_ifs, agg_bandwidth));
                total_num_gsl_ifs += num_ifs;
                line_counter++;
            }
            fstate_file.close();
        } else {
            throw std::runtime_error(format_string("File %s could not be read.", filename.c_str()));
        }
        std::cout << "    >> Read all GSL interfaces information for the " << node_gsl_if_info.size() << " nodes" << std::endl;
        std::cout << "    >> Number of GSL interfaces to create... " << total_num_gsl_ifs << std::endl;

        // Create and install GSL network devices
        NetDeviceContainer devices = gsl_helper.Install(m_satelliteNodes, m_groundStationNodes, node_gsl_if_info);
        std::cout << "    >> Finished install GSL interfaces (interfaces, network devices, one shared channel)" << std::endl;

        // Install queueing disciplines
        tch_gsl.Install(devices);
        std::cout << "    >> Finished installing traffic control layer qdisc which will be removed later" << std::endl;

        // Assign IP addresses
        //
        // This is slow because of an inefficient implementation, if you want to speed it up, you can need to edit:
        // src/internet/helper/ipv4-address-helper.cc
        //
        // And then within function Ipv4AddressHelper::NewAddress (void), comment out:
        // Ipv4AddressGenerator::AddAllocated (addr);
        //
        // Beware that if you do this, and there are IP assignment conflicts, they are not detected.
        //
        std::cout << "    >> Assigning IP addresses..." << std::endl;
        std::cout << "       (with many interfaces, this can take long due to an inefficient IP assignment conflict checker)" << std::endl;
        std::cout << "       Progress (as there are more entries, it becomes slower):" << std::endl;
        int64_t start_time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        int64_t last_time_ns = start_time_ns;
        for (uint32_t i = 0; i < devices.GetN(); i++) {

            // Assign IPv4 address
            m_ipv4_helper.Assign(devices.Get(i));
            m_ipv4_helper.NewNetwork();

            // Give a progress update if at an even 10%
            int update_interval = (int) std::ceil(devices.GetN() / 10.0);
            if (((i + 1) % update_interval) == 0 || (i + 1) == devices.GetN()) {
                int64_t now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                printf("       - %.2f%% (t = %.2f s, update took %.2f s)\n",
                    (float) (i + 1) / (float) devices.GetN() * 100.0,
                    (now_ns - start_time_ns) / 1e9,
                    (now_ns - last_time_ns) / 1e9
                );
                last_time_ns = now_ns;
            }

        }
        std::cout << "    >> Finished assigning IPs" << std::endl;

        // Remove the traffic control layer (must be done here, else the Ipv4 helper will assign a default one)
        TrafficControlHelper tch_uninstaller;
        std::cout << "    >> Removing traffic control layers (qdiscs)..." << std::endl;
        for (uint32_t i = 0; i < devices.GetN(); i++) {
            tch_uninstaller.Uninstall(devices.Get(i));
        }
        std::cout << "    >> Finished removing GSL queueing disciplines" << std::endl;

        // Check that all interfaces were created
        NS_ABORT_MSG_IF(total_num_gsl_ifs != devices.GetN(), "Not the expected amount of interfaces has been created.");

        std::cout << "    >> GSL interfaces are setup" << std::endl;

    }

    void
    TopologySatelliteNetwork::PopulateArpCaches() {

        // ARP lookups hinder performance, and actually won't succeed, so to prevent that from happening,
        // all GSL interfaces' IPs are added into an ARP cache

        // ARP cache with all ground station and satellite GSL channel interface info
        Ptr<ArpCache> arpAll = CreateObject<ArpCache>();
        arpAll->SetAliveTimeout (Seconds(3600 * 24 * 365)); // Valid one year

        // Satellite ARP entries
        for (uint32_t i = 0; i < m_allNodes.GetN(); i++) {

            // Information about all interfaces (TODO: Only needs to be GSL interfaces)
            for (size_t j = 1; j < m_allNodes.Get(i)->GetObject<Ipv4>()->GetNInterfaces(); j++) {
                Mac48Address mac48Address = Mac48Address::ConvertFrom(m_allNodes.Get(i)->GetObject<Ipv4>()->GetNetDevice(j)->GetAddress());
                Ipv4Address ipv4Address = m_allNodes.Get(i)->GetObject<Ipv4>()->GetAddress(j, 0).GetLocal();

                // Add the info of the GSL interface to the cache
                ArpCache::Entry * entry = arpAll->Add(ipv4Address);
                entry->SetMacAddress(mac48Address);

                // Set a pointer to the ARP cache it should use (will be filled at the end of this function, it's only a pointer)
                m_allNodes.Get(i)->GetObject<Ipv4L3Protocol>()->GetInterface(j)->SetAttribute("ArpCache", PointerValue(arpAll));

            }

        }

    }

    void TopologySatelliteNetwork::CollectUtilizationStatistics() {
        if (m_enable_isl_utilization_tracking) {

            // Open CSV file
            FILE* file_utilization_csv = fopen((m_basicSimulation->GetLogsDir() + "/isl_utilization.csv").c_str(), "w+");

            // Go over every ISL network device
            for (size_t i = 0; i < m_islNetDevices.GetN(); i++) {
                Ptr<PointToPointLaserNetDevice> dev = m_islNetDevices.Get(i)->GetObject<PointToPointLaserNetDevice>();
                const std::vector<double> utilization = dev->FinalizeUtilization();
                std::pair<int32_t, int32_t> src_dst = m_islFromTo[i];
                int64_t interval_left_side_ns = 0;
                for (size_t j = 0; j < utilization.size(); j++) {

                    // Only write if it is the last one, or if the utilization is different from the next
                    if (j == utilization.size() - 1 || utilization[j] != utilization[j + 1]) {

                        // Write plain to the CSV file:
                        // <src>,<dst>,<interval start (ns)>,<interval end (ns)>,<utilization 0.0-1.0>
                        fprintf(file_utilization_csv,
                                "%d,%d,%" PRId64 ",%" PRId64 ",%f\n",
                                src_dst.first,
                                src_dst.second,
                                interval_left_side_ns,
                                (j + 1) * m_isl_utilization_tracking_interval_ns,
                                utilization[j]
                        );

                        interval_left_side_ns = (j + 1) * m_isl_utilization_tracking_interval_ns;

                    }
                }
            }

            // Close CSV file
            fclose(file_utilization_csv);

        }
    }

    uint32_t TopologySatelliteNetwork::GetNumSatellites() {
        return m_satelliteNodes.GetN();
    }

    uint32_t TopologySatelliteNetwork::GetNumGroundStations() {
        return m_groundStationNodes.GetN();
    }

    const NodeContainer& TopologySatelliteNetwork::GetNodes() {
        return m_allNodes;
    }

    int64_t TopologySatelliteNetwork::GetNumNodes() {
        return m_allNodes.GetN();
    }

    const NodeContainer& TopologySatelliteNetwork::GetSatelliteNodes() {
        return m_satelliteNodes;
    }

    const NodeContainer& TopologySatelliteNetwork::GetGroundStationNodes() {
        return m_groundStationNodes;
    }

    const std::vector<Ptr<GroundStation>>& TopologySatelliteNetwork::GetGroundStations() {
        return m_groundStations;
    }

    const std::vector<Ptr<Satellite>>& TopologySatelliteNetwork::GetSatellites() {
        return m_satellites;
    }

    void TopologySatelliteNetwork::EnsureValidNodeId(uint32_t node_id) {
        if (node_id < 0 || node_id >= m_satellites.size() + m_groundStations.size()) {
            throw std::runtime_error("Invalid node identifier.");
        }
    }

    bool TopologySatelliteNetwork::IsSatelliteId(uint32_t node_id) {
        EnsureValidNodeId(node_id);
        return node_id < m_satellites.size();
    }

    bool TopologySatelliteNetwork::IsGroundStationId(uint32_t node_id) {
        EnsureValidNodeId(node_id);
        return node_id >= m_satellites.size() && node_id ;
    }

    const Ptr<Satellite> TopologySatelliteNetwork::GetSatellite(uint32_t satellite_id) {
        if (satellite_id >= m_satellites.size()) {
            throw std::runtime_error("Cannot retrieve satellite with an invalid satellite ID");
        }
        Ptr<Satellite> satellite = m_satellites.at(satellite_id);
        return satellite;
    }

    uint32_t TopologySatelliteNetwork::NodeToGroundStationId(uint32_t node_id) {
        EnsureValidNodeId(node_id);
        return node_id - GetNumSatellites();
    }

    bool TopologySatelliteNetwork::IsValidEndpoint(int64_t node_id) {
        return m_endpoints.find(node_id) != m_endpoints.end();
    }

    const std::set<int64_t>& TopologySatelliteNetwork::GetEndpoints() {
        return m_endpoints;
    }

}
