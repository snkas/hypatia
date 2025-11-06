
# Manila to Dalian over Kuiper-630 first shell
cd test_manila_dalian_over_kuiper || exit 1
source ../../env/bin/activate
python3 step_1_generate_satellite_networks_state.py || exit 1
python3 step_2_generate_runs.py || exit 1
python3 step_3_run.py || exit 1
python3 step_4_generate_plots.py || exit 1
python3 step_5_verify.py || exit 1
cd ..
