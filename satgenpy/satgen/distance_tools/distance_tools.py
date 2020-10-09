# The MIT License (MIT)
#
# Copyright (c) 2020 ETH Zurich
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
from geopy.distance import great_circle


def distance_m_between_satellites(sat1, sat2, epoch_str, date_str):
    """
    Computes the straight distance between two satellites in meters.

    :param sat1:       The first satellite
    :param sat2:       The other satellite
    :param epoch_str:  Epoch time of the observer (string)
    :param date_str:   The time instant when the distance should be measured (string)

    :return: The distance between the satellites in meters
    """

    # Create an observer somewhere on the planet
    observer = ephem.Observer()
    observer.epoch = epoch_str
    observer.date = date_str
    observer.lat = 0
    observer.lon = 0
    observer.elevation = 0

    # Calculate the relative location of the satellites to this observer
    sat1.compute(observer)
    sat2.compute(observer)

    # Calculate the angle observed by the observer to the satellites (this is done because the .compute() calls earlier)
    angle_radians = float(repr(ephem.separation(sat1, sat2)))

    # Now we have a triangle with three knows:
    # (1) a = sat1.range (distance observer to satellite 1)
    # (2) b = sat2.range (distance observer to satellite 2)
    # (3) C = angle (the angle at the observer point within the triangle)
    #
    # Using the formula:
    # c^2 = a^2 + b^2 - 2 * a * b * cos(C)
    #
    # This gives us side c, the distance between the two satellites
    return math.sqrt(sat1.range ** 2 + sat2.range ** 2 - (2 * sat1.range * sat2.range * math.cos(angle_radians)))


def distance_m_ground_station_to_satellite(ground_station, satellite, epoch_str, date_str):
    """
    Computes the straight distance between a ground station and a satellite in meters

    :param ground_station:  The ground station
    :param satellite:       The satellite
    :param epoch_str:       Epoch time of the observer (ground station) (string)
    :param date_str:        The time instant when the distance should be measured (string)

    :return: The distance between the ground station and the satellite in meters
    """

    # Create an observer on the planet where the ground station is
    observer = ephem.Observer()
    observer.epoch = epoch_str
    observer.date = date_str
    observer.lat = str(ground_station["latitude_degrees_str"])   # Very important: string argument is in degrees.
    observer.lon = str(ground_station["longitude_degrees_str"])  # DO NOT pass a float as it is interpreted as radians
    observer.elevation = ground_station["elevation_m_float"]

    # Compute distance from satellite to observer
    satellite.compute(observer)

    # Return distance
    return satellite.range


def geodesic_distance_m_between_ground_stations(ground_station_1, ground_station_2):
    """
    Calculate the geodesic distance between two ground stations.

    :param ground_station_1:         First ground station
    :param ground_station_2:         Another ground station

    :return: Geodesic distance in meters
    """

    # WGS72 value; taken from https://geographiclib.sourceforge.io/html/NET/NETGeographicLib_8h_source.html
    earth_radius_km = 6378.135  # 6378135.0 meters

    return great_circle(
        (float(ground_station_1["latitude_degrees_str"]), float(ground_station_1["longitude_degrees_str"])),
        (float(ground_station_2["latitude_degrees_str"]), float(ground_station_2["longitude_degrees_str"])),
        radius=earth_radius_km
    ).m


def straight_distance_m_between_ground_stations(ground_station_1, ground_station_2):
    """
    Calculate the straight distance between two ground stations (goes through the Earth)

    :param ground_station_1:         First ground station
    :param ground_station_2:         Another ground station

    :return: Straight distance in meters (goes through the Earth)
    """

    # WGS72 value; taken from https://geographiclib.sourceforge.io/html/NET/NETGeographicLib_8h_source.html
    earth_radius_m = 6378135.0

    # First get the angle between the two ground stations from the Earth's core
    fraction_of_earth_circumference = geodesic_distance_m_between_ground_stations(
        ground_station_1,
        ground_station_2
    ) / (earth_radius_m * 2.0 * math.pi)
    angle_radians = fraction_of_earth_circumference * 2 * math.pi

    # Now see the Earth as a circle you know the hypotenuse, and half the angle is that of the triangle
    # with the 90 degree corner. Multiply by two to get the straight distance.
    polygon_side_m = 2 * math.sin(angle_radians / 2.0) * earth_radius_m

    return polygon_side_m


def create_basic_ground_station_for_satellite_shadow(satellite, epoch_str, date_str):
    """
    Calculate the (latitude, longitude) of the satellite shadow on the Earth and creates a ground station there.

    :param satellite:   Satellite
    :param epoch_str:   Epoch (string)
    :param date_str:    Time moment (string)

    :return: Basic ground station
    """

    satellite.compute(date_str, epoch=epoch_str)

    return {
        "gid": -1,
        "name": "Shadow of " + satellite.name,
        "latitude_degrees_str": str(math.degrees(satellite.sublat)),
        "longitude_degrees_str": str(math.degrees(satellite.sublong)),
        "elevation_m_float": 0,
    }


def geodetic2cartesian(lat_degrees, lon_degrees, ele_m):
    """
    Compute geodetic coordinates (latitude, longitude, elevation) to Cartesian coordinates.

    :param lat_degrees: Latitude in degrees (float)
    :param lon_degrees: Longitude in degrees (float)
    :param ele_m:  Elevation in meters

    :return: Cartesian coordinate as 3-tuple of (x, y, z)
    """

    #
    # Adapted from: https://github.com/andykee/pygeodesy/blob/master/pygeodesy/transform.py
    #

    # WGS72 value,
    # Source: https://geographiclib.sourceforge.io/html/NET/NETGeographicLib_8h_source.html
    a = 6378135.0

    # Ellipsoid flattening factor; WGS72 value
    # Taken from https://geographiclib.sourceforge.io/html/NET/NETGeographicLib_8h_source.html
    f = 1.0 / 298.26

    # First numerical eccentricity of ellipsoid
    e = math.sqrt(2.0 * f - f * f)
    lat = lat_degrees * (math.pi / 180.0)
    lon = lon_degrees * (math.pi / 180.0)

    # Radius of curvature in the prime vertical of the surface of the geodetic ellipsoid
    v = a / math.sqrt(1.0 - e * e * math.sin(lat) * math.sin(lat))

    x = (v + ele_m) * math.cos(lat) * math.cos(lon)
    y = (v + ele_m) * math.cos(lat) * math.sin(lon)
    z = (v * (1.0 - e * e) + ele_m) * math.sin(lat)

    return x, y, z
