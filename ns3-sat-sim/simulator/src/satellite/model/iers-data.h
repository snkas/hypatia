/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 INESC TEC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Pedro Silva  <pmms@inesctec.pt>
 *
 */

#ifndef IERS_DATA_H
#define IERS_DATA_H

#include <stdint.h>
#include <vector>

/*
 * IMPORTANT: This file was generated automatically. Do not change it!
 *            Instead, update files 'tai-utc.dat', and 'finals.data' on
 *            src/satellite/model/data/ folder using the latest files
 *            provided at:
 * http://www.usno.navy.mil/USNO/earth-orientation/eo-info/general/bullc
 * and 
 * http://www.usno.navy.mil/USNO/earth-orientation/eo-products/weekly or
 * http://www.iers.org/IERS/EN/DataProducts/EarthOrientationData/eop.html
 */
namespace ns3 {

class IersData {
public:
  struct EopParameters {
    // radians, radians, seconds, milliseconds
    double xp, yp, dut1, lod;
  };

  static const std::vector<EopParameters> EopValues;
  static const uint32_t BaseLeapSeconds;
  static const uint32_t CurLeapSeconds;
  static const std::vector<uint32_t> LeapSeconds;

protected:
  static const EopParameters __eop_params[];
  static const uint32_t __leap_secs[];
};

}

#endif // IERS_DATA_H

