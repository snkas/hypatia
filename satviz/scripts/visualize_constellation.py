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

from astropy import units as u
from poliastro.bodies import Earth
from poliastro.twobody import Orbit
from astropy.time import Time
from extractor import CZMLExtractor

# Generate dynamic visualizations for entire constellation (multiple shells).

EARTH_RADIUS = 6378135.0 # WGS72 value; taken from https://geographiclib.sourceforge.io/html/NET/NETGeographicLib_8h_source.html

# CONSTELLATION GENERATION GENERAL CONSTANTS
ECCENTRICITY = 0.0000001  # Circular orbits are zero, but pyephem does not permit 0, so lowest possible value
ARG_OF_PERIGEE_DEGREE = 0.0
PHASE_DIFF = True
EPOCH = "2000-01-01 00:00:00"

# Shell wise color codes
COLOR = [[255, 0, 0, 200], [32, 128, 46, 200], [0, 0, 255, 200], [245, 66, 242, 200], [245, 126, 66, 200]]

# CONSTELLATION SPECIFIC PARAMETERS

# STARLINK
NAME = "Starlink"

SHELL_CNTR = 5

MEAN_MOTION_REV_PER_DAY = [None]*SHELL_CNTR
ALTITUDE_M = [None]*SHELL_CNTR
NUM_ORBS = [None]*SHELL_CNTR
NUM_SATS_PER_ORB = [None]*SHELL_CNTR
INCLINATION_DEGREE = [None]*SHELL_CNTR
BASE_ID = [None]*SHELL_CNTR
ORB_WISE_IDS = [None]*SHELL_CNTR

MEAN_MOTION_REV_PER_DAY[0] = 15.19  # Altitude ~550000 km
ALTITUDE_M[0] = 550000  # Altitude ~550000 km
NUM_ORBS[0] = 72
NUM_SATS_PER_ORB[0] = 22
INCLINATION_DEGREE[0] = 53
BASE_ID[0] = 0
ORB_WISE_IDS[0] = []

MEAN_MOTION_REV_PER_DAY[1] = 13.4  # Altitude ~1110 km
ALTITUDE_M[1] = 1110000  # Altitude ~1110 km
NUM_ORBS[1] = 32
NUM_SATS_PER_ORB[1] = 50
INCLINATION_DEGREE[1] = 53.8
BASE_ID[1] = 1584
ORB_WISE_IDS[1] = []

MEAN_MOTION_REV_PER_DAY[2] = 13.35  # Altitude ~1130 km
ALTITUDE_M[2] = 1130000  # Altitude ~1130 km
NUM_ORBS[2] = 8
NUM_SATS_PER_ORB[2] = 50
INCLINATION_DEGREE[2] = 74
BASE_ID[2] = 3184
ORB_WISE_IDS[2] = []

MEAN_MOTION_REV_PER_DAY[3] = 12.97  # Altitude ~1275 km
ALTITUDE_M[3] = 1275000  # Altitude ~1275 km
NUM_ORBS[3] = 5
NUM_SATS_PER_ORB[3] = 75
INCLINATION_DEGREE[3] = 81
BASE_ID[3] = 3584
ORB_WISE_IDS[3] = []

MEAN_MOTION_REV_PER_DAY[4] = 12.84  # Altitude ~1325 km
ALTITUDE_M[4] = 1325000  # Altitude ~1325 km
NUM_ORBS[4] = 6
NUM_SATS_PER_ORB[4] = 75
INCLINATION_DEGREE[4] = 70
BASE_ID[4] = 3959
ORB_WISE_IDS[4] = []

"""
# TELESAT
NAME = "Telesat"
SHELL_CNTR = 2

MEAN_MOTION_REV_PER_DAY = [None]*SHELL_CNTR
ALTITUDE_M = [None]*SHELL_CNTR
NUM_ORBS = [None]*SHELL_CNTR
NUM_SATS_PER_ORB = [None]*SHELL_CNTR
INCLINATION_DEGREE = [None]*SHELL_CNTR
BASE_ID = [None]*SHELL_CNTR
ORB_WISE_IDS = [None]*SHELL_CNTR

MEAN_MOTION_REV_PER_DAY[0] = 13.66  # Altitude ~1015 km
ALTITUDE_M[0] = 1015000  # Altitude ~1015 km
NUM_ORBS[0] = 27
NUM_SATS_PER_ORB[0] = 13
INCLINATION_DEGREE[0] = 98.98
BASE_ID[0] = 0
ORB_WISE_IDS[0] = []

MEAN_MOTION_REV_PER_DAY[1] = 12.84  # Altitude ~1325 km
ALTITUDE_M[1] = 1325000  # Altitude ~1325 km
NUM_ORBS[1] = 40
NUM_SATS_PER_ORB[1] = 33
INCLINATION_DEGREE[1] = 50.88
BASE_ID[1] = 351
ORB_WISE_IDS[1] = []
"""

"""
# KUIPER
NAME = "kuiper"
################################################################
# The below constants are taken from Kuiper's FCC filing as below:
# [1]: https://www.itu.int/ITU-R/space/asreceived/Publication/DisplayPublication/8716
################################################################

SHELL_CNTR = 3

MEAN_MOTION_REV_PER_DAY = [None]*SHELL_CNTR
ALTITUDE_M = [None]*SHELL_CNTR
NUM_ORBS = [None]*SHELL_CNTR
NUM_SATS_PER_ORB = [None]*SHELL_CNTR
INCLINATION_DEGREE = [None]*SHELL_CNTR
BASE_ID = [None]*SHELL_CNTR
ORB_WISE_IDS = [None]*SHELL_CNTR

MEAN_MOTION_REV_PER_DAY[0] = 14.80  # Altitude ~630 km
ALTITUDE_M[0] = 630000  # Altitude ~630 km
NUM_ORBS[0] = 34
NUM_SATS_PER_ORB[0] = 34
INCLINATION_DEGREE[0] = 51.9
BASE_ID[0] = 0
ORB_WISE_IDS[0] = []

MEAN_MOTION_REV_PER_DAY[1] = 14.86  # Altitude ~610 km
ALTITUDE_M[1] = 610000  # Altitude ~610 km
NUM_ORBS[1] = 36
NUM_SATS_PER_ORB[1] = 36
INCLINATION_DEGREE[1] = 42
BASE_ID[1] = 1156
ORB_WISE_IDS[1] = []

MEAN_MOTION_REV_PER_DAY[2] = 14.93  # Altitude ~590 km
ALTITUDE_M[2] = 590000  # Altitude ~590 km
NUM_ORBS[2] = 28
NUM_SATS_PER_ORB[2] = 28
INCLINATION_DEGREE[2] = 33
BASE_ID[2] = 2452
ORB_WISE_IDS[2] = []
"""


# General files needed to generate visualizations; Do not change for different simulations
topFile = "../static_html/top.html"
bottomFile = "../static_html/bottom.html"

# Output directory for creating visualization html files
OUT_DIR = "../viz_output/"
JSON_NAME  = NAME+"_5shell.json"
OUT_JSON_FILE = OUT_DIR + JSON_NAME
OUT_HTML_FILE = OUT_DIR + NAME + "_5shell.html"

START = Time(EPOCH, scale="tdb")
END = START + (10*60) * u.second
sample_points = 10
extractor = CZMLExtractor(START, END, sample_points)


def generate_satellite_trajectories():
    """
    Generates and adds satellite orbits to extractor.
    :return: None
    """

    for i in range(0, SHELL_CNTR):

        SEMI_MAJOR_AXIS = ((EARTH_RADIUS + ALTITUDE_M[i]) / 1000) * u.km
        for orbit in range(0, NUM_ORBS[i]):
            orbit_sat_ids = []
            # Orbit-dependent
            raan_degree = (orbit * 360.0 / NUM_ORBS[i]) * u.deg
            orbit_wise_shift = 0
            if orbit % 2 == 1:
                if PHASE_DIFF:
                    orbit_wise_shift = 360.0 / (NUM_SATS_PER_ORB[i] * 2.0)

            # For each satellite in the orbit
            for n_sat in range(0, NUM_SATS_PER_ORB[i]):
                nu = (orbit_wise_shift + (n_sat * 360 / NUM_SATS_PER_ORB[i])) * u.deg
                ss = Orbit.from_classical(Earth,
                                          SEMI_MAJOR_AXIS,
                                          ECCENTRICITY * u.one,
                                          INCLINATION_DEGREE[i] * u.deg,
                                          raan_degree,
                                          ARG_OF_PERIGEE_DEGREE * u.deg,
                                          nu,
                                          epoch=START)
                extractor.add_orbit(ss, label_text="",
                                    path_show=False)  # path_color=[255, 255, 255, 10])  # path_show=False
                orbit_sat_ids.append(orbit * NUM_SATS_PER_ORB[i] + n_sat + BASE_ID[i])
                print("done for ", orbit, n_sat)
            ORB_WISE_IDS[i].append(orbit_sat_ids)

        for orb in ORB_WISE_IDS[i]:
            for k in range(len(orb)):
                if k < len(orb) - 1:
                    extractor.add_link(orb[k], orb[k+1], START, END, color=COLOR[i], width=i*2.75+0.5)
                else:
                    extractor.add_link(orb[k], orb[0], START, END, color=COLOR[i], width=i*2.75+0.5)


def write_viz_files():
    """
    Writes JSON and TML files to the output folder
    :return: None
    """
    writer_json = open(OUT_JSON_FILE, 'w')
    writer_html = open(OUT_HTML_FILE, 'w')
    writer_json.write(str(extractor.packets) + "\n")
    writer_json.close()
    with open(topFile, 'r') as fi:
        writer_html.write(fi.read())
    writer_html.write("\nviewer.dataSources.add(Cesium.CzmlDataSource.load('"+JSON_NAME+"'));\n")
    with open(bottomFile, 'r') as fb:
        writer_html.write(fb.read())
    writer_html.close()


generate_satellite_trajectories()
write_viz_files()