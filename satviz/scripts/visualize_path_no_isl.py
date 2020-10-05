# MIT License
#
# Copyright (c) 2020 Debopam Bhattacherjee
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

import math
import ephem
import pandas as pd

try:
    from . import util
except (ImportError, SystemError):
    import util

# Visualizes paths between endpoints when no ISL connectivity exists

EARTH_RADIUS = 6378135.0  # WGS72 value; taken from https://geographiclib.sourceforge.io/html/NET/NETGeographicLib_8h_source.html

# CONSTELLATION GENERATION GENERAL CONSTANTS
ECCENTRICITY = 0.0000001  # Circular orbits are zero, but pyephem does not permit 0, so lowest possible value
ARG_OF_PERIGEE_DEGREE = 0.0
PHASE_DIFF = True
EPOCH = "2000-01-01 00:00:00"

# CONSTELLATION SPECIFIC PARAMETERS
"""
# STARLINK 550
NAME = "starlink_550"

################################################################
# The below constants are taken from Starlink's FCC filing as below:
# [1]: https://fcc.report/IBFS/SAT-MOD-20190830-00087
################################################################

MEAN_MOTION_REV_PER_DAY = 15.19  # Altitude ~550 km
ALTITUDE_M = 550000  # Altitude ~550 km
SATELLITE_CONE_RADIUS_M = 940700 # From https://fcc.report/IBFS/SAT-MOD-20181108-00083/1569860.pdf (minimum angle of elevation: 25 deg)
MAX_GSL_LENGTH_M = math.sqrt(math.pow(SATELLITE_CONE_RADIUS_M, 2) + math.pow(ALTITUDE_M, 2))
MAX_ISL_LENGTH_M = 2 * math.sqrt(math.pow(EARTH_RADIUS + ALTITUDE_M, 2) - math.pow(EARTH_RADIUS + 80000, 2)) # ISLs are not allowed to dip below 80 km altitude in order to avoid weather conditions
NUM_ORBS = 72
NUM_SATS_PER_ORB = 22
INCLINATION_DEGREE = 53
"""

# KUIPER 630
NAME = "kuiper_630"

################################################################
# The below constants are taken from Kuiper's FCC filing as below:
# [1]: https://www.itu.int/ITU-R/space/asreceived/Publication/DisplayPublication/8716
################################################################

MEAN_MOTION_REV_PER_DAY = 14.80  # Altitude ~630 km
ALTITUDE_M = 630000  # Altitude ~630 km
SATELLITE_CONE_RADIUS_M = ALTITUDE_M / math.tan(
    math.radians(30.0))  # Considering an elevation angle of 30 degrees; possible values [1]: 20(min)/30/35/45
MAX_GSL_LENGTH_M = math.sqrt(math.pow(SATELLITE_CONE_RADIUS_M, 2) + math.pow(ALTITUDE_M, 2))
MAX_ISL_LENGTH_M = 2 * math.sqrt(math.pow(EARTH_RADIUS + ALTITUDE_M, 2) - math.pow(EARTH_RADIUS + 80000,
                                                                                   2))  # ISLs are not allowed to dip below 80 km altitude in order to avoid weather conditions
NUM_ORBS = 34
NUM_SATS_PER_ORB = 34
INCLINATION_DEGREE = 51.9

# General files needed to generate visualizations; Do not change for different simulations
topFile = "../static_html/top.html"
bottomFile = "../static_html/bottom.html"
city_detail_file = "../../paper/satellite_networks_state/input_data/ground_stations_paris_moscow_grid.basic.txt"

# Time in ms for which visualization will be generated
GEN_TIME = 158300 #116000  # ms

# Input file; Generated during simulation
# City IDs are available in the city_detail_file.
# If city ID is X (for Paris X = 24) and constellation is Starlink_550 (1584 satellites),
# then offset ID is 1584 + 24 = 1608.
path_file = "../../paper/satgenpy_analysis/data/kuiper_630_isls_none_ground_stations_paris_moscow_grid_algorithm_free_one_only_gs_relays/100ms_for_200s/manual/data/networkx_path_1156_to_1232.txt"

# Output directory for creating visualization html files
OUT_DIR = "../viz_output/"
OUT_HTML_FILE = OUT_DIR + NAME + "_path"

sat_objs = []
city_details = {}
paths_over_time = []


def generate_path_at_time():
    """
    Generates end-to-end path at specified time
    :return: HTML formatted string for visualization
    """
    viz_string = ""
    global src_GS
    global dst_GS
    global paths_over_time
    global OUT_HTML_FILE
    lines = [line.rstrip('\n') for line in open(path_file)]
    for i in range(len(lines)):
        val = lines[i].split(",")
        nodes = val[1].split("-")
        paths_over_time.append((int(val[0]), nodes))
    paths_over_time.append((0, nodes))
    SEL_PATH_TIME = 0
    SEL_PATH = []
    for i in range(len(paths_over_time)):
        start_ms = round((paths_over_time[i][0]) / 1000000)
        start_next = 99999999999
        try:
            start_next = round((paths_over_time[i+1][0]) / 1000000)
        except:
            None
        print(start_ms, GEN_TIME)
        if GEN_TIME >= start_ms and GEN_TIME < start_next:
            SEL_PATH_TIME = paths_over_time[i][0]
            SEL_PATH = paths_over_time[i ][1]
            break
    print(SEL_PATH_TIME, SEL_PATH)

    shifted_epoch = (pd.to_datetime(EPOCH) + pd.to_timedelta(GEN_TIME, unit='ms')).strftime(
        format='%Y/%m/%d %H:%M:%S.%f')
    print(shifted_epoch)

    for p in range(len(city_details)):
        viz_string += "var redSphere = viewer.entities.add({name : '', position: Cesium.Cartesian3.fromDegrees(" \
                        + str(city_details[p]["long_deg"]) + ", " \
                        + str(city_details[p]["lat_deg"]) + ", " \
                        + str(city_details[p]["alt_km"] * 1000) + "), " \
                        + "ellipsoid : {radii : new Cesium.Cartesian3(30000.0, 30000.0, 30000.0), " \
                        + "material : Cesium.Color.YELLOW.withAlpha(1),}});\n"

    for i in range(len(sat_objs)):
        sat_objs[i]["sat_obj"].compute(shifted_epoch)
        viz_string += "var redSphere = viewer.entities.add({name : '', position: Cesium.Cartesian3.fromDegrees(" \
                      + str(math.degrees(sat_objs[i]["sat_obj"].sublong)) + ", " \
                      + str(math.degrees(sat_objs[i]["sat_obj"].sublat)) + ", " + str(
            sat_objs[i]["alt_km"] * 1000) + "), " \
                      + "ellipsoid : {radii : new Cesium.Cartesian3(20000.0, 20000.0, 20000.0), " \
                      + "material : Cesium.Color.BLACK.withAlpha(1),}});\n"

    orbit_links = util.find_orbit_links(sat_objs, NUM_ORBS, NUM_SATS_PER_ORB)
    for key in orbit_links:
        sat1 = orbit_links[key]["sat1"]
        sat2 = orbit_links[key]["sat2"]
        viz_string += "viewer.entities.add({name : '', polyline: { positions: Cesium.Cartesian3.fromDegreesArrayHeights([" \
                      + str(math.degrees(sat_objs[sat1]["sat_obj"].sublong)) + "," \
                      + str(math.degrees(sat_objs[sat1]["sat_obj"].sublat)) + "," \
                      + str(sat_objs[sat1]["alt_km"] * 1000) + "," \
                      + str(math.degrees(sat_objs[sat2]["sat_obj"].sublong)) + "," \
                      + str(math.degrees(sat_objs[sat2]["sat_obj"].sublat)) + "," \
                      + str(sat_objs[sat2]["alt_km"] * 1000) + "]), " \
                      + "width: 0.5, arcType: Cesium.ArcType.NONE, " \
                      + "material: new Cesium.PolylineOutlineMaterialProperty({ " \
                      + "color: Cesium.Color.GREY.withAlpha(0.3), outlineWidth: 0, outlineColor: Cesium.Color.BLACK})}});"

    for p in range(len(SEL_PATH)):
        if int(SEL_PATH[p]) >= NUM_ORBS * NUM_SATS_PER_ORB:
            GS = int(SEL_PATH[p]) - NUM_ORBS * NUM_SATS_PER_ORB
            print(GS, city_details[GS]["long_deg"], city_details[GS]["lat_deg"])
            viz_string += "var redSphere = viewer.entities.add({name : '', position: Cesium.Cartesian3.fromDegrees(" \
                          + str(city_details[GS]["long_deg"]) + ", " \
                          + str(city_details[GS]["lat_deg"]) + ", " \
                          + str(city_details[GS]["alt_km"] * 1000) + "), " \
                          + "ellipsoid : {radii : new Cesium.Cartesian3(50000.0, 50000.0, 50000.0), " \
                          + "material : Cesium.Color.GREEN.withAlpha(1),}});\n"

    for p in range(len(SEL_PATH) - 1):
        viz_string += "viewer.entities.add({name : '', polyline: { positions: Cesium.Cartesian3.fromDegreesArrayHeights(["
        if int(SEL_PATH[p]) >= NUM_ORBS * NUM_SATS_PER_ORB:
            GS = int(SEL_PATH[p]) - NUM_ORBS * NUM_SATS_PER_ORB
            viz_string += str(city_details[GS]["long_deg"]) + "," \
                          + str(city_details[GS]["lat_deg"]) + "," \
                          + str(city_details[GS]["alt_km"] * 1000) + ","
        else:
            viz_string += str(math.degrees(sat_objs[int(SEL_PATH[p])]["sat_obj"].sublong)) + "," \
                          + str(math.degrees(sat_objs[int(SEL_PATH[p])]["sat_obj"].sublat)) + "," \
                          + str(sat_objs[int(SEL_PATH[p])]["alt_km"] * 1000) + ","
        if int(SEL_PATH[p + 1]) >= NUM_ORBS * NUM_SATS_PER_ORB:
            GS = int(SEL_PATH[p + 1]) - NUM_ORBS * NUM_SATS_PER_ORB
            viz_string += str(city_details[GS]["long_deg"]) + "," \
                          + str(city_details[GS]["lat_deg"]) + "," \
                          + str(city_details[GS]["alt_km"] * 1000) + "]), "
        else:
            viz_string += str(math.degrees(sat_objs[int(SEL_PATH[p + 1])]["sat_obj"].sublong)) + "," \
                          + str(math.degrees(sat_objs[int(SEL_PATH[p + 1])]["sat_obj"].sublat)) + "," \
                          + str(sat_objs[int(SEL_PATH[p + 1])]["alt_km"] * 1000) + "]), "
        viz_string += "width: 3.0, arcType: Cesium.ArcType.NONE, " \
                      + "material: new Cesium.PolylineOutlineMaterialProperty({ " \
                      + "color: Cesium.Color.RED.withAlpha(1.0), outlineWidth: 0, outlineColor: Cesium.Color.BLACK})}});"
    OUT_HTML_FILE += "_" + str(GEN_TIME) + ".html"
    return viz_string


city_details = util.read_city_details(city_details, city_detail_file)
sat_objs = util.generate_sat_obj_list(
    NUM_ORBS,
    NUM_SATS_PER_ORB,
    EPOCH,
    PHASE_DIFF,
    INCLINATION_DEGREE,
    ECCENTRICITY,
    ARG_OF_PERIGEE_DEGREE,
    MEAN_MOTION_REV_PER_DAY,
    ALTITUDE_M
)
viz_string = generate_path_at_time()
util.write_viz_files(viz_string, topFile, bottomFile, OUT_HTML_FILE)
