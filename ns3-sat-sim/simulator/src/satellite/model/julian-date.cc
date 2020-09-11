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

#include "julian-date.h"

#include <cmath>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <stdint.h>
#include <string>

#include "ns3/assert.h"
#include "ns3/nstime.h"

#include "iers-data.h"
#include "sgp4unit.h"
#include "sgp4ext.h"

namespace ns3 {

const uint32_t JulianDate::PosixYear = 1970;          //!< POSIX epoch year
const uint32_t JulianDate::MinYear = 1992;            //!< IERS data since
const uint32_t JulianDate::MaxYear = 2099;            //!< maximum year
const double JulianDate::PosixEpoch = 2440587.5;      //!< 1 Jan 1970, 0h
const uint32_t JulianDate::J2000Epoch = 2451545;      //!< 1 Jan 2000, 12h
const uint32_t JulianDate::Posix1992 = 8035;          //!< 1 Jan 1992 (Unix days)
const uint32_t JulianDate::HourToMs = 3600000;        //!< millisecs in an hour
const uint32_t JulianDate::DayToMs = HourToMs*24;     //!< millisecs in a day
const Time JulianDate::TtToTai = MilliSeconds(32184); //!< TT - TAI (in ms)
const Time JulianDate::TaiToGps = MilliSeconds(19000);//!< TAI - GPST (in ms)

std::ostream&
operator<< (std::ostream &os, DateTime::TimeSystem ts)
{
  switch (ts)
  {
    case DateTime::UTC:   os << "UTC"; break;
    case DateTime::TAI:   os << "TAI"; break;
    case DateTime::TT:    os << "TT";  break;
    case DateTime::UT1:   os << "UT1"; break;
    case DateTime::GPST:  os << "GPS"; break;
    case DateTime::POSIX: os << "UTC"; break;     // POSIX is printed as UTC
    default: break;
  }

  return os;
}

JulianDate::JulianDate ()
{
  // the date since complete IERS data is available
  m_days = static_cast<uint32_t> ((MinYear - PosixYear)*365.25);
  m_ms_day = 0;
  m_time_scale = DateTime::UTC;
}

JulianDate::JulianDate (double jd)
{
  SetDate (jd);
}

JulianDate::JulianDate (uint32_t days, uint32_t ms_day)
{
  SetDate (days, ms_day);
}

JulianDate::JulianDate (const std::string &date, TimeSystem ts)
{
  SetDate (date, ts);
}

double
JulianDate::GetDouble (TimeSystem ts) const
{
  double base = m_days + m_ms_day/static_cast<double> (DayToMs);

  if (ts != DateTime::POSIX)
    return base += PosixEpoch;

  // transform base into Julian date, and add offset (if any)
  return base + OffsetFromUtc (m_days, ts).GetDays ();
}

DateTime
JulianDate::GetDateTime (void) const
{
  return GregorianDate ();
}

DateTime
JulianDate::GetDateTime (TimeSystem ts) const
{
  return (*this + OffsetFromUtc (m_days, ts)).GregorianDate ();
}

std::string
JulianDate::ToString (void) const
{
  std::ostringstream oss;
  DateTime dt = (*this + OffsetFromUtc ()).GregorianDate ();

  oss << std::setfill ('0');
  oss << std::setw (4) << dt.year << "-" << std::setw (2) << dt.month << "-"
      << std::setw (2) << dt.day << " " << std::setw (2) << dt.hours << ":"
      << std::setw (2) << dt.minutes << ":" << std::setw (2) << dt.seconds
      << "." << std::setw (3) << dt.millisecs << " " << m_time_scale;

  return oss.str ();
}

std::string
JulianDate::ToString (TimeSystem ts) const
{
  JulianDate jd = *this;

  jd.m_time_scale = ts;

  return jd.ToString ();
}

std::pair<double, double>
JulianDate::GetPolarMotion (void) const
{
  const std::vector<IersData::EopParameters> &v = IersData::EopValues;
  uint32_t pos = m_days - Posix1992;            // full days since 01 Jan 1992

  // if there is data available
  if (pos < v.size ())
    return std::make_pair (v[pos].xp, v[pos].yp);

  return std::make_pair (0, 0);
}

double
JulianDate::GetOmegaEarth (void) const
{
  const std::vector<IersData::EopParameters> &v = IersData::EopValues;
  uint32_t pos = m_days - Posix1992;              // full days since 1 Jan 1992
  double lod = (pos < v.size() ? v[pos].lod : 0); // LOD in milliseconds

  // use IERS angular velocity formula with extra precision if LOD is available
  // LOD is in milliseconds, and the result is in radians/s
  return 7.2921151467064e-5*(1.0 - lod/DayToMs);
}

// from http://aa.usno.navy.mil/faq/docs/GAST.php
double
JulianDate::GetGmst (void) const
{
  ///@TODO interpolate DUT1 between daily values
  return gstime ((*this + Dut1 (m_days)).GetDouble ());

  ///@TODO evaluate which formula is more accurate

  //JulianDate jdut1 = *this + Dut1 (m_days);

  // from http://aa.usno.navy.mil/faq/docs/GAST.php
  //
  // 6.697374558 + 0.06570982441908*D0 + 1.00273790935*H + 0.000026*T²
  //
  // JD0 = jdut1.m_days + PosixEpoch
  // D0  = jdut1.m_days + PosixEpoch - J2000Epoch
  // H   = jdut1.m_ms_day/HourToMs
  // D   = D0 + H/24
  // T   = D/36525.0 = (D0 + jdut1.m_ms_day/DayToMs)/36525.0

  //double d0 = jdut1.m_days + PosixEpoch - J2000Epoch;
  //double h = jdut1.m_ms_day/static_cast<double> (HourToMs);
  //double t = (d0 + jdut1.m_ms_day/static_cast<double> (DayToMs))/36525.0;
  //double gmst = 6.697374558 + 0.06570982441908*d0 + 1.00273790935*h +
  //              0.000026*t*t;

  //gmst = fmod(gmst * M_PI/12, 2*M_PI);

  //return (gmst < 0 ? gmst+2*M_PI : gmst);
}

void
JulianDate::SetDate (double jd)
{
  // POSIX/Unix epoch is used internally
  jd -= PosixEpoch;

  m_days = static_cast<uint32_t> (jd);
  m_ms_day = static_cast<uint32_t> ((jd - m_days)*DayToMs);
  m_time_scale = DateTime::UTC;
}

void
JulianDate::SetDate (uint32_t days, uint32_t ms_day)
{
  // POSIX/Unix epoch is used internally
  m_days = days;
  m_ms_day = ms_day;
  m_time_scale = DateTime::POSIX;
}

void
JulianDate::SetDate (const std::string &date, TimeSystem ts)
{
  double seconds;
  DateTime dt;

  NS_ASSERT_MSG (
    ts != DateTime::POSIX, "POSIX/Unix time scale is not valid for dates."
  );

  // YYYY-MM-DD HH:MM:SS(.MMM)
  std::sscanf (
    date.c_str(), "%04d%*c%02d%*c%02d %02d%*c%02d%*c%lf",
    &dt.year, &dt.month, &dt.day, &dt.hours, &dt.minutes, &seconds
  );

  NS_ASSERT_MSG (
    dt.year >= MinYear, "Complete EOP data is not available before that date!"
  );

  NS_ASSERT_MSG (
    dt.year <= MaxYear, "Dates beyond 2099 are not supported!"
  );

  dt.seconds = static_cast<uint32_t> (seconds);
  dt.millisecs = static_cast<uint32_t> ((seconds - dt.seconds)*1000 + 0.5);

  // based on formula from http://aa.usno.navy.mil/faq/docs/JD_Formula.php
  m_days = 367*dt.year - 7*(dt.year + (dt.month+9)/12)/4 + 275*dt.month/9 +
           dt.day - static_cast<uint32_t>(PosixEpoch - 1721013.5);
  m_ms_day = (dt.hours*3600 + dt.minutes*60 + dt.seconds)*1000 + dt.millisecs;
  m_time_scale = ts;

  *this += OffsetToUtc ();
}

JulianDate
JulianDate::operator+ (const Time &t) const
{
  JulianDate jd;

  // if time is negative, call operator- with a positive time
  if (t.IsStrictlyNegative ())
    return *this - MilliSeconds (-t.GetMilliSeconds ());

  jd.m_days = static_cast<uint32_t> (t.GetDays ());
  jd.m_ms_day = static_cast<uint32_t> (t.GetMilliSeconds () % DayToMs);

  jd.m_ms_day += m_ms_day;
  jd.m_days += m_days + jd.m_ms_day/DayToMs;
  jd.m_ms_day %= DayToMs;
  jd.m_time_scale = m_time_scale;

  return jd;
}

void
JulianDate::operator+= (const Time &t)
{
  *this = *this + t;
}

JulianDate
JulianDate::operator- (const Time &t) const
{
  JulianDate jd;

  // if time is negative, call operator+ with a positive time
  if (t.IsStrictlyNegative ())
    return *this + MilliSeconds (-t.GetMilliSeconds ());

  jd.m_days = static_cast<uint32_t> (t.GetDays ());
  jd.m_ms_day = static_cast<uint32_t> (t.GetMilliSeconds () % DayToMs);

  if (jd.m_ms_day > m_ms_day)
  {
    jd.m_ms_day = DayToMs - (jd.m_ms_day - m_ms_day);
    jd.m_days += 1;
  }
  else
    jd.m_ms_day = m_ms_day - jd.m_ms_day;

  jd.m_days = m_days - jd.m_days;
  jd.m_time_scale = m_time_scale;

  return jd;
}

void
JulianDate::operator-= (const Time &t)
{
  *this = *this - t;
}

Time
JulianDate::operator- (const JulianDate &jd) const
{
  Time t1 = ns3::Days(m_days) + ns3::MilliSeconds(m_ms_day);
  Time t2 = ns3::Days(jd.m_days) + ns3::MilliSeconds(jd.m_ms_day);

  return t1 - t2;
}

bool
JulianDate::operator< (const JulianDate &jd) const {
  if(m_days != jd.m_days)
    return m_days < jd.m_days;

  return m_ms_day < jd.m_ms_day;
}

bool
JulianDate::operator<= (const JulianDate &jd) const {
  if(m_days != jd.m_days)
    return m_days < jd.m_days;

  return m_ms_day <= jd.m_ms_day;
}

bool
JulianDate::operator> (const JulianDate &jd) const {
  if(m_days != jd.m_days)
    return m_days > jd.m_days;

  return m_ms_day > jd.m_ms_day;
}

bool
JulianDate::operator>= (const JulianDate &jd) const {
  if(m_days != jd.m_days)
    return m_days > jd.m_days;

  return m_ms_day >= jd.m_ms_day;
}

bool
JulianDate::operator== (const JulianDate &jd) const {
  if(m_days != jd.m_days)
    return false;

  return m_ms_day == jd.m_ms_day;
}

bool
JulianDate::operator!= (const JulianDate &jd) const {
  if(m_days != jd.m_days)
    return true;

  return m_ms_day != jd.m_ms_day;
}

std::ostream&
operator<< (std::ostream &os, const JulianDate &jd)
{
  os << jd.ToString ();

  return os;
}

//
//
//

bool
JulianDate::IsLeapYear (uint32_t year)
{
  // if the year is divisible by 4, it is a leap year
  // [this works because we only consider dates between 1992 and 2099]
  return ((year & 0x03) == 0);
}

Time
JulianDate::TaiMinusUtc (uint32_t daysInPosix)
{
  const std::vector<uint32_t> &v = IersData::LeapSeconds;
  std::vector<uint32_t>::const_iterator it;

  it = std::lower_bound (v.begin (), v.end (), daysInPosix);

  return MilliSeconds (((it - v.begin ()) + IersData::BaseLeapSeconds)*1000);
}

Time
JulianDate::Dut1 (uint32_t daysInPosix)
{
  const std::vector<IersData::EopParameters> &v = IersData::EopValues;
  uint32_t pos = daysInPosix - Posix1992;         // full days since 01 Jan 1992

  return MilliSeconds (pos < v.size () ? v[pos].dut1*1000 : 0);
}

DateTime
JulianDate::GregorianDate (void) const
{
  return GregorianDate (m_days, m_ms_day);
}

DateTime
JulianDate::GregorianDate (uint32_t days, uint32_t ms_day)
{
  uint32_t month_days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  uint32_t d = days - Posix1992, days_of_year, leap_years;
  Time t = MilliSeconds (ms_day);
  DateTime dt;

  // this formula only works if we consider an year that is multiple of 4
  dt.year = static_cast<uint32_t> (d / 365.25) + MinYear;

  leap_years = (dt.year - (MinYear + 1))/4;
  days_of_year = d - ((dt.year - MinYear)*365 + leap_years);

  // if it is a leap year, February has 29 days
  if (IsLeapYear (dt.year))
    month_days[1] += 1;

  for (uint32_t i = 0; i < 12; ++i) {
    if (days_of_year <= month_days[i]) {
      dt.month = i+1;

      break;
    }

    days_of_year -= month_days[i];
  }

  dt.day = days_of_year;

  dt.hours = static_cast<uint32_t> (t.GetHours ());
  t -= Hours (dt.hours);

  dt.minutes = static_cast<uint32_t> (t.GetMinutes ());
  t -= Minutes (dt.minutes);

  dt.seconds = static_cast<uint32_t> (t.GetSeconds ());
  t -= Seconds (dt.seconds);

  dt.millisecs = static_cast<uint32_t> (t.GetMilliSeconds ());

  return dt;
}

Time
JulianDate::OffsetFromUtc () const
{
  return OffsetFromUtc (m_days, m_time_scale);
}

Time
JulianDate::OffsetFromUtc (uint32_t daysInPosix, TimeSystem ts)
{
  // already in sync for UTC and POSIX
  Time offset = MilliSeconds (0);

  switch(ts) {
    case DateTime::UT1: offset = Dut1 (daysInPosix); break;
    case DateTime::TAI: offset = TaiMinusUtc (daysInPosix); break;
    case DateTime::TT:  offset = TtToTai + TaiMinusUtc (daysInPosix); break;
    case DateTime::GPST: offset = TaiMinusUtc (daysInPosix) - TaiToGps; break;
    default: break;
  }

  return offset;
}

Time
JulianDate::OffsetToUtc (void) const {
  return OffsetToUtc (m_days, m_ms_day, m_time_scale);
}

Time
JulianDate::OffsetToUtc (uint32_t daysInPosix, uint32_t ms_day, TimeSystem ts)
{
  // already in sync
  if (ts == DateTime::UTC || ts == DateTime::POSIX)
    return MilliSeconds (0);

  Time tai_utc = TaiMinusUtc (daysInPosix);
  Time offset = (ts == DateTime::TT ? TtToTai : MilliSeconds (0));

  // if it is not the same day in UTC, check the leap secs for the previous day
  if (ms_day < (offset + tai_utc).GetMilliSeconds ())
    tai_utc = TaiMinusUtc (daysInPosix-1);

  switch (ts) {
    case DateTime::UT1: offset = Dut1 (daysInPosix); break;
    case DateTime::TAI: offset = tai_utc; break;
    case DateTime::TT:  offset = TtToTai + tai_utc; break;
    case DateTime::GPST: offset = tai_utc - TaiToGps; break;
    default: break;
  }

  // return the offset as negative
  return MilliSeconds (-offset.GetMilliSeconds ());
}

}
