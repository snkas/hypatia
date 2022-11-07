# Usage help
if [ "$1" == "--help" ] || [ "$#" != "1" ]; then
  echo "Usage: bash starlink_500s_100ms.sh [number of threads]"
  exit 0
fi

# Fetch arguments
num_threads=$1

# Check validity of arguments
if [ "${num_threads}" -lt "0" ] || [ "${num_threads}" -gt "128" ]; then
  echo "Invalid number of threads: ${num_threads}"
  exit 1
fi

# Print what is being run
echo "Running workload starlink 500s duration and 100ms sample with ${num_threads} threads"

python main_starlink_550.py 500 100 isls_plus_grid ground_stations_top_100 algorithm_free_one_only_over_isls ${num_threads}