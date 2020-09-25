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

try:
    from .run_list import *
except (ImportError, SystemError):
    from run_list import *

local_shell = exputil.LocalShell()

# Remove
local_shell.remove_force_recursive("temp/pdf")
local_shell.make_full_dir("temp/pdf")
local_shell.remove_force_recursive("temp/data")
local_shell.make_full_dir("temp/data")

# TCP runs
for run in get_tcp_run_list():
    local_shell.make_full_dir("temp/pdf/" + run["name"])
    local_shell.make_full_dir("temp/data/" + run["name"])
    local_shell.perfect_exec(
        "cd ../../ns3-sat-sim/simulator/contrib/basic-sim/tools/plotting/plot_tcp_flow; "
        "python plot_tcp_flow.py "
        "../../../../../../../integration_tests/test_manila_dalian_over_kuiper/temp/runs/" + run["name"]
        + "/logs_ns3 "
        "../../../../../../../integration_tests/test_manila_dalian_over_kuiper/temp/data/" + run["name"] + " "
        "../../../../../../../integration_tests/test_manila_dalian_over_kuiper/temp/pdf/" + run["name"] + " "
        "0 " + str(1 * 1000 * 1000 * 1000),  # Flow 0, 1 * 1000 * 1000 * 1000 ns = 1s interval
        output_redirect=exputil.OutputRedirect.CONSOLE
    )
