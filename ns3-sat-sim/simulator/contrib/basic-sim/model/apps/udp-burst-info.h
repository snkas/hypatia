#ifndef UDP_BURST_INFO_H
#define UDP_BURST_INFO_H

#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <cstring>
#include <fstream>
#include <cinttypes>
#include <algorithm>
#include <regex>

namespace ns3 {

    class UdpBurstInfo
    {
    public:
        UdpBurstInfo(
                int64_t udp_burst_id,
                int64_t from_node_id,
                int64_t to_node_id,
                double target_rate_megabit_per_s,
                int64_t start_time_ns,
                int64_t duration_ns,
                std::string additional_parameters,
                std::string metadata
        );
        int64_t GetUdpBurstId() const;
        int64_t GetFromNodeId();
        int64_t GetToNodeId();
        double GetTargetRateMegabitPerSec();
        int64_t GetStartTimeNs();
        int64_t GetDurationNs();
        std::string GetAdditionalParameters();
        std::string GetMetadata();
    private:
        int64_t m_udp_burst_id;
        int64_t m_from_node_id;
        int64_t m_to_node_id;
        double m_target_rate_megabit_per_s;
        int64_t m_start_time_ns;
        int64_t m_duration_ns;
        std::string m_additional_parameters;
        std::string m_metadata;
    };

}

#endif /* UDP_BURST_INFO_H */
