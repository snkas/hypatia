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
