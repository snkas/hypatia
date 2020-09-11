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

local_shell = exputil.LocalShell()

# Paths
temp_dir_paths = [

    # Satellite networks state
    "satellite_networks_state/gen_data",

    # Figures
    "figures/a_b/multiple_rtt_matching/pdf",
    "figures/a_b/tcp_cwnd/pdf",
    "figures/a_b/tcp_isls_vs_gs_relays/pdf",
    "figures/a_b/tcp_mayhem/pdf",
    "figures/a_b/tcp_rate/pdf",
    "figures/constellation_comparison/general_ecdfs/pdf",
    "figures/traffic_matrix_load_scalability/pdf",
    "figures/traffic_matrix_unused_bandwidth/pdf",

    # Satgenpy analysis
    "satgenpy_analysis/data",

    # ns-3 experiments
    "ns3_experiments/a_b/runs",
    "ns3_experiments/a_b/data",
    "ns3_experiments/a_b/pdf",
    "ns3_experiments/traffic_matrix/extra_satgenpy_analysis_data",
    "ns3_experiments/traffic_matrix/runs",
    "ns3_experiments/traffic_matrix/data",
    "ns3_experiments/traffic_matrix/pdf",
    "ns3_experiments/traffic_matrix_load/runs",
    "ns3_experiments/traffic_matrix_load/data",
    "ns3_experiments/traffic_matrix_load/pdf",

]

# Check it exists
if not local_shell.file_exists("hypatia_paper_temp_data.tar.gz"):
    print("hypatia_paper_temp_data.tar.gz is not located in the paper directory; please put it there.")

# Check to be sure this is what the user wants
if input("Are you sure you want to extract? It will overwrite any existing temporary data. (yes/no): ") != "yes":
    print("Did not receive a 'yes', so not doing extraction.")
    exit(1)

# Extract it into temp_data
print("Copying archive into temp_data/ directory")
local_shell.remove_force_recursive("temp_data")
local_shell.perfect_exec("tar -xzvf hypatia_paper_temp_data.tar.gz")
print("  > Extraction successful")
print("")

# Remove the existing directories
print("Remove and write temporary directories")
for temp_dir_path in temp_dir_paths:
    flat_directory_name = temp_dir_path.replace("/", "_")
    print("  > Deleting..... " + temp_dir_path)
    local_shell.remove_force_recursive(temp_dir_path)
    print("  > Creating..... " + temp_dir_path)
    local_shell.make_full_dir(temp_dir_path)
    print("  > Extracting... " + temp_dir_path)
    local_shell.perfect_exec("scp -r temp_data/%s/* %s/" % (flat_directory_name, temp_dir_path))
print("")

# Cleaning up
print("Removing temporary temp_data/ directory which was used for extraction")
local_shell.remove_force_recursive("temp_data")
print("")

# Finished
print("Finished.")
