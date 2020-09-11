###
###  Released under the MIT License (MIT) --- see ../LICENSE
###  Copyright (c) 2014 Ankit Singla, Sangeetha Abdu Jyothi, Chi-Yao Hong,
###  Lucian Popa, P. Brighten Godfrey, Alexandra Kolla, Simon Kassing
###

#####################################
### STYLING

# Terminal (gnuplot 4.4+); Swiss neutral Helvetica font
set terminal pdfcairo font "Helvetica, 20" linewidth 1.5 rounded dashed

# Line style for axes
set style line 80 lt rgb "#808080"

# Line style for grid
set style line 81 lt 0  # Dashed
set style line 81 lt rgb "#999999"  # Grey grid

# Grey grid and border
set grid back linestyle 81
set border 3 back linestyle 80
set xtics nomirror
set ytics nomirror

# Line styles
set style line 1 lt rgb "#2177b0" lw 2.4 pt 1 ps 0
set style line 2 lt rgb "#fc7f2b" lw 2.4 pt 2 ps 0
set style line 3 lt rgb "#2f9e37" lw 2.4 pt 3 ps 0
set style line 4 lt rgb "#d42a2d" lw 2.4 pt 4 ps 1.4
set style line 5 lt rgb "#80007F" lw 2.4 pt 5 ps 1.4
set style line 6 lt rgb "#8a554c" lw 2.4 pt 6 ps 1.4
set style line 7 lt rgb "#e079be" lw 2.4 pt 0 ps 1.4
set style line 8 lt rgb "#7d7d7d" lw 2.4 pt 0 ps 1.4
set style line 9 lt rgb "#000000" lw 2.4 pt 0 ps 1.4

# Output
set output "pdf/time_vs_tcp_cwnd_and_bdp_plus_queue_pair_a.pdf"

#####################################
### AXES AND KEY

# Axes labels
set xlabel "Time (s)" # Markup: e.g. 99^{th}, {/Symbol s}, {/Helvetica-Italic P}
set ylabel "# of packets"

# Axes ranges
set xrange [0:]       # Explicitly set the x-range [lower:upper]
set yrange [0:]       # Explicitly set the y-range [lower:upper]
# set xtics (0, 100, 300, 500, 700, 900)
# set ytics <start>, <incr> {,<end>}
# set format x "%.2f%%"  # Set the x-tic format, e.g. in this case it takes 2 sign. decimals: "24.13%""

# For logarithmic axes
# set log x           # Set logarithmic x-axis
# set log y           # Set logarithmic y-axis
# set mxtics 3        # Set number of intermediate tics on x-axis (for log plots)
# set mytics 3        # Set number of intermediate tics on y-axis (for log plots)
set object 2 rect from 155.4,0 to 165.2,350 fc rgb "#77e88787" lw 1 fs noborder # fs solid 0
# Font of the key (a.k.a. legend)
set key font ",18"
set key reverse
set key top left Left
set key spacing 2
set key off

set label "BDP+Q" at 13,226 textcolor rgb "#2177b0"
set label "CWND" at 20,90 textcolor rgb "#fc7f2b"


#####################################
### PLOTS
set datafile separator ","
plot    "../../../satgenpy_analysis/data/kuiper_630_isls_plus_grid_ground_stations_top_100_algorithm_free_one_only_over_isls/100ms_for_200s/manual/data/networkx_rtt_1174_to_1229.txt" using ($1/1000000000):($2 / 1000000000 * 1250000 / 1380 + 100) title "BDP + Q" w steps ls 1, \
        "../../../ns3_experiments/a_b/data/kuiper_630_isls_1174_to_1229_with_TcpNewReno_at_10_Mbps/tcp_flow_0_cwnd.csv" using ($2/1000000000):($3/1380) title "CWND" w steps ls 2, \
