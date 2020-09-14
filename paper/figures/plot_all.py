# The MIT License (MIT)
#
# Copyright (c) 2020 ETH Zurich
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

from os import walk
import exputil

local_shell = exputil.LocalShell()

plot_folders = [
    "constellation_comparison/general_ecdfs",
    "traffic_matrix_unused_bandwidth",
    "traffic_matrix_load_scalability",
    "two_compete",
    "a_b/multiple_rtt_matching",
    "a_b/tcp_cwnd",
    "a_b/tcp_rate",
    "a_b/tcp_mayhem",
    "a_b/tcp_isls_vs_gs_relays",
]

for plot_folder in plot_folders:
    local_shell.make_full_dir(plot_folder + "/pdf")
    for (_, _, filenames) in walk(plot_folder):
        for f in filenames:
            if f.endswith(".plt"):
                print("Executing gnuplot: " + f)
                local_shell.perfect_exec(
                    "cd %s; gnuplot %s" % (plot_folder, f),
                    output_redirect=exputil.OutputRedirect.CONSOLE
                )
        break

# Copy over a few directly

# ISLs vs. GS relays: ISLs image
local_shell.copy_file(
    "../satgenpy_analysis/data/"
    "kuiper_630_isls_plus_grid_ground_stations_top_100_algorithm_free_one_only_over_isls/"
    "100ms_for_200s/manual/pdf/graphics_1180_to_1177_time_0ms.pdf",
    "a_b/tcp_isls_vs_gs_relays/pdf/satgenpy_figure_for_isls_0ms.pdf"
)
local_shell.copy_file(
    "../satgenpy_analysis/data/"
    "kuiper_630_isls_plus_grid_ground_stations_top_100_algorithm_free_one_only_over_isls/"
    "100ms_for_200s/manual/pdf/graphics_1180_to_1177_time_159400ms.pdf",
    "a_b/tcp_isls_vs_gs_relays/pdf/satgenpy_figure_for_isls_159400ms.pdf"
)

# ISLs vs. GS relays: GS relays image
local_shell.copy_file(
    "../satgenpy_analysis/data/"
    "kuiper_630_isls_none_ground_stations_paris_moscow_grid_algorithm_free_one_only_gs_relays/"
    "100ms_for_200s/manual/pdf/graphics_1156_to_1232_time_0ms.pdf",
    "a_b/tcp_isls_vs_gs_relays/pdf/satgenpy_figure_for_gs_relays_0ms.pdf"
)
local_shell.copy_file(
    "../satgenpy_analysis/data/"
    "kuiper_630_isls_none_ground_stations_paris_moscow_grid_algorithm_free_one_only_gs_relays/"
    "100ms_for_200s/manual/pdf/graphics_1156_to_1232_time_158300ms.pdf",
    "a_b/tcp_isls_vs_gs_relays/pdf/satgenpy_figure_for_gs_relays_158300ms.pdf"
)
