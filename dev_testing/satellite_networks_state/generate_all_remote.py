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

# Setup parameters
# TODO: You have to configure these to match your own machine setup
remote_user = "user"
remote_path_to_satellite_network_state_dir = "/path/to/hypatia/paper/satellite_networks_state"
machines = [
    "machine0", "machine1", "machine2"
]

# Check for reachability
for machine in machines:
    print(machine)
    remote_shell = exputil.RemoteShell(remote_user, machine)
    remote_shell.perfect_exec("echo Reached", output_redirect=exputil.OutputRedirect.CONSOLE)

# How difficult each task is (estimated)
task_weight = {
    0: 5,
    1: 3,
    2: 1,
    3: 5,
    4: 3,
    5: 1,
    6: 5,
    7: 3,
    8: 1,
    9: 4,
    10: 3,
    11: 1,
    12: 3,
    13: 2,
    14: 1,
}

# Assign tasks
machine_tasks = []
for i in range(len(machines)):
    machine_tasks.append([])
machine_total_weight = [0.0] * len(machines)
for tid in range(15):
    lowest_machine_id = 0
    lowest_weight = 10000000000
    for i in range(len(machines)):
        if machine_total_weight[i] < lowest_weight:
            lowest_weight = machine_total_weight[i]
            lowest_machine_id = i
    machine_total_weight[lowest_machine_id] += task_weight[tid]
    machine_tasks[lowest_machine_id].append(tid)

# Start all tasks
for i in range(len(machines)):
    print("Running on: " + machines[i])
    remote_shell = exputil.RemoteShell(remote_user, machines[i])

    # Kill all other tasks
    remote_shell.killall("screen")

    # Start each task assigned to this machine
    for tid in machine_tasks[i]:
        remote_shell.detached_exec(
            "cd %s; bash generate_for_paper.sh %d %d" % (
                remote_path_to_satellite_network_state_dir,
                tid,
                int(24.0 / len(machine_tasks[i]))
            ),
            keep_alive=True
        )

    # Check that all tasks started
    remote_shell.perfect_exec("sleep 1")
    if remote_shell.count_screens() != len(machine_tasks[i]):
        raise ValueError("It went wrong")
