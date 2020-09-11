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
set style line 1 lt rgb "#2177B0" lw 0 pt 1 ps 0
set style line 2 lt rgb "#FC7F2B" lw 0 pt 2 ps 0
set style line 3 lt rgb "#2F9E37" lw 0 pt 3 ps 0
set style line 4 lt rgb "#D42A2D" lw 2.4 pt 4 ps 1.4

# Output
set output "pdf/histogram_missed_path_changes.pdf"

#####################################
### AXES AND KEY

# Axes labels
set xlabel "Time step" # Markup: e.g. 99^{th}, {/Symbol s}, {/Helvetica-Italic P}
set ylabel "Fraction of pairs"

# Axes ranges
# set xrange [1:]       # Explicitly set the x-range [lower:upper]
# set yrange [0:]       # Explicitly set the y-range [lower:upper]
# set xtics (0, 100, 300, 500, 700, 900)
# set ytics <start>, <incr> {,<end>}
# set format x "%.2f%%"  # Set the x-tic format, e.g. in this case it takes 2 sign. decimals: "24.13%""

# For logarithmic axes
# set log x           # Set logarithmic x-axis
# set log y           # Set logarithmic y-axis
# set mxtics 3        # Set number of intermediate tics on x-axis (for log plots)
# set mytics 3        # Set number of intermediate tics on y-axis (for log plots)

# Font of the key (a.k.a. legend)
# set key font ",18"
# set key reverse
# set key bottom right Left
# set key spacing 2

# set terminal pngcairo  transparent enhanced font "arial,10" fontscale 1.0 size 600, 400
# set output 'histograms.2.png'
set boxwidth 0.9 absolute
set style fill   solid 1.00 border lt -1
set key right top vertical Right noreverse noenhanced autotitle nobox
set key at 1.85, 0.7
set label "# of path" at 1.25, 0.9
set label "changes missed" at 1.0, 0.8
set style increment default
set style histogram clustered gap 1 title textcolor lt -1
set datafile missing '-'
set style data histograms
set xtics border in scale 0,0 nomirror rotate by -45  autojustify
set xtics  norangelimit
set xtics   ()
set xrange [ * : * ] noreverse writeback
set x2range [ * : * ] noreverse writeback
set yrange [ 0.00000 :  ] noreverse writeback
set y2range [ * : * ] noreverse writeback
set zrange [ * : * ] noreverse writeback
set cbrange [ * : * ] noreverse writeback
set rrange [ * : * ] noreverse writeback
plot '../../../satgenpy_analysis/data/kuiper_630_isls_plus_grid_ground_stations_top_100_algorithm_free_one_only_over_isls/200s/path/data/histogram_missed_path_changes.txt' using 2:xtic(1) ti col ls 1, '' u 3 ti col ls 2, '' u 4 ti col ls 3
