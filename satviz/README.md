# SatViz: Satellite Visualization

SatViz is a Cesium-based visualization pipeline to generate interactive
satellite network visualizations. It makes use of the online Cesium API
by generating CesiumJS code. The API calls require its user to obtain 
a Cesium access token (via [https://cesium.com/]()).

## Getting started

1. Obtain a Cesium access token at [https://cesium.com/]()

2. Edit `static_html/top.html`, and insert your Cesium access 
   token at line 10:

   ```javascript
   Cesium.Ion.defaultAccessToken = '<CESIUM_ACCESS_TOKEN>';
   ```

3. Now you are able to make use of the scripts in `scripts/`


## Script description

1. `visualize_constellation.py`: Generates visualizations for entire constellation (multiple shells).

2. `visualize_horizon_over_time.py`: Finds satellite positions (azimuth, altitude) over time for a static observer and plots them relative to the observer.

3. `visualize_path.py`: Visualizes paths between pairs of endpoints at specific time instances.

4. `visualize_path_no_isl.py`: Visualizes paths between pairs of endpoints when no inter-satellite connectivity exists.

5. `visualize_path_wise_utilization.py`: Visualizes link utilization for specific end-end paths at a specific time instance.

6. `visualize_utilization.py`: Visualizes link utilization for all end-end paths at a specific time instance.

## Visualizations in the paper

1. `Fig. 11: Constellation trajectories`: Use script `visualize_constellation.py` to generate constellations. To generate Starlink, Kuiper, or Telesat, one should uncomment the corresponding parameter block in the script, and comment out the other parameter blocks. The default is Starlink 5-shell.

2. `Fig. 12: Ground observer's view`: Use script `visualize_horizon_over_time.py`. To change observer location, change the `LOCATION` coordinates. In order to visualize for X seconds at a granularity of Y seconds, set `VIZ_TIME = X` and `VIZ_GRAN = Y`. The default values are: `LOCATION = (59.9311, 30.3609)` corresponding to St. Petersburg, `X = 170`, and `Y = 5`.

3. `Fig. 13: Shortest path changes over time`: Use script `visualize_path.py`. Change the values of `GEN_TIME` and `path_file` for various visualization generation times and city pairs respectively. The default values generate `Fig. 13 (left)`. The same script can be used to generate `Fig. 16(a) and 17(a)`.

4. `Fig. 14: Congestion shifts over time`: Use script `visualize_path_wise_utilization.py`. Change the values of `GEN_TIME`, `path_file`, and `IN_UTIL_FILE` for specifying visualization time, end-to-end path, and utilization. The default values generate `Fig. 14 (top)`.

5. `Fig. 15: Constellation-wide utilization`: Use script `visualize_utilization.py`. Change the values of `GEN_TIME` and `IN_UTIL_FILE` for specifying visualization generation time and utilization. The default values generate `Fig. 15`.

6. `Fig. 16(b) and 17(b)`: Use script `visualize_path_no_isl.py` to visualize paths when constellation does not have inter-satellite connectivity. Change the values of `GEN_TIME` and `path_file` for various visualization generation times and city pairs respectively. The default values generate `Fig 17(b)`.