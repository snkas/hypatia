# Satgenpy analysis

Given that you have generated the satellite network state data over time in `satellite_networks_state`,
here you can there is an analysis of the constellations, for it both as a whole, as well as for a
few particular pairs.


## Getting started

1. Make sure you have all dependencies installed as prescribed in `<hypatia>/satgenpy/README.md`

2. Perform the full analysis (takes some time):
   ```
   python perform_full_analysis.py
   ```

3. The analysis for each constellation is now in `data/<satellite network name>`
