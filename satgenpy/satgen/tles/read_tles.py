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

import ephem
from astropy.time import Time
from astropy import units as u


def read_tles(filename_tles):
    """
    Read a constellation of satellites from the TLES file.

    :param filename_tles:                    Filename of the TLES (typically /path/to/tles.txt)

    :return: Dictionary: {
                    "n_orbits":             Number of orbits
                    "n_sats_per_orbit":     Satellites per orbit
                    "epoch":                Epoch
                    "satellites":           Dictionary of satellite id to
                                            {"ephem_obj_manual": <obj>, "ephem_obj_direct": <obj>}
              }
    """
    satellites = []
    with open(filename_tles, 'r') as f:
        n_orbits, n_sats_per_orbit = [int(n) for n in f.readline().split()]
        universal_epoch = None
        i = 0
        for tles_line_1 in f:
            tles_line_2 = f.readline()
            tles_line_3 = f.readline()

            # Retrieve name and identifier
            name = tles_line_1
            sid = int(name.split()[1])
            if sid != i:
                raise ValueError("Satellite identifier is not increasing by one each line")
            i += 1

            # Fetch and check the epoch from the TLES data
            # In the TLE, the epoch is given with a Julian data of yyddd.fraction
            # ddd is actually one-based, meaning e.g. 18001 is 1st of January, or 2018-01-01 00:00.
            # As such, to convert it to Astropy Time, we add (ddd - 1) days to it
            # See also: https://www.celestrak.com/columns/v04n03/#FAQ04
            epoch_year = tles_line_2[18:20]
            epoch_day = float(tles_line_2[20:32])
            epoch = Time("20" + epoch_year + "-01-01 00:00:00", scale="tdb") + (epoch_day - 1) * u.day
            if universal_epoch is None:
                universal_epoch = epoch
            if epoch != universal_epoch:
                raise ValueError("The epoch of all TLES must be the same")

            # Finally, store the satellite information
            satellites.append(ephem.readtle(tles_line_1, tles_line_2, tles_line_3))

    return {
        "n_orbits": n_orbits,
        "n_sats_per_orbit": n_sats_per_orbit,
        "epoch": epoch,
        "satellites": satellites
    }


def satellite_ephem_to_str(satellite_ephem):
    res = "EphemSatellite {\n"
    res += "  name = \"" + str(satellite_ephem.name) + "\",\n"
    res += "  _ap = " + str(satellite_ephem._ap) + ",\n"
    res += "  _decay = " + str(satellite_ephem._decay) + ",\n"
    res += "  _drag = " + str(satellite_ephem._drag) + ",\n"
    res += "  _e = " + str(satellite_ephem._e) + ",\n"
    res += "  _epoch = " + str(satellite_ephem._epoch) + ",\n"
    res += "  _inc = " + str(satellite_ephem._inc) + ",\n"
    res += "  _M = " + str(satellite_ephem._M) + ",\n"
    res += "  _n = " + str(satellite_ephem._n) + ",\n"
    res += "  _orbit = " + str(satellite_ephem._orbit) + ",\n"
    res += "  _raan = " + str(satellite_ephem._raan) + "\n"
    res += "}"
    return res
