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


import unittest

import ephem
import math
from astropy.time import Time
from astropy import units as u
import exputil

from satgen.distance_tools import *
from satgen.ground_stations import *


class TestDistanceTools(unittest.TestCase):

    def test_distance_between_satellites(self):
        kuiper_satellite_0 = ephem.readtle(
            "Kuiper-630 0",
            "1 00001U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    04",
            "2 00001  51.9000   0.0000 0000001   0.0000   0.0000 14.80000000    02"
        )

        kuiper_satellite_1 = ephem.readtle(
            "Kuiper-630 1",
            "1 00002U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    05",
            "2 00002  51.9000   0.0000 0000001   0.0000  10.5882 14.80000000    07"
        )

        kuiper_satellite_17 = ephem.readtle(
            "Kuiper-630 17",
            "1 00018U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    02",
            "2 00018  51.9000   0.0000 0000001   0.0000 180.0000 14.80000000    09"
        )

        kuiper_satellite_18 = ephem.readtle(
            "Kuiper-630 18",
            "1 00019U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    03",
            "2 00019  51.9000   0.0000 0000001   0.0000 190.5882 14.80000000    04"
        )

        kuiper_satellite_19 = ephem.readtle(
            "Kuiper-630 19",
            "1 00020U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    05",
            "2 00020  51.9000   0.0000 0000001   0.0000 201.1765 14.80000000    05"
        )

        for extra_time_ns in [
            0,  # 0
            1,  # 1ns
            1000,  # 1 microsecond
            1000000,  # 1ms
            1000000000,  # 1s
            60000000000,  # 60s
            10 * 60000000000,  # 10 minutes
            20 * 60000000000,  # 20 minutes
            30 * 60000000000,  # 30 minutes
            40 * 60000000000,  # 40 minutes
            50 * 60000000000,  # 50 minutes
            60 * 60000000000,  # 60 minutes
            70 * 60000000000,  # 70 minutes
            80 * 60000000000,  # 80 minutes
            90 * 60000000000,  # 90 minutes
            100 * 60000000000,  # 100 minutes
        ]:
            epoch = Time("2000-01-01 00:00:00", scale="tdb")
            time = epoch + extra_time_ns * u.ns

            # Distance to themselves should always be zero
            self.assertEqual(
                distance_m_between_satellites(kuiper_satellite_0, kuiper_satellite_0, str(epoch), str(time)),
                0
            )
            self.assertEqual(
                distance_m_between_satellites(kuiper_satellite_1, kuiper_satellite_1, str(epoch), str(time)),
                0
            )
            self.assertEqual(
                distance_m_between_satellites(kuiper_satellite_17, kuiper_satellite_17, str(epoch), str(time)),
                0
            )
            self.assertEqual(
                distance_m_between_satellites(kuiper_satellite_18, kuiper_satellite_18, str(epoch), str(time)),
                0
            )

            # Distances should not matter if (a, b) or (b, a)
            self.assertEqual(
                distance_m_between_satellites(kuiper_satellite_0, kuiper_satellite_1, str(epoch), str(time)),
                distance_m_between_satellites(kuiper_satellite_1, kuiper_satellite_0, str(epoch), str(time)),
            )
            self.assertEqual(
                distance_m_between_satellites(kuiper_satellite_1, kuiper_satellite_17, str(epoch), str(time)),
                distance_m_between_satellites(kuiper_satellite_17, kuiper_satellite_1, str(epoch), str(time)),
            )
            self.assertEqual(
                distance_m_between_satellites(kuiper_satellite_19, kuiper_satellite_17, str(epoch), str(time)),
                distance_m_between_satellites(kuiper_satellite_17, kuiper_satellite_19, str(epoch), str(time)),
            )

            # Distance between 0 and 1 should be less than between 0 and 18 (must be on other side of planet)
            self.assertGreater(
                distance_m_between_satellites(kuiper_satellite_0, kuiper_satellite_18, str(epoch), str(time)),
                distance_m_between_satellites(kuiper_satellite_0, kuiper_satellite_1, str(epoch), str(time)),
            )

            # Triangle inequality
            self.assertGreater(
                distance_m_between_satellites(kuiper_satellite_17, kuiper_satellite_18, str(epoch), str(time))
                +
                distance_m_between_satellites(kuiper_satellite_18, kuiper_satellite_19, str(epoch), str(time)),
                distance_m_between_satellites(kuiper_satellite_17, kuiper_satellite_19, str(epoch), str(time))
            )

            # Earth radius = 6378135 m
            # Kuiper altitude = 630 km
            # So, the circle is 630000 + 6378135 = 7008135 m in radius
            # As such, with 34 satellites, the side of this 34-polygon is:
            polygon_side_m = 2 * (7008135.0 * math.sin(math.radians(360.0 / 33.0) / 2.0))
            self.assertTrue(
                polygon_side_m >= distance_m_between_satellites(
                    kuiper_satellite_17,
                    kuiper_satellite_18,
                    str(epoch),
                    str(time)
                ) >= 0.9 * polygon_side_m
            )
            self.assertTrue(
                polygon_side_m >= distance_m_between_satellites(
                    kuiper_satellite_18,
                    kuiper_satellite_19,
                    str(epoch),
                    str(time)
                ) >= 0.9 * polygon_side_m
            )
            self.assertTrue(
                polygon_side_m >= distance_m_between_satellites(
                    kuiper_satellite_0,
                    kuiper_satellite_1,
                    str(epoch),
                    str(time)
                ) >= 0.9 * polygon_side_m
            )

    def test_distance_between_ground_stations(self):
        local_shell = exputil.LocalShell()

        # Create some ground stations
        with open("ground_stations.temp.txt", "w+") as f_out:
            f_out.write("0,Amsterdam,52.379189,4.899431,0\n")
            f_out.write("1,Paris,48.864716,2.349014,0\n")
            f_out.write("2,Rio de Janeiro,-22.970722,-43.182365,0\n")
            f_out.write("3,Manila,14.599512,120.984222,0\n")
            f_out.write("4,Perth,-31.953512,115.857048,0\n")
            f_out.write("5,Some place on Antarctica,-72.927148,33.450844,0\n")
            f_out.write("6,New York,40.730610,-73.935242,0\n")
            f_out.write("7,Some place in Greenland,79.741382,-53.143087,0")
        ground_stations = read_ground_stations_basic("ground_stations.temp.txt")

        # Distance to itself is always 0
        for i in range(8):
            self.assertEqual(
                geodesic_distance_m_between_ground_stations(ground_stations[i], ground_stations[i]),
                0
            )
            self.assertEqual(
                straight_distance_m_between_ground_stations(ground_stations[i], ground_stations[i]),
                0
            )

        # Direction does not matter
        for i in range(8):
            for j in range(8):
                self.assertAlmostEqual(
                    geodesic_distance_m_between_ground_stations(ground_stations[i], ground_stations[j]),
                    geodesic_distance_m_between_ground_stations(ground_stations[j], ground_stations[i]),
                    delta=0.00001
                )
                self.assertAlmostEqual(
                    straight_distance_m_between_ground_stations(ground_stations[i], ground_stations[j]),
                    straight_distance_m_between_ground_stations(ground_stations[j], ground_stations[i]),
                    delta=0.00001
                )

                # Geodesic is always strictly greater than straight
                if i != j:
                    self.assertGreater(
                        geodesic_distance_m_between_ground_stations(ground_stations[i], ground_stations[j]),
                        straight_distance_m_between_ground_stations(ground_stations[i], ground_stations[j])
                    )

        # Amsterdam to Paris
        self.assertAlmostEqual(
            geodesic_distance_m_between_ground_stations(ground_stations[0], ground_stations[1]),
            430000,  # 430 km
            delta=1000.0
        )

        # Amsterdam to New York
        self.assertAlmostEqual(
            geodesic_distance_m_between_ground_stations(ground_stations[0], ground_stations[6]),
            5861000,  # 5861 km
            delta=5000.0
        )

        # New York to Antarctica
        self.assertAlmostEqual(
            geodesic_distance_m_between_ground_stations(ground_stations[6], ground_stations[5]),
            14861000,  # 14861 km
            delta=20000.0
        )

        # Clean up
        local_shell.remove("ground_stations.temp.txt")

    def test_distance_ground_station_to_satellite(self):

        epoch = Time("2000-01-01 00:00:00", scale="tdb")
        time = epoch + 100 * 1000 * 1000 * 1000 * u.ns

        # Two satellites
        telesat_18 = ephem.readtle(
            "Telesat-1015 18",
            "1 00019U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    03",
            "2 00019  98.9800  13.3333 0000001   0.0000 152.3077 13.66000000    04"
        )
        telesat_19 = ephem.readtle(
            "Telesat-1015 19",
            "1 00020U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    05",
            "2 00020  98.9800  13.3333 0000001   0.0000 180.0000 13.66000000    00"
        )

        # Their shadows
        shadow_18 = create_basic_ground_station_for_satellite_shadow(telesat_18, str(epoch), str(time))
        shadow_19 = create_basic_ground_station_for_satellite_shadow(telesat_19, str(epoch), str(time))

        # Distance to shadow should be around 1015km
        self.assertAlmostEqual(
            distance_m_ground_station_to_satellite(shadow_18, telesat_18, str(epoch), str(time)),
            1015000,  # 1015km
            delta=5000  # Accurate within 5km
        )
        distance_shadow_19_to_satellite_19 = distance_m_ground_station_to_satellite(
            shadow_19, telesat_19, str(epoch), str(time)
        )
        self.assertAlmostEqual(
            distance_shadow_19_to_satellite_19,
            1015000,  # 1015km
            delta=5000  # Accurate within 5km
        )

        # Distance between the two shadows:
        # 21.61890110054602, 96.54190305000301
        # -5.732296878862085, 92.0396062736707
        shadow_distance_m = geodesic_distance_m_between_ground_stations(shadow_18, shadow_19)
        self.assertAlmostEqual(
            shadow_distance_m,
            3080640,  # 3080.64 km, from Google Maps
            delta=5000  # With an accuracy of 5km
        )

        # The Pythagoras distance must be within 10% assuming the geodesic does not cause to much of an increase
        distance_shadow_18_to_satellite_19 = distance_m_ground_station_to_satellite(
            shadow_18, telesat_19, str(epoch), str(time)
        )
        self.assertAlmostEqual(
            math.sqrt(shadow_distance_m ** 2 + distance_shadow_19_to_satellite_19 ** 2),
            distance_shadow_18_to_satellite_19,
            delta=0.1 * math.sqrt(shadow_distance_m ** 2 + distance_shadow_19_to_satellite_19 ** 2)
        )

        # Check that the hypotenuse is not exceeded
        straight_shadow_distance_m = straight_distance_m_between_ground_stations(shadow_18, shadow_19)
        self.assertGreater(
            distance_shadow_18_to_satellite_19,
            math.sqrt(
                straight_shadow_distance_m ** 2
                +
                distance_shadow_19_to_satellite_19 ** 2
            ),
        )

        # Check what happens with cartesian coordinates
        a = geodetic2cartesian(
            float(shadow_18["latitude_degrees_str"]),
            float(shadow_18["longitude_degrees_str"]),
            shadow_18["elevation_m_float"],
        )
        b = geodetic2cartesian(
            float(shadow_19["latitude_degrees_str"]),
            float(shadow_19["longitude_degrees_str"]),
            shadow_19["elevation_m_float"],
        )

        # For now, we will keep a loose bound of 1% here, but it needs to be tightened
        # It mostly has to do with that the great circle does not account for the ellipsoid effect
        self.assertAlmostEqual(
            math.sqrt((a[0] - b[0]) ** 2 + (a[1] - b[1]) ** 2 + (a[2] - b[2]) ** 2),
            straight_shadow_distance_m,
            delta=20000  # 20km
        )
