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

import ephem
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from dms2dec.dms_convert import dms2dec

# For a static observer, finds satellite positions (azimuth, altitude) over time
# Plots visible satellites over time


# CONSTELLATION GENERATION GENERAL CONSTANTS
EPOCH = "2020/01/01 00:00:01"
ECCENTRICITY = 0.001  # Circular orbits
ARG_OF_PERIGEE = 0.0
EARTH_RADIUS = 6378.135  # km
PHASE_DIFF = 'Y'

# Shell wise color codes
COLOR = [(1, 0, 0), (32/255, 128/255, 46/255), (0, 0, 1), (245/255, 66/255, 242/255), (245/255, 126/255, 66/255)]


# CONSTELLATION SPECIFIC PARAMETERS
# STARLINK

MIN_DEG_ELEVATION = 25.0
NUM_SHELLS = 5

MEAN_MOTION_REV_PER_DAY = [None]*NUM_SHELLS
NUM_ORBS = [None]*NUM_SHELLS
NUM_SATS_PER_ORB = [None]*NUM_SHELLS
INCLINATION_DEGREE = [None]*NUM_SHELLS

MEAN_MOTION_REV_PER_DAY[0] = 15.19  # Altitude ~550000 km
NUM_ORBS[0] = 72
NUM_SATS_PER_ORB[0] = 22
INCLINATION_DEGREE[0] = 53

MEAN_MOTION_REV_PER_DAY[1] = 13.4  # Altitude ~1110 km
NUM_ORBS[1] = 32
NUM_SATS_PER_ORB[1] = 50
INCLINATION_DEGREE[1] = 53.8

MEAN_MOTION_REV_PER_DAY[2] = 13.35  # Altitude ~1130 km
NUM_ORBS[2] = 8
NUM_SATS_PER_ORB[2] = 50
INCLINATION_DEGREE[2] = 74

MEAN_MOTION_REV_PER_DAY[3] = 12.97  # Altitude ~1275 km
NUM_ORBS[3] = 5
NUM_SATS_PER_ORB[3] = 75
INCLINATION_DEGREE[3] = 81

MEAN_MOTION_REV_PER_DAY[4] = 12.84  # Altitude ~1325 km
NUM_ORBS[4] = 6
NUM_SATS_PER_ORB[4] = 75
INCLINATION_DEGREE[4] = 70

"""
# Telesat

MIN_DEG_ELEVATION = 10.0
NUM_SHELLS = 2

MEAN_MOTION_REV_PER_DAY = [None]*NUM_SHELLS
NUM_ORBS = [None]*NUM_SHELLS
NUM_SATS_PER_ORB = [None]*NUM_SHELLS
INCLINATION_DEGREE = [None]*NUM_SHELLS

MEAN_MOTION_REV_PER_DAY[0] = 13.66  # Altitude ~1015 km
NUM_ORBS[0] = 27
NUM_SATS_PER_ORB[0] = 13
INCLINATION_DEGREE[0] = 98.98


MEAN_MOTION_REV_PER_DAY[1] = 12.84  # Altitude ~1325 km
NUM_ORBS[1] = 40
NUM_SATS_PER_ORB[1] = 33
INCLINATION_DEGREE[1] = 50.88
"""

"""
# KUIPER

MIN_DEG_ELEVATION = 30.0
NUM_SHELLS = 3

MEAN_MOTION_REV_PER_DAY = [None]*NUM_SHELLS
NUM_ORBS = [None]*NUM_SHELLS
NUM_SATS_PER_ORB = [None]*NUM_SHELLS
INCLINATION_DEGREE = [None]*NUM_SHELLS

MEAN_MOTION_REV_PER_DAY[0] = 14.80  # Altitude ~630 km
NUM_ORBS[0] = 34
NUM_SATS_PER_ORB[0] = 34
INCLINATION_DEGREE[0] = 51.9

MEAN_MOTION_REV_PER_DAY[1] = 14.86  # Altitude ~610 km
NUM_ORBS[1] = 36
NUM_SATS_PER_ORB[1] = 36
INCLINATION_DEGREE[1] = 42

MEAN_MOTION_REV_PER_DAY[2] = 14.93  # Altitude ~590 km
NUM_ORBS[2] = 28
NUM_SATS_PER_ORB[2] = 28
INCLINATION_DEGREE[2] = 33
"""

sat_objs = []


def decimalDegrees2DMS(value):
    """
    Changes decimal coordinates to degree-minute-second coordinates
    :param value: decimal coordinate
    :return: degree-minute-second coordinate
    """
    degrees = int(value)
    submin = abs((value - int(value)) * 60)
    minutes = int(submin)
    subseconds = abs((submin - int(submin)) * 60)
    dms_val = str(degrees) + ":" + str(minutes) + ":" + str(subseconds)[0:5]
    return dms_val


def find_horizon(sat_objs, sec, location):
    """
    Get satellite altitude and azimuth list for specific observer and time
    :param sat_objs: List of satellite objects
    :param sec: Time in second
    :param location: Observer location
    :return: Azimuth list, altitude list, shell list (different shells have different colors)
    """
    lat_dms = decimalDegrees2DMS(location[0])
    long_dms = decimalDegrees2DMS(location[1])
    observer = ephem.Observer()
    observer.lon, observer.lat = long_dms, lat_dms
    shifted_epoch = (pd.to_datetime(EPOCH) + pd.to_timedelta(sec, unit='s')).strftime(format='%Y/%m/%d %H:%M:%S')
    observer.date = shifted_epoch
    alt_list = []
    azim_list = []
    shell_list = []
    for shell_cntr in range(0, NUM_SHELLS):
        print(len(sat_objs[shell_cntr]))
        for id in range(len(sat_objs[shell_cntr])):
            sat = sat_objs[shell_cntr][id]
            sat.compute(observer)
            angle_alt = dms2dec(str(sat.alt))
            if sat.alt < 0:
                angle_alt = 0 - angle_alt
            angle_az = dms2dec(str(sat.az))
            if sat.az < 0:
                angle_az = 0 - angle_az

            print("%d %f %f" % (id, angle_alt, angle_az))
            #if angle_alt > MIN_DEG_ELEVATION:
            if angle_alt > 0.0:
                #orb_id = math.floor(id/NUM_ORBS)
                print("%d %f %f" % (id, angle_alt, angle_az))
                alt_list.append(angle_alt)
                azim_list.append(angle_az)
                shell_list.append(shell_cntr)
    X = np.array(azim_list)
    Y = np.array(alt_list)
    S = np.array(shell_list)
    return X, Y, S


def generate_sat_obj_list():
    """
    Generates list of satellite objects based on orbital elements
    :return: List of satellites
    """
    global sat_objs
    sat_objs = [None] * NUM_SHELLS
    for shell_cntr in range(0, NUM_SHELLS):
        sat_objs[shell_cntr] = [None]*(NUM_ORBS[shell_cntr]* NUM_SATS_PER_ORB[shell_cntr])
        counter = 0
        for orb in range(0, NUM_ORBS[shell_cntr]):
            raan = orb * 360 / NUM_ORBS[shell_cntr]
            orbit_wise_shift = 0
            if orb % 2 == 1:
                if PHASE_DIFF == 'Y':
                    orbit_wise_shift = 360 / (NUM_SATS_PER_ORB[shell_cntr] * 2)

            for n_sat in range(0, NUM_SATS_PER_ORB[shell_cntr]):
                mean_anomaly = orbit_wise_shift + (n_sat * 360 / NUM_SATS_PER_ORB[shell_cntr])

                sat = ephem.EarthSatellite()
                sat._epoch = EPOCH
                sat._inc = ephem.degrees(INCLINATION_DEGREE[shell_cntr])
                sat._e = ECCENTRICITY
                sat._raan = ephem.degrees(raan)
                sat._ap = ARG_OF_PERIGEE
                sat._M = ephem.degrees(mean_anomaly)
                sat._n = MEAN_MOTION_REV_PER_DAY[shell_cntr]
                sat_objs[shell_cntr][counter] = sat
                counter += 1
    return sat_objs


# LOCATION = (28.7041, 77.1025)  # Delhi
# LOCATION = (59.9139, 10.7522)  # Oslo
# LOCATION = (-33.4489, -70.6693)  # Santiago
# LOCATION = (-47.0478056, 149.1647065)  # Christchurch

LOCATION = (59.9311, 30.3609)  # Saint Petersburg

VIZ_TIME = 170  # Total time of visualization in seconds
VIZ_GRAN = 5  # Granularity of visualization in seconds

generate_sat_obj_list()
plt.ion()
cntr=1
# for sec in range(3000, 3000 + 60*60, 10):
for sec in range(0, VIZ_TIME, VIZ_GRAN):
    X, Y, S = find_horizon(sat_objs, sec, LOCATION)
    # print(X, Y)
    plt.clf()
    plt.xlabel("Observed azimuth (degrees)")
    plt.ylabel("Elevation (degrees)")
    plt.ylim((0, 90))
    plt.xlim((0, 360))
    #plt.scatter(X, Y)
    for i, j in enumerate(X):
        # look for the color based on shell number
        color = COLOR[S[i] % len(COLOR)]
        print(sec, X[i], Y[i], S[i], color)
        plt.scatter(X[i], Y[i], color=color)
    plt.axhspan(0.0, MIN_DEG_ELEVATION, facecolor='b', alpha=0.5)

    ax = plt.gca()
    #sets the ratio to 5
    ax.set_aspect(3)
    plt.show()
    plt.pause(0.1)
    #plt.savefig('OUTPUT_FOLDER/horizon_'+str(cntr)+'.png')
    cntr += 1

# If images are saved in an output folder (use plt.savefig above), run the below command (ffmpeg) to generate video.
# ffmpeg -r 5 -i horizon_%d.png -c:v libx264 -vf "format=yuv420p" outputMovie.mp4

