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
import os

local_shell = exputil.LocalShell()

# Remove
local_shell.remove_force_recursive("pdf")
local_shell.make_full_dir("pdf")
local_shell.remove_force_recursive("data")
local_shell.make_full_dir("data")

cubic = ""
reno = ""

def list_directories(folder_path):
    directories = []
    for item in os.listdir(folder_path):
        item_path = os.path.join(folder_path, item)
        if os.path.isdir(item_path):
            directories.append(item_path)
    return directories

def process_progress_files(directory):
    global cubic
    global reno
    file = f"{directory}/tcp_flows.txt"
    with open(file, 'r') as file:
        lines = file.readlines()
        if lines:
            lines = lines[1:]
            count = len(lines)
            for line in lines:
                data = f"{int(count)/2},{line.split()[1]},{float(line.split()[9]) / float(line.split()[7]) * 1000}\n"
                if "Cubic" in directory:
                    cubic = cubic + data
                else:
                    reno = reno + data

def plot(folder, id):
    local_shell.perfect_exec(
        "cd ../../../ns3-sat-sim/simulator/contrib/basic-sim/tools/plotting/plot_tcp_flow; "
        "python plot_tcp_flow.py "
        f"../../../../../../../paper/ns3_experiments/custom_experiment/runs/{folder}/logs_ns3 "
        f"../../../../../../../paper/ns3_experiments/custom_experiment/data/{folder} "
        f"../../../../../../../paper/ns3_experiments/custom_experiment/pdf/{folder} "
        f"{id} " + str(1 * 1000 * 1000 * 1000),  # Flow 0, 1 * 1000 * 1000 * 1000 ns = 1s interval
        output_redirect=exputil.OutputRedirect.CONSOLE
    )

directories = list_directories("runs")

for directory in directories:
    process_progress_files(directory + "/logs_ns3")

plot("starlink_TcpCubic_1", 0)
plot("starlink_TcpCubic_1", 1)
plot("starlink_TcpNewReno_1", 0)
plot("starlink_TcpNewReno_1", 1)

with open("data/cubic.csv", 'w') as f:
    f.write(cubic)

with open("data/newreno.csv", 'w') as f:
    f.write(reno)

