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

import exputil
import time

local_shell = exputil.LocalShell()
max_num_processes = 6

# Check that no screen is running
if local_shell.count_screens() != 0:
    print("There is a screen already running. "
          "Please kill all screens before running this analysis script (killall screen).")
    exit(1)

# Re-create data directory
local_shell.remove_force_recursive("data")
local_shell.make_full_dir("data")
local_shell.make_full_dir("data/command_logs")

# Where to store all commands
commands_to_run = []

# Manual
print("Generating commands for manually selected endpoints pair (printing of routes and RTT over time)...")

# Rio de Janeiro to St. Petersburg with only ISLs on Kuiper
commands_to_run.append("cd ../../satgenpy; python -m satgen.post_analysis.main_print_routes_and_rtt "
                       "../paper/satgenpy_analysis/data ../paper/satellite_networks_state/gen_data/"
                       "kuiper_630_isls_plus_grid_ground_stations_top_100_algorithm_free_one_only_over_isls "
                       "100 200 1174 1229 "
                       "> ../paper/satgenpy_analysis/data/command_logs/manual_kuiper_isls_1174_to_1229.log 2>&1")

# Manila to Dalian with only ISLs on Kuiper
commands_to_run.append("cd ../../satgenpy; python -m satgen.post_analysis.main_print_routes_and_rtt "
                       "../paper/satgenpy_analysis/data ../paper/satellite_networks_state/gen_data/"
                       "kuiper_630_isls_plus_grid_ground_stations_top_100_algorithm_free_one_only_over_isls "
                       "100 200 1173 1241 "
                       "> ../paper/satgenpy_analysis/data/command_logs/manual_kuiper_isls_1173_to_1241.log 2>&1")

# Istanbul to Nairobi with only ISLs on Kuiper
commands_to_run.append("cd ../../satgenpy; python -m satgen.post_analysis.main_print_routes_and_rtt "
                       "../paper/satgenpy_analysis/data ../paper/satellite_networks_state/gen_data/"
                       "kuiper_630_isls_plus_grid_ground_stations_top_100_algorithm_free_one_only_over_isls "
                       "100 200 1170 1252 "
                       "> ../paper/satgenpy_analysis/data/command_logs/manual_kuiper_isls_1170_to_1252.log 2>&1")

# Paris to Moscow with only ISLs on Kuiper
commands_to_run.append("cd ../../satgenpy; python -m satgen.post_analysis.main_print_routes_and_rtt "
                       "../paper/satgenpy_analysis/data ../paper/satellite_networks_state/gen_data/"
                       "kuiper_630_isls_plus_grid_ground_stations_top_100_algorithm_free_one_only_over_isls "
                       "100 200 1180 1177 "
                       "> ../paper/satgenpy_analysis/data/command_logs/manual_kuiper_isls_1180_to_1177.log 2>&1")
commands_to_run.append("cd ../../satgenpy; python -m satgen.post_analysis.main_print_graphical_routes_and_rtt "
                       "../paper/satgenpy_analysis/data ../paper/satellite_networks_state/gen_data/"
                       "kuiper_630_isls_plus_grid_ground_stations_top_100_algorithm_free_one_only_over_isls "
                       "100 200 1180 1177 "
                       "> ../paper/satgenpy_analysis/data/command_logs/manual_graphical_kuiper_isls_1180_to_1177.log"
                       " 2>&1")

# Chicago (1193) to Zhengzhou (1243) with only ISLs on Kuiper
commands_to_run.append("cd ../../satgenpy; python -m satgen.post_analysis.main_print_routes_and_rtt "
                       "../paper/satgenpy_analysis/data ../paper/satellite_networks_state/gen_data/"
                       "kuiper_630_isls_plus_grid_ground_stations_top_100_algorithm_free_one_only_over_isls "
                       "100 200 1193 1243 "
                       "> ../paper/satgenpy_analysis/data/command_logs/manual_kuiper_isls_1193_to_1243.log 2>&1")
commands_to_run.append("cd ../../satgenpy; python -m satgen.post_analysis.main_print_graphical_routes_and_rtt "
                       "../paper/satgenpy_analysis/data ../paper/satellite_networks_state/gen_data/"
                       "kuiper_630_isls_plus_grid_ground_stations_top_100_algorithm_free_one_only_over_isls "
                       "100 200 1193 1243 "
                       "> ../paper/satgenpy_analysis/data/command_logs/manual_graphical_kuiper_isls_1193_to_1243.log"
                       " 2>&1")

# Paris to Moscow with only ground-station relays (added a grid to support this) on Kuiper
commands_to_run.append("cd ../../satgenpy; python -m satgen.post_analysis.main_print_routes_and_rtt "
                       "../paper/satgenpy_analysis/data ../paper/satellite_networks_state/gen_data/"
                       "kuiper_630_isls_none_ground_stations_paris_moscow_grid_algorithm_free_one_only_gs_relays "
                       "100 200 1156 1232 "
                       "> ../paper/satgenpy_analysis/data/command_logs/manual_kuiper_isls_1156_to_1232.log 2>&1")

commands_to_run.append("cd ../../satgenpy; python -m satgen.post_analysis.main_print_graphical_routes_and_rtt "
                       "../paper/satgenpy_analysis/data ../paper/satellite_networks_state/gen_data/"
                       "kuiper_630_isls_none_ground_stations_paris_moscow_grid_algorithm_free_one_only_gs_relays "
                       "100 200 1156 1232 "
                       "> ../paper/satgenpy_analysis/data/command_logs/"
                       "manual_graphical_kuiper_gs_relay_1156_to_1232.log"
                       " 2>&1")


# Paris (1608) to Luanda (1650) with only ISLs on Starlink
commands_to_run.append("cd ../../satgenpy; python -m satgen.post_analysis.main_print_routes_and_rtt "
                       "../paper/satgenpy_analysis/data ../paper/satellite_networks_state/gen_data/"
                       "starlink_550_isls_plus_grid_ground_stations_top_100_algorithm_free_one_only_over_isls "
                       "100 200 1608 1650 "
                       "> ../paper/satgenpy_analysis/data/command_logs/manual_starlink_isls_1608_to_1650.log 2>&1")
commands_to_run.append("cd ../../satgenpy; python -m satgen.post_analysis.main_print_graphical_routes_and_rtt "
                       "../paper/satgenpy_analysis/data ../paper/satellite_networks_state/gen_data/"
                       "starlink_550_isls_plus_grid_ground_stations_top_100_algorithm_free_one_only_over_isls "
                       "100 200 1608 1650 "
                       "> ../paper/satgenpy_analysis/data/command_logs/manual_graphical_starlink_isls_1608_to_1650.log"
                       " 2>&1")


# Constellation comparison
print("Generating commands for constellation comparison...")
for satgenpy_generated_constellation in [
    "kuiper_630_isls_none_ground_stations_paris_moscow_grid_algorithm_free_one_only_gs_relays",
    "kuiper_630_isls_plus_grid_ground_stations_top_100_algorithm_free_one_only_over_isls",
    "starlink_550_isls_plus_grid_ground_stations_top_100_algorithm_free_one_only_over_isls",
    "telesat_1015_isls_plus_grid_ground_stations_top_100_algorithm_free_one_only_over_isls"
]:
    for duration_s in [200]:
        list_update_interval_ms = [50, 100, 1000]

        # Path
        for update_interval_ms in list_update_interval_ms:
            commands_to_run.append(
                "cd ../../satgenpy; "
                "python -m satgen.post_analysis.main_analyze_path "
                "../paper/satgenpy_analysis/data ../paper/satellite_networks_state/gen_data/%s %d %d "
                "> ../paper/satgenpy_analysis/data/command_logs/constellation_comp_path_%s_%dms_for_%ds.log "
                "2>&1" % (
                    satgenpy_generated_constellation, update_interval_ms, duration_s,
                    satgenpy_generated_constellation, update_interval_ms, duration_s
                )
            )

        # RTT
        for update_interval_ms in list_update_interval_ms:
            commands_to_run.append(
                "cd ../../satgenpy; "
                "python -m satgen.post_analysis.main_analyze_rtt "
                "../paper/satgenpy_analysis/data ../paper/satellite_networks_state/gen_data/%s %d %d "
                "> ../paper/satgenpy_analysis/data/command_logs/constellation_comp_rtt_%s_%dms_for_%ds.log "
                "2>&1" % (
                    satgenpy_generated_constellation, update_interval_ms, duration_s,
                    satgenpy_generated_constellation, update_interval_ms, duration_s
                )
            )

        # Time step path
        commands_to_run.append(
            "cd ../../satgenpy; "
            "python -m satgen.post_analysis.main_analyze_time_step_path "
            "../paper/satgenpy_analysis/data ../paper/satellite_networks_state/gen_data/%s %s %d "
            "> ../paper/satgenpy_analysis/data/command_logs/constellation_comp_time_step_path_%s_%ds.log "
            "2>&1" % (
                satgenpy_generated_constellation,
                ",".join(list(map(lambda x: str(x), list_update_interval_ms))),
                duration_s,
                satgenpy_generated_constellation,
                duration_s
            )
        )

# Run the commands
print("Running commands (at most %d in parallel)..." % max_num_processes)
for i in range(len(commands_to_run)):
    print("Starting command %d out of %d: %s" % (i + 1, len(commands_to_run), commands_to_run[i]))
    local_shell.detached_exec(commands_to_run[i])
    while local_shell.count_screens() >= max_num_processes:
        time.sleep(2)

# Awaiting final completion before exiting
print("Waiting completion of the last %d..." % max_num_processes)
while local_shell.count_screens() > 0:
    time.sleep(2)
print("Finished.")
