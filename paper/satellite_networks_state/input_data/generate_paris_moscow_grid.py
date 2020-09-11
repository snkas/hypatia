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

# Latitude and longitude of endpoints
# (Taken from the top 100)
paris_latitude_longitude = (48.85341, 2.3488)
moscow_latitude_longitude = (55.754996, 37.621849)

# Grid size (+ 2 because also along border an extra padding is added)
num_latitude = 4
num_longitude = 8
delta_latitude = (paris_latitude_longitude[0] - moscow_latitude_longitude[0]) / float(num_latitude)
delta_longitude = (paris_latitude_longitude[1] - moscow_latitude_longitude[1]) / float(num_longitude)

# Open file
with open("ground_stations_paris_moscow_grid.basic.txt", "w+") as f_out:

    # Paris (source)
    f_out.write("0,Paris,%.10f,%.10f,0\n" % (paris_latitude_longitude[0], paris_latitude_longitude[1]))

    # Grid of way points
    num_added = 1
    for i in range(-1, num_latitude + 2):
        for j in range(-1, num_longitude + 2):
            if not ((i == 0 and j == 0) or (i == num_latitude and j == num_longitude)):
                f_out.write("%d,Waypoint-%d,%.10f,%.10f,0\n" % (
                    num_added,
                    num_added,
                    paris_latitude_longitude[0] - i * delta_latitude,
                    paris_latitude_longitude[1] - j * delta_longitude
                ))
                num_added += 1

    # Moscow (destination)
    f_out.write("%d,Moskva-(Moscow),%.10f,%.10f,0\n" % (
        num_added,
        moscow_latitude_longitude[0],
        moscow_latitude_longitude[1]
    ))
