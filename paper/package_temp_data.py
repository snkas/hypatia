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

# Remove existing directory
print("Creating temp_data/ directory")
local_shell.remove_force_recursive("temp_data")
local_shell.make_dir("temp_data")
print("")

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
    "figures/two_compete/pdf",

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
    "ns3_experiments/two_compete/runs",
    "ns3_experiments/two_compete/data",
    "ns3_experiments/two_compete/pdf",
    "ns3_experiments/two_compete/extra_satgenpy_analysis_data",

]

# Make all the directories we are going to package
print("Copying over temporary directories into temp_data/")
for temp_dir_path in temp_dir_paths:
    flat_directory_name = temp_dir_path.replace("/", "_")
    print("  > Copying over... " + temp_dir_path)
    local_shell.make_dir("temp_data/%s" % flat_directory_name)
    if not local_shell.path_exists(temp_dir_path):
        print("Failure -- directory does not exist: " + temp_dir_path)
        exit(1)
    local_shell.perfect_exec("scp -r %s/* temp_data/%s/" % (temp_dir_path, flat_directory_name))
print("")

# Perform cleaning commands
print("Executing cleaning command")
if not local_shell.file_exists("clean.sh"):
    print("Failure -- there must be a clean.sh to remove directory names etc.")
    print("For example, you can create an empty one if there is no cleaning needed:\n")
    print("           echo \"echo 'No cleaning'\" > clean.sh\n")
    exit(1)
print("  > Executing: bash clean.sh")
local_shell.perfect_exec("bash clean.sh", output_redirect=exputil.OutputRedirect.CONSOLE)
print("  > Finished executing clean.sh")
print("")

# Finally create the zip
print("Creating final archive")
local_shell.perfect_exec("tar -czvf hypatia_paper_temp_data.tar.gz temp_data")
print("  > Output filename... hypatia_paper_temp_data.tar.gz")
print("  > Removing temp_data/ as it is no longer needed")
local_shell.remove_force_recursive("temp_data")
print("")

# Finished
print("Finished.")
