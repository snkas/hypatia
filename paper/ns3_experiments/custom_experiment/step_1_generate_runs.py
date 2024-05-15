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
import networkload
import random

local_shell = exputil.LocalShell()
local_shell.remove_force_recursive("runs")
local_shell.remove_force_recursive("pdf")
local_shell.remove_force_recursive("data")

# generate list
for num_flows in [1, 2, 4, 8]:
    for tcp in ["TcpNewReno", "TcpCubic"]:
        Starlink = 1584
        DC = Starlink + 74
        Syndey = Starlink + 84
        Houston = Starlink + 62

        run_dir = f"runs/starlink_{tcp}_{num_flows}"
        local_shell.make_full_dir(run_dir)
        local_shell.copy_file("templates/template_config_ns3.properties", run_dir + "/config_ns3.properties")
        local_shell.sed_replace_in_file_plain(run_dir + "/config_ns3.properties", "[TCP-SOCKET-TYPE]", tcp)
        local_shell.sed_replace_in_file_plain(run_dir + "/config_ns3.properties", "[IDS]", ', '.join(map(str, range(num_flows * 2))))

        list_from_to = list(zip([Syndey] * num_flows + [Houston] * num_flows, [DC] * num_flows * 2))
        time_start = list(random.sample(range(1000000000), num_flows * 2))
        
        random.shuffle(list_from_to)
        time_start.sort()

        # Write the schedule
        networkload.write_schedule(
            run_dir + "/schedule_starlink_550.csv",
            len(list_from_to),
            list_from_to,
            [1000000000000] * len(list_from_to),
            time_start
        )

