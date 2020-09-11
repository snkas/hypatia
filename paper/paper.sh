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

### SATELLITE NETWORKS STATE

cd satellite_networks_state || exit 1
bash generate_all_local.sh || exit 1
cd .. || exit 1

### SATGENPY ANALYSIS

# Satgenpy analysis
cd satgenpy_analysis || exit 1
python perform_full_analysis.py || exit 1
cd .. || exit 1

### NS-3 EXPERIMENTS

cd ns3_experiments || exit 1

# A to B
cd a_b || exit 1
python step_1_generate_runs.py || exit 1
python step_2_run.py || exit 1
python step_3_generate_plots.py || exit 1
cd .. || exit 1

# Traffic matrix
cd traffic_matrix || exit 1
python step_1_generate_runs.py || exit 1
python step_2_run.py || exit 1
python step_3_generate_plots.py || exit 1
cd .. || exit 1

# ns-3: Traffic matrix load
cd traffic_matrix_load || exit 1
python step_1_generate_runs.py || exit 1
python step_2_run.py || exit 1
python step_3_generate_plots.py || exit 1
cd ..

cd .. || exit 1

### Figures

cd figures || exit 1
python plot_all.py || exit 1
python generate_pngs.py || exit 1
cd .. || exit 1
