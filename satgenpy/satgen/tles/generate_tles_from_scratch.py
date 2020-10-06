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
from sgp4.exporter import export_tle
from sgp4.api import Satrec, WGS72
from sgp4.api import jday


def generate_tles_from_scratch_with_sgp(
        filename_out,
        constellation_name,
        num_orbits,
        num_sats_per_orbit,
        phase_diff,
        inclination_degree,
        eccentricity,
        arg_of_perigee_degree,
        mean_motion_rev_per_day
):

    with open(filename_out, "w+") as f_out:

        # First line:
        #
        # <number of orbits> <number of satellites per orbit>
        #
        f_out.write("%d %d\n" % (num_orbits, num_sats_per_orbit))

        # Each of the subsequent (number of orbits * number of satellites per orbit) blocks
        # define a satellite as follows:
        #
        # <constellation_name> <global satellite id>
        # <TLE line 1>
        # <TLE line 2>
        satellite_counter = 0
        for orbit in range(0, num_orbits):

            # Orbit-dependent
            raan_degree = orbit * 360.0 / num_orbits
            orbit_wise_shift = 0
            if orbit % 2 == 1:
                if phase_diff:
                    orbit_wise_shift = 360.0 / (num_sats_per_orbit * 2.0)

            # For each satellite in the orbit
            for n_sat in range(0, num_sats_per_orbit):
                mean_anomaly_degree = orbit_wise_shift + (n_sat * 360 / num_sats_per_orbit)

                # Epoch is set to the year 2000
                # This conveniently in TLE format gives 00001.00000000
                # for the epoch year and Julian day fraction entry
                jd, fr = jday(2000, 1, 1, 0, 0, 0)

                # Use SGP-4 to generate TLE
                sat_sgp4 = Satrec()

                # Based on: https://pypi.org/project/sgp4/
                sat_sgp4.sgp4init(
                    WGS72,                  # Gravity model [1]
                    'i',                    # Operating mode (a = old AFPSC mode, i = improved mode)
                    satellite_counter + 1,  # satnum:  satellite number
                    (jd + fr) - 2433281.5,  # epoch:   days since 1949 December 31 00:00 UT [2]
                    0.0,                    # bstar:   drag coefficient (kg/m2er)
                    0.0,                    # ndot:    ballistic coefficient (revs/day)
                    0.0,                    # nndot:   second derivative of mean motion (revs/day^3)
                    eccentricity,           # ecco:    eccentricity
                    math.radians(arg_of_perigee_degree),              # argpo:   argument or perigee (radians)
                    math.radians(inclination_degree),                 # inclo:    inclination(radians)
                    math.radians(mean_anomaly_degree),                # mo:       mean anomaly (radians)
                    mean_motion_rev_per_day * 60 / 13750.9870831397,  # no_kazai: mean motion (radians/minute) [3]
                    math.radians(raan_degree)                         # nodeo:    right ascension of
                                                                      #           ascending node (radians)
                )

                # Side notes:
                # [1] WGS72 is also used in the NS-3 model
                # [2] Due to a bug in sgp4init, the TLE below irrespective of the value here gives zeros.
                # [3] Conversion factor from:
                #     https://www.translatorscafe.com/unit-converter/en-US/ (continue on next line)
                #     velocity-angular/1-9/radian/second-revolution/day/
                #

                # Export TLE from the SGP-4 object
                line1, line2 = export_tle(sat_sgp4)

                # Line 1 has some problems: there are unknown characters entered for the international
                # designator, and the Julian date is not respected
                # As such, we set our own bogus international designator 00000ABC
                # and we set our own epoch date as 1 January, 2000
                # Why it's 00001.00000000: https://www.celestrak.com/columns/v04n03/#FAQ04
                tle_line1 = line1[:7] + "U 00000ABC 00001.00000000 " + line1[33:]
                tle_line1 = tle_line1[:68] + str(calculate_tle_line_checksum(tle_line1[:68]))
                tle_line2 = line2

                # Check that the checksum is correct
                if len(tle_line1) != 69 or calculate_tle_line_checksum(tle_line1[:68]) != int(tle_line1[68]):
                    raise ValueError("TLE line 1 checksum failed")
                if len(tle_line2) != 69 or calculate_tle_line_checksum(tle_line2[:68]) != int(tle_line2[68]):
                    raise ValueError("TLE line 2 checksum failed")

                # Write TLE to file
                f_out.write(constellation_name + " " + str(orbit * num_sats_per_orbit + n_sat) + "\n")
                f_out.write(tle_line1 + "\n")
                f_out.write(tle_line2 + "\n")

                # One more satellite there
                satellite_counter += 1


def generate_tles_from_scratch_manual(
        filename_out,
        constellation_name,
        num_orbits,
        num_sats_per_orbit,
        phase_diff,
        inclination_degree,
        eccentricity,
        arg_of_perigee_degree,
        mean_motion_rev_per_day
):

    with open(filename_out, "w+") as f_out:

        # First line:
        #
        # <number of orbits> <number of satellites per orbit>
        #
        f_out.write("%d %d\n" % (num_orbits, num_sats_per_orbit))

        # Each of the subsequent (number of orbits * number of satellites per orbit) blocks
        # define a satellite as follows:
        #
        # <constellation_name> <global satellite id>
        # <TLE line 1>
        # <TLE line 2>
        satellite_counter = 0
        for orbit in range(0, num_orbits):

            # Orbit-dependent
            raan_degree = orbit * 360.0 / num_orbits
            orbit_wise_shift = 0
            if orbit % 2 == 1:
                if phase_diff:
                    orbit_wise_shift = 360.0 / (num_sats_per_orbit * 2.0)

            # For each satellite in the orbit
            for n_sat in range(0, num_sats_per_orbit):
                mean_anomaly_degree = orbit_wise_shift + (n_sat * 360 / num_sats_per_orbit)

                # Epoch is 2000-01-01 00:00:00, which is 00001 in ddyyy format
                # See also: https://www.celestrak.com/columns/v04n03/#FAQ04
                tle_line1 = "1 %05dU 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    0" % (
                    satellite_counter + 1
                )

                tle_line2 = "2 %05d %s %s %s %s %s %s    0" % (
                    satellite_counter + 1,
                    ("%3.4f" % inclination_degree).rjust(8),
                    ("%3.4f" % raan_degree).rjust(8),
                    ("%0.7f" % eccentricity)[2:],
                    ("%3.4f" % arg_of_perigee_degree).rjust(8),
                    ("%3.4f" % mean_anomaly_degree).rjust(8),
                    ("%2.8f" % mean_motion_rev_per_day).rjust(11),
                )

                # Append checksums
                tle_line1 = tle_line1 + str(calculate_tle_line_checksum(tle_line1))
                tle_line2 = tle_line2 + str(calculate_tle_line_checksum(tle_line2))

                # Write TLE to file
                f_out.write(constellation_name + " " + str(orbit * num_sats_per_orbit + n_sat) + "\n")
                f_out.write(tle_line1 + "\n")
                f_out.write(tle_line2 + "\n")

                # One more satellite there
                satellite_counter += 1


def calculate_tle_line_checksum(tle_line_without_checksum):
    if len(tle_line_without_checksum) != 68:
        raise ValueError("Must have exactly 68 characters")
    s = 0
    for i in range(len(tle_line_without_checksum)):
        if tle_line_without_checksum[i].isnumeric():
            s += int(tle_line_without_checksum[i])
        if tle_line_without_checksum[i] == "-":
            s += 1
    return s % 10
