# Copyright (c) 2012-2019 Juan Luis Cano Rodríguez and the poliastro development team
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

from typing import Any, List

import numpy as np
from astropy import units as u
from astropy.coordinates import CartesianRepresentation
from astropy.time import Time, TimeDelta
from czml3.core import Packet, Preamble
from czml3.enums import InterpolationAlgorithms, ReferenceFrames
from czml3.properties import (
    Billboard,
    Clock,
    Color,
    Label,
    Material,
    Path,
    Position,
    PositionList,
    Polyline,
    SolidColorMaterial,
)
from czml3.types import IntervalValue, TimeInterval

from poliastro.bodies import Earth
from poli_czml_utils import ellipsoidal_to_cartesian, project_point_on_ellipsoid
from poliastro.twobody.propagation import propagate

PIC_SATELLITE = "data:image/png;base64,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" #sat7
PIC_GROUNDSTATION = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAACvSURBVDhPrZDRDcMgDAU9GqN0lIzijw6SUbJJygUeNQgSqepJTyHG91LVVpwDdfxM3T9TSl1EXZvDwii471fivK73cBFFQNTT/d2KoGpfGOpSIkhUpgUMxq9DFEsWv4IXhlyCnhBFnZcFEEuYqbiUlNwWgMTdrZ3JbQFoEVG53rd8ztG9aPJMnBUQf/VFraBJeWnLS0RfjbKyLJA8FkT5seDYS1Qwyv8t0B/5C2ZmH2/eTGNNBgMmAAAAAElFTkSuQmCC"


class CZMLExtractor:
    """A class for extracting orbitary data to Cesium"""

    def __init__(
        self, start_epoch, end_epoch, N, attractor=None, pr_map=None, scene3D=True
    ):
        """
        Orbital constructor
        Parameters
        ----------
        start_epoch: ~astropy.time.core.Time
            Starting epoch
        end_epoch: ~astropy.time.core.Time
            Ending epoch
        N: int
            Default number of sample points.
            Unless otherwise specified, the number
            of sampled data points will be N when calling
            add_orbit()
        attractor: poliastro.Body
            Attractor of the orbits
        scene3D: bool
            Determines the scene mode. If set to true, the scene
            is set to 3D mode, otherwise it's the orthographic
            projection.
        """
        self.packets = []  # type: List[Packet]

        self.attractor = attractor
        self.orbits = []  # type: List[Any]
        self.N = N
        self.i = 0
        self.j = 0

        if not self.attractor:
            self.attractor = Earth
        elif not (self.attractor.R and self.attractor.R_polar):
            raise ValueError(
                "Invalid ellipsoid of attractor.\n"
                + "Make sure your body has valid 'R' and 'R_polar' parameters"
            )

        ellipsoid = (
            self.attractor.R.to(u.m).value,
            self.attractor.R.to(u.m).value,
            self.attractor.R_polar.to(u.m).value,
        )

        self.cust_prop = [ellipsoid, pr_map, scene3D]

        self.start_epoch = Time(start_epoch, format="isot")
        self.end_epoch = Time(end_epoch, format="isot")

        self._init_czml_()

        self._change_custom_params(*self.cust_prop)

    def _init_orbit_packet_cords_(self, i, rtol):
        """
        Parameters
        ----------
        i: int
            Index of referenced orbit
        rtol: float
            Maximum relative error permitted
        Returns
        -------
        coordinate list
        """
        cart_cords = []  # type: List[float]

        h = (self.end_epoch - self.orbits[i][2]).to(u.second) / self.orbits[i][1]

        # Get rounding factor given the relative tolerance
        rf = 0
        while rtol < 1:
            rtol *= 10
            rf += 1

        for k in range(self.orbits[i][1] + 2):
            position = propagate(self.orbits[i][0], TimeDelta(k * h), rtol=rtol)

            cords = position.represent_as(CartesianRepresentation).xyz.to(u.meter).value
            cords = np.insert(cords, 0, h.value * k, axis=0)

            # flatten list
            cart_cords += list(map(lambda x: round(x[0], rf), cords.tolist()))

        return cart_cords

    def _init_groundtrack_packet_cords_(self, i, rtol):
        """
        Parameters
        ----------
        i: int
            Index of referenced orbit
        rtol: float
            Maximum relative error permitted
        Returns
        -------
        coordinate list
        """
        cart_cords = []  # type: List[float]

        h = (self.end_epoch - self.orbits[i][2]).to(u.second) / self.orbits[i][1]

        # Get rounding factor given the relative tolerance
        rf = 0
        while rtol < 1:
            rtol *= 10
            rf += 1

        ellipsoid = self.cust_prop[0]

        for k in range(self.orbits[i][1] + 2):
            position = propagate(self.orbits[i][0], TimeDelta(k * h), rtol=rtol)

            cords = position.represent_as(CartesianRepresentation).xyz.to(u.meter).value
            cords = np.insert(cords, 0, h.value * k, axis=0)

            # flatten list
            cords = list(map(lambda x: round(x[0], rf), cords.tolist()))
            t, p = cords[0], cords[1:]
            pr_p = project_point_on_ellipsoid(
                p[0], p[1], p[2], ellipsoid[0], ellipsoid[1], ellipsoid[2]
            )
            # Add a small number to ensure that our point lies above the surface of the
            # ellipsoid. We do this because small losses in precision may cause the point
            # to lie slightly bellow the surface. An iterative method could be used instead
            # but the error margin is too small to be worth it.

            _cords = t, pr_p[0] + 0.1, pr_p[1] + 0.1, pr_p[2] + 0.1
            cart_cords += _cords

        return cart_cords

    def _init_czml_(self):
        """
        Only called at the initialization of the extractor
        Builds packets.
        """
        pckt = Preamble(
            name="document_packet",
            clock=IntervalValue(
                start=self.start_epoch.value,
                end=self.end_epoch.value,
                value=Clock(currentTime=self.start_epoch.value, multiplier=1),
            ),
        )
        self.packets.append(pckt)

    def _change_custom_params(self, ellipsoid, pr_map, scene3D):
        """
        Change the custom properties package.
        Parameters
        ----------
        ellipsoid: list(int)
            Defines the attractor ellipsoid. The list must have three numbers
            representing the radii in the x, y and z axis
        pr_map: str
            A URL to the projection of the defined ellipsoid (UV map)
        """

        if pr_map is None:
            pr_map = (
                 "https://upload.wikimedia.org/wikipedia/commons/a/ac/Earthmap1000x500.jpg",
            )
            self.cust_prop[1] = pr_map

        custom_props = {
            "custom_attractor": True,
            "ellipsoid": [{"array": ellipsoid}],
            "map_url": pr_map,
            "scene3D": scene3D,
        }

        pckt = Packet(id="custom_properties", properties=custom_props)

        self.packets.append(pckt)

    def add_ground_station(
        self,
        pos,
        description=None,
        label_fill_color=None,
        label_font=None,
        label_outline_color=None,
        label_text=None,
        label_show=True,
    ):
        """
        Adds a ground station
        Parameters
        ----------
        orbit: poliastro.Orbit
            Orbit to be added
        pos: list [~astropy.units]
            coordinates of ground station
            [u v] ellipsoidal coordinates (0 elevation)
        Id parameters:
        -------------
        description: str
            Set ground station description
        Label parameters
        ----------
        label_fill_color: list (int)
            Fill Color in rgba format
        label_outline_color: list (int)
            Outline Color in rgba format
        label_font: str
            Set label font style and size (CSS syntax)
        label_text: str
            Set label text
        label_show: bool
            Indicates whether the label is visible
        """
        if (
            len(pos) == 2
            and isinstance(pos[0], u.quantity.Quantity)
            and isinstance(pos[0], u.quantity.Quantity)
        ):
            u0, v0 = pos

            if self.cust_prop[0]:
                a, b = (
                    self.cust_prop[0][0],
                    self.cust_prop[0][2],
                )  # get semi-major and semi-minor axises
            else:
                a, b = Earth.R.to(u.m).value, Earth.R_polar.to(u.m).value
            pos = list(map(lambda x: x.value, ellipsoidal_to_cartesian(a, b, u0, v0)))
        else:
            raise TypeError(
                "Invalid coordinates. Coordinates must be of the form [u, v] where u, v are astropy units"
            )

        pckt = Packet(
            id=self.i,
            description=description,
            availability=TimeInterval(start=self.start_epoch.value, end=self.end_epoch.value),
            position=Position(cartesian=pos),
            label=Label(
                show=label_show,
                text=label_text,
                font=label_font if label_font is not None else "11pt Lucida Console",
                fillColor=Color(rgba=label_fill_color)
                if label_fill_color is not None
                else None,
                outlineColor=Color(rgba=label_outline_color)
                if label_outline_color is not None
                else None,
            ),
            billboard=Billboard(image=PIC_GROUNDSTATION, show=True),
        )

        self.packets.append(pckt)
        self.i += 1

    def add_orbit(
        self,
        orbit,
        rtol=1e-10,
        N=None,
        groundtrack_show=False,
        groundtrack_lead_time=None,
        groundtrack_trail_time=None,
        groundtrack_width=None,
        groundtrack_color=None,
        id_name=None,
        description=None,
        path_width=None,
        path_show=None,
        path_color=[205, 205, 205, 50],
        label_fill_color=None,
        label_outline_color=None,
        label_font=None,
        label_text=None,
        label_show=None,
    ):
        """
        Adds an orbit
        Parameters
        ----------
        orbit: poliastro.Orbit
            Orbit to be added
        rtol: float
            Maximum relative error permitted
        N: int
            Number of sample points
        Groundtrack parameters:
        -----------------------
        groundtrack_show: bool
            If set to true, the groundtrack is
            displayed.
        groundtrack_lead_time: double
            The time the animation is ahead of the real-time groundtrack
        groundtrack_trail_time: double
            The time the animation is behind the real-time groundtrack
        groundtrack_width: int
            Groundtrack width
        groundtrack_color: list (int)
            Rgba groundtrack color. By default, it is set to the path color
        Id parameters:
        --------------
        id_name: str
            Set orbit name
        description: str
            Set orbit description
        Path parameters
        ---------------
        path_width: int
            Path width
        path_show: bool
            Indicates whether the path is visible
        path_color: list (int)
            Rgba path color
        Label parameters
        ----------
        label_fill_color: list (int)
            Fill Color in rgba format
        label_outline_color: list (int)
            Outline Color in rgba format
        label_font: str
            Set label font style and size (CSS syntax)
        label_text: str
            Set label text
        label_show: bool
            Indicates whether the label is visible
        """

        if N is None:
            N = self.N

        if orbit.epoch < Time(self.start_epoch):
            orbit = orbit.propagate(self.start_epoch - orbit.epoch)
        elif orbit.epoch > Time(self.end_epoch):
            raise ValueError(
                "The orbit's epoch cannot exceed the constructor's ending epoch"
            )

        if rtol <= 0 or rtol >= 1:
            raise ValueError(
                "The relative tolerance must be a value in the range (0, 1)"
            )

        self.orbits.append([orbit, N, orbit.epoch])
        cartesian_cords = self._init_orbit_packet_cords_(self.i, rtol=rtol)

        start_epoch = Time(min(self.orbits[self.i][2], self.start_epoch), format="isot")

        pckt = Packet(
            id=self.i,
            name=id_name,
            description=description,
            availability=TimeInterval(start=self.start_epoch.value, end=self.end_epoch.value),
            position=Position(
                interpolationDegree=5,
                interpolationAlgorithm=InterpolationAlgorithms.LAGRANGE,
                referenceFrame=ReferenceFrames.INERTIAL,
                cartesian=cartesian_cords,
                epoch=start_epoch.value,
            ),
            path=Path(
                show=path_show,
                width=path_width,
                material=Material(
                    solidColor=SolidColorMaterial(color=Color(rgba=path_color))
                )
                if path_color is not None
                else Material(
                    solidColor=SolidColorMaterial(color=Color(rgba=[255, 255, 0, 255]))
                ),
                resolution=120,
            ),
            label=Label(
                text=label_text,
                font=label_font if label_font is not None else "11pt Lucida Console",
                show=label_show,
                fillColor=Color(rgba=label_fill_color)
                if label_fill_color is not None
                else Color(rgba=[255, 255, 0, 255]),
                outlineColor=Color(rgba=label_outline_color)
                if label_outline_color is not None
                else Color(rgba=[255, 255, 0, 255]),
            ),
            #billboard=Billboard(image=PIC_SATELLITE, show=True),
            billboard=Billboard(image=PIC_SATELLITE, show=True, scale=0.15),
        )

        self.packets.append(pckt)

        if groundtrack_show:

            groundtrack_color = path_color

            groundtrack_cords = self._init_groundtrack_packet_cords_(self.i, rtol=rtol)
            pckt = Packet(
                id="groundtrack" + str(self.i),
                availability=TimeInterval(start=self.start_epoch, end=self.end_epoch),
                position=Position(
                    interpolationDegree=5,
                    interpolationAlgorithm=InterpolationAlgorithms.LAGRANGE,
                    referenceFrame=ReferenceFrames.INERTIAL,
                    cartesian=groundtrack_cords,
                    epoch=start_epoch.value,
                ),
                path=Path(
                    show=True,
                    material=Material(
                        solidColor=SolidColorMaterial(
                            color=Color(rgba=groundtrack_color)
                        )
                    )
                    if groundtrack_color is not None
                    else Material(
                        solidColor=SolidColorMaterial(
                            color=Color(rgba=[255, 255, 0, 255])
                        )
                    ),
                    resolution=60,
                    width=groundtrack_width,
                    leadTime=groundtrack_lead_time if groundtrack_lead_time else 100,
                    trailTime=groundtrack_trail_time if groundtrack_trail_time else 100,
                ),
            )
            self.packets.append(pckt)

        self.i += 1
        return self.i-1

    def add_link(
        self,
        sat1,
        sat2,
        start_time,
        end_time,
        color=[255, 255, 0, 255],
        width=1.0,
        id_name=None,
        description=None
    ):

        start_time = Time(start_time, format="isot")
        end_time = Time(end_time, format="isot")

        pckt = Packet(
            id="s" + str(self.j),
            name=id_name,
            description=description,
            availability=TimeInterval(start=start_time.value, end=end_time.value),
            polyline=Polyline(
                positions=PositionList(
                    references=[str(sat1) + "#position", str(sat2) + "#position"]
                ),
                material=Material(
                    solidColor=SolidColorMaterial(
                        color=Color(rgba=color)
                    )
                ),
                width=width,
            )
        )

        self.packets.append(pckt)
        self.j += 1
