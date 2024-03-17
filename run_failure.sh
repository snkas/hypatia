# Set constellation name on lines 32-33 in main_starlink_550_failure.py
# Specify failures in failure_config.txt and provide its path on line 276 in main_helper.py
# Enter desired values for variables below and run with ./run_failure.sh

echo "Running device failure pipeline..."

duration=30 # in seconds
timestep=1000 # in milliseconds
isls_choice="isls_plus_grid" # "isls_none" or "isls_plus_grid"
gs_choice="top_100" # "top_100" or "paris_moscow_grid"
algorithm="free_one_only_over_isls"  # "free_one_only_gs_relays" or "free_one_only_over_isls"
num_threads=5
src=0 # Paris
dest=9 # Moscow
constellation_base_name="starlink_550_failure_0"
constellation_name="${constellation_base_name}_${isls_choice}_ground_stations_${gs_choice}_algorithm_${algorithm}"

cd paper/satellite_networks_state
# python main_starlink_550_failure.py "$duration" "$timestep" "$isls_choice" "ground_stations_$gs_choice" "algorithm_$algorithm" "$num_threads"

cd ../../satgenpy
python -m satgen.post_analysis.main_analyze_all_pairs_failure ../out ../paper/satellite_networks_state/gen_data/$constellation_name $timestep $duration
