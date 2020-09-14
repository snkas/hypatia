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

pdf_folders = [
    "constellation_comparison/general_ecdfs/pdf",
    "traffic_matrix_unused_bandwidth/pdf",
    "traffic_matrix_load_scalability/pdf",
    "two_compete",
    "a_b/multiple_rtt_matching/pdf",
    "a_b/tcp_cwnd/pdf",
    "a_b/tcp_rate/pdf",
    "a_b/tcp_mayhem/pdf",
    "a_b/tcp_isls_vs_gs_relays/pdf",
]

for pdf_folder in pdf_folders:
    for (_, _, filenames) in walk(pdf_folder):
        for f in filenames:
            if f.endswith(".pdf"):
                print("Converting .pdf to .png: " + f)
                local_shell.perfect_exec(
                    "cd %s; pdftoppm %s %s -png" % (pdf_folder, f, f.replace(".pdf", ""))
                )
        break
