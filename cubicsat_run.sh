# script to run our custom experiments for TCP Cubic

# build ns3 3.41 (has TCP Cubic)
./hypatia_install_dependencies.sh
./hypatia_build.sh

# get pregenerated satellite data
cd paper
wget https://github.com/snkas/hypatia/releases/download/v1/hypatia_paper_temp_data.tar.gz
# this may take a while but repond with 'yes' when prompted
python3 extract_temp_data.py

# generate paths and delays for simulation
cd satgenpy_analysis
python3 custom_analysis.py
cd ..

# run experiment
# graphs and data will be in the 'pdf' and 'data' folders
./custom_experiment.sh

# generate visualization
# you will need to go to the IP address so see it in a browser
cd ../satviz
./cesium_run.sh
