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

# Usage help
if [ "$1" == "--help" ] || [ "$#" != "2" ]; then
  echo "Usage: bash generate_for_paper.sh [id: 0 - 14] [number of threads]"
  exit 0
fi

# Fetch arguments
id="$1"
num_threads=$2

# Check validity of arguments
if [ "${id}" -lt "0" ] || [ "${id}" -gt "14" ]; then
  echo "Invalid workload id: ${id}"
  exit 1
fi
if [ "${num_threads}" -lt "0" ] || [ "${num_threads}" -gt "128" ]; then
  echo "Invalid number of threads: ${num_threads}"
  exit 1
fi

# Print what is being run
echo "Running workload ${id} with ${num_threads} threads"

# Kuiper-630 without ISLS, only ground station relays
if [ "${id}" = "0" ]; then
  python main_kuiper_630.py 200 50 isls_none ground_stations_paris_moscow_grid algorithm_free_one_only_gs_relays ${num_threads}
fi
if [ "${id}" = "1" ]; then
  python main_kuiper_630.py 200 100 isls_none ground_stations_paris_moscow_grid algorithm_free_one_only_gs_relays ${num_threads}
fi
if [ "${id}" = "2" ]; then
  python main_kuiper_630.py 200 1000 isls_none ground_stations_paris_moscow_grid algorithm_free_one_only_gs_relays ${num_threads}
fi

# Kuiper-630 with ISLs
if [ "${id}" = "3" ]; then
  python main_kuiper_630.py 200 50 isls_plus_grid ground_stations_top_100 algorithm_free_one_only_over_isls ${num_threads}
fi
if [ "${id}" = "4" ]; then
  python main_kuiper_630.py 200 100 isls_plus_grid ground_stations_top_100 algorithm_free_one_only_over_isls ${num_threads}
fi
if [ "${id}" = "5" ]; then
  python main_kuiper_630.py 200 1000 isls_plus_grid ground_stations_top_100 algorithm_free_one_only_over_isls ${num_threads}
fi

# Starlink-550 with ISLs
if [ "${id}" = "6" ]; then
  python main_starlink_550.py 200 50 isls_plus_grid ground_stations_top_100 algorithm_free_one_only_over_isls ${num_threads}
fi
if [ "${id}" = "7" ]; then
  python main_starlink_550.py 200 100 isls_plus_grid ground_stations_top_100 algorithm_free_one_only_over_isls ${num_threads}
fi
if [ "${id}" = "8" ]; then
  python main_starlink_550.py 200 1000 isls_plus_grid ground_stations_top_100 algorithm_free_one_only_over_isls ${num_threads}
fi

# Telesat-1015 with ISLs
if [ "${id}" = "9" ]; then
  python main_telesat_1015.py 200 50 isls_plus_grid ground_stations_top_100 algorithm_free_one_only_over_isls ${num_threads}
fi
if [ "${id}" = "10" ]; then
  python main_telesat_1015.py 200 100 isls_plus_grid ground_stations_top_100 algorithm_free_one_only_over_isls ${num_threads}
fi
if [ "${id}" = "11" ]; then
  python main_telesat_1015.py 200 1000 isls_plus_grid ground_stations_top_100 algorithm_free_one_only_over_isls ${num_threads}
fi

# Legacy 25x25 with ISLs
# (Not used in final paper, was for initial testing), for legacy reasons still here
if [ "${id}" = "12" ]; then
  python main_25x25.py 200 50 algorithm_free_one_only_over_isls ${num_threads}
fi
if [ "${id}" = "13" ]; then
  python main_25x25.py 200 100 algorithm_free_one_only_over_isls ${num_threads}
fi
if [ "${id}" = "14" ]; then
  python main_25x25.py 200 1000 algorithm_free_one_only_over_isls ${num_threads}
fi
