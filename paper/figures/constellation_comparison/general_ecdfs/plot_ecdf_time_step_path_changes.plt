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
set style line 1 lt rgb "#2177B0" lw 4 pt 1 ps 0
set style line 2 lt rgb "#FC7F2B" lw 4 pt 2 ps 0 dt 2
set style line 3 lt rgb "#2F9E37" lw 4 pt 3 ps 0 dt 3
set style line 4 lt rgb "#D42A2D" lw 2.4 pt 4 ps 1.4

# Output
set output "pdf/ecdf_time_step_path_changes.pdf"

#####################################
### AXES AND KEY

# Axes labels
set xlabel "# of path changes in a time step" # Markup: e.g. 99^{th}, {/Symbol s}, {/Helvetica-Italic P}
set ylabel "ECDF (time steps)"

# Axes ranges
set xrange [1:]       # Explicitly set the x-range [lower:upper]
set yrange [0:]       # Explicitly set the y-range [lower:upper]
# set xtics (0, 100, 300, 500, 700, 900)
# set ytics <start>, <incr> {,<end>}
# set format x "%.2f%%"  # Set the x-tic format, e.g. in this case it takes 2 sign. decimals: "24.13%""
set log x

# For logarithmic axes
# set log x           # Set logarithmic x-axis
# set log y           # Set logarithmic y-axis
# set mxtics 3        # Set number of intermediate tics on x-axis (for log plots)
# set mytics 3        # Set number of intermediate tics on y-axis (for log plots)

# Font of the key (a.k.a. legend)
set key font ",18"
set key reverse
set key bottom right Left
set key spacing 2

#####################################
### PLOTS
set datafile separator ","
plot    "../../../satgenpy_analysis/data/kuiper_630_isls_plus_grid_ground_stations_top_100_algorithm_free_one_only_over_isls/50ms_for_200s/path/data/ecdf_time_step_num_path_changes.txt" using ($1):($2) title "50ms"  with steps ls 1, \
        "../../../satgenpy_analysis/data/kuiper_630_isls_plus_grid_ground_stations_top_100_algorithm_free_one_only_over_isls/100ms_for_200s/path/data/ecdf_time_step_num_path_changes.txt" using ($1):($2) title "100ms"  with steps ls 2, \
        "../../../satgenpy_analysis/data/kuiper_630_isls_plus_grid_ground_stations_top_100_algorithm_free_one_only_over_isls/1000ms_for_200s/path/data/ecdf_time_step_num_path_changes.txt" using ($1):($2) title "1000ms"  with steps ls 3, \
