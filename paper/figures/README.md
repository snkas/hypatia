# Figures

Given that you have generated all data in the `ns3_experiments` and `satgenpy_analysis/data` folders, you use this directory to generate the figures from the paper.

## Getting started

1. Execute gnuplot files:
   ```
   python plot_all.py
   ```
   
2. All .pdf versions of the figures have now been generated.

3. Convert .pdf to .png because the .pdf can be slow to load because of the amount of points in them:
   ```
   python generate_pngs.py
   ```

## Mapping paper figure numbers to the pdfs:

* Fig. 1: (Made with draw.io)
* Fig. 2: `traffic_matrix_load_scalability/pdf/plot_goodput_rate_vs_slowdown.pdf`
* Fig. 3(a): `a_b/multiple_rtt_matching/pdf/time_vs_multiple_rtt_pair_a.pdf`
* Fig. 3(b): `a_b/multiple_rtt_matching/pdf/time_vs_multiple_rtt_pair_b.pdf`
* Fig. 3(c): `a_b/multiple_rtt_matching/pdf/time_vs_multiple_rtt_pair_c.pdf`
* Fig. 4(a): `a_b/tcp_cwnd/pdf/time_vs_cwnd_and_bdp_plus_queue_pair_a.pdf`
* Fig. 4(b): `a_b/tcp_cwnd/pdf/time_vs_cwnd_and_bdp_plus_queue_pair_b.pdf`
* Fig. 4(c): `a_b/tcp_cwnd/pdf/time_vs_cwnd_and_bdp_plus_queue_pair_c.pdf`
* Fig. 5(a): `a_b/tcp_mayhem/pdf/time_vs_multiple_rtt_mayhem.pdf`
* Fig. 5(b): `a_b/tcp_mayhem/pdf/time_vs_tcp_cwnd_and_bdp_mayhem.pdf`
* Fig. 5(c): `a_b/tcp_mayhem/pdf/time_vs_tcp_rate_mayhem.pdf`
* Fig. 6: `constellation_comparison/general_ecdfs/pdf/ecdf_max_rtt_to_geodesic_slowdown.pdf`
* Fig. 7(a): `constellation_comparison/general_ecdfs/pdf/ecdf_max_rtt.pdf`
* Fig. 7(b): `constellation_comparison/general_ecdfs/pdf/ecdf_max_minus_min_rtt.pdf`
* Fig. 7(c): `constellation_comparison/general_ecdfs/pdf/ecdf_max_rtt_to_min_rtt_slowdown.pdf`
* Fig. 8(a): `constellation_comparison/general_ecdfs/pdf/ecdf_num_path_changes.pdf`
* Fig. 8(b): `constellation_comparison/general_ecdfs/pdf/ecdf_max_minus_min_hop_count.pdf`
* Fig. 8(c): `constellation_comparison/general_ecdfs/pdf/ecdf_max_hop_count_to_min_hop_count.pdf`
* Fig. 9(a): `constellation_comparison/general_ecdfs/pdf/ecdf_time_step_path_changes.pdf`
* Fig. 9(b): `constellation_comparison/general_ecdfs/pdf/histogram_missed_path_changes.pdf`
* Fig. 10: `traffic_matrix_unused_bandwidth/pdf/plot_specific_tm_time_vs_available_bandwidth_over_path.pdf`
* Fig. 11(a): (satviz visualization; to be announced)
* Fig. 11(b): (satviz visualization; to be announced)
* Fig. 11(c): (satviz visualization; to be announced)
* Fig. 12(a): (satviz visualization; to be announced)
* Fig. 12(b): (satviz visualization; to be announced)
* Fig. 13(left): (satviz visualization; to be announced)
* Fig. 13(right): (satviz visualization; to be announced)
* Fig. 14(top): (satviz visualization; to be announced)
* Fig. 14(bottom): (satviz visualization; to be announced)
* Fig. 15: (satviz visualization; to be announced)
* Fig. 16(a): (satviz visualization; to be announced)
* Fig. 16(b): (satviz visualization; to be announced)
* Fig. 17(a): (satviz visualization; to be announced)
* Fig. 17(b): (satviz visualization; to be announced)
* Fig. 18(a): `a_b/tcp_isls_vs_gs_relays/pdf/isls_vs_gs_relays_time_vs_isls_rtt.pdf`
* Fig. 18(b): `a_b/tcp_isls_vs_gs_relays/pdf/isls_vs_gs_relays_time_vs_gs_relays_rtt.pdf`
* Fig. 18(c): `a_b/tcp_isls_vs_gs_relays/pdf/isls_vs_gs_relays_time_vs_computed_rtt.pdf`
* Fig. 19(a): `a_b/tcp_isls_vs_gs_relays/pdf/isls_vs_gs_relays_time_vs_isls_tcp_cwnd_and_bdp_plus_queue.pdf`
* Fig. 19(b): `a_b/tcp_isls_vs_gs_relays/pdf/isls_vs_gs_relays_time_vs_gs_relays_tcp_cwnd_and_bdp_plus_queue.pdf`
* Fig. 19(c): `a_b/tcp_isls_vs_gs_relays/pdf/isls_vs_gs_relays_time_vs_tcp_rate.pdf`
