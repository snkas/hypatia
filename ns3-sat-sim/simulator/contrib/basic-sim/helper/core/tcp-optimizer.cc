#include "ns3/tcp-optimizer.h"

namespace ns3 {

void TcpOptimizer::Generic() {

    // Clock granularity
    printf("  > Clock granularity.......... 1 ns\n");
    Config::SetDefault("ns3::TcpSocketBase::ClockGranularity", TimeValue(NanoSeconds(1)));

    // Initial congestion window
    uint32_t init_cwnd_pkts = 10;  // 1 is default, but we use 10
    printf("  > Initial CWND............... %u packets\n", init_cwnd_pkts);
    Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(init_cwnd_pkts));

    // Send buffer size
    int64_t snd_buf_size_byte = 131072 * 256;  // 131072 bytes = 128 KiB is default, we set to 32 MiB
    printf("  > Send buffer size........... %.3f MB\n", snd_buf_size_byte / 1e6);
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(snd_buf_size_byte));

    // Receive buffer size
    int64_t rcv_buf_size_byte = 131072 * 256;  // 131072 bytes = 128 KiB is default, we set to 32 MiB
    printf("  > Receive buffer size........ %.3f MB\n", rcv_buf_size_byte / 1e6);
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(rcv_buf_size_byte));

    // Segment size
    int64_t segment_size_byte = 1380;   // 536 byte is default, but we know the a point-to-point network device has an MTU of 1500.
    // IP header size: min. 20 byte, max. 60 byte
    // TCP header size: min. 20 byte, max. 60 byte
    // So, 1500 - 60 - 60 = 1380 would be the safest bet (given we don't do tunneling)
    // This could be increased higher, e.g. as discussed here:
    // https://blog.cloudflare.com/increasing-ipv6-mtu/ (retrieved April 7th, 2020)
    // In past ns-3 simulations, I've observed that the IP + TCP header is generally not larger than 80 bytes.
    // This means it could be potentially set closer to 1400-1420.
    printf("  > Segment size............... %" PRId64 " byte\n", segment_size_byte);
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(segment_size_byte));

    // Timestamp option
    bool opt_timestamp_enabled = true;  // Default: true.
    // To get an RTT measurement with a resolution of less than 1ms, it needs
    // to be disabled because the fields in the TCP Option are in milliseconds.
    // When disabling it, there are two downsides:
    //  (1) Less protection against wrapped sequence numbers (PAWS)
    //  (2) Algorithm to see if it has entered loss recovery unnecessarily are not as possible (Eiffel)
    // See: https://tools.ietf.org/html/rfc7323#section-3
    //      https://tools.ietf.org/html/rfc3522
    printf("  > Timestamp option........... %s\n", opt_timestamp_enabled ? "enabled" : "disabled");
    Config::SetDefault("ns3::TcpSocketBase::Timestamp", BooleanValue(opt_timestamp_enabled));

    // SACK option
    bool opt_sack_enabled = true;  // Default: true.
    // Selective acknowledgment improves TCP performance, however can
    // be time-consuming to simulate because set operations and merges have
    // to be performed.
    printf("  > SACK option................ %s\n", opt_sack_enabled ? "enabled" : "disabled");
    Config::SetDefault("ns3::TcpSocketBase::Sack", BooleanValue(opt_sack_enabled));

    // Window scaling option
    bool opt_win_scaling_enabled = true;  // Default: true.
    printf("  > Window scaling option...... %s\n", opt_win_scaling_enabled ? "enabled" : "disabled");
    Config::SetDefault("ns3::TcpSocketBase::WindowScaling", BooleanValue(opt_win_scaling_enabled));

    // Pacing
    bool pacing_enabled = false;  // Default: false.
    printf("  > Pacing..................... %s\n", pacing_enabled ? "enabled" : "disabled");
    Config::SetDefault("ns3::TcpSocketState::EnablePacing", BooleanValue(pacing_enabled));

}

void TcpOptimizer::OptimizeBasic(Ptr<BasicSimulation> basicSimulation) {
    std::cout << "TCP OPTIMIZATION BASIC" << std::endl;
    Generic();
    std::cout << std::endl;
    basicSimulation->RegisterTimestamp("Setup TCP parameters");
}

void TcpOptimizer::OptimizeUsingWorstCaseRtt(Ptr<BasicSimulation> basicSimulation, int64_t worst_case_rtt_ns) {
    std::cout << "TCP OPTIMIZATION USING WORST-CASE RTT" << std::endl;

    Generic();

    // Maximum segment lifetime
    int64_t max_seg_lifetime_ns = 5 * worst_case_rtt_ns; // 120s is default
    printf("  > Maximum segment lifetime... %.3f ms\n", max_seg_lifetime_ns / 1e6);
    Config::SetDefault("ns3::TcpSocketBase::MaxSegLifetime", DoubleValue(max_seg_lifetime_ns / 1e9));

    // Minimum retransmission timeout
    int64_t min_rto_ns = (int64_t)(1.5 * worst_case_rtt_ns);  // 1s is default, Linux uses 200ms
    printf("  > Minimum RTO................ %.3f ms\n", min_rto_ns / 1e6);
    Config::SetDefault("ns3::TcpSocketBase::MinRto", TimeValue(NanoSeconds(min_rto_ns)));

    // Initial RTT estimate
    int64_t initial_rtt_estimate_ns = 2 * worst_case_rtt_ns;  // 1s is default
    printf("  > Initial RTT measurement.... %.3f ms\n", initial_rtt_estimate_ns / 1e6);
    Config::SetDefault("ns3::RttEstimator::InitialEstimation", TimeValue(NanoSeconds(initial_rtt_estimate_ns)));

    // Connection timeout
    int64_t connection_timeout_ns = 2 * worst_case_rtt_ns;  // 3s is default
    printf("  > Connection timeout......... %.3f ms\n", connection_timeout_ns / 1e6);
    Config::SetDefault("ns3::TcpSocket::ConnTimeout", TimeValue(NanoSeconds(connection_timeout_ns)));

    // Delayed ACK timeout
    int64_t delayed_ack_timeout_ns = 0.2 * worst_case_rtt_ns;  // 0.2s is default
    printf("  > Delayed ACK timeout........ %.3f ms\n", delayed_ack_timeout_ns / 1e6);
    Config::SetDefault("ns3::TcpSocket::DelAckTimeout", TimeValue(NanoSeconds(delayed_ack_timeout_ns)));

    // Persist timeout
    int64_t persist_timeout_ns = 4 * worst_case_rtt_ns;  // 6s is default
    printf("  > Persist timeout............ %.3f ms\n", persist_timeout_ns / 1e6);
    Config::SetDefault("ns3::TcpSocket::PersistTimeout", TimeValue(NanoSeconds(persist_timeout_ns)));

    std::cout << std::endl;
    basicSimulation->RegisterTimestamp("Setup TCP parameters");
}

}
