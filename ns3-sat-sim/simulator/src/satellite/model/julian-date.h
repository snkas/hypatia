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

#ifndef JULIANDATE_H
#define JULIANDATE_H

#include <functional>
#include <ostream>
#include <stdint.h>
#include <string>

#include "ns3/nstime.h"

namespace ns3 {

/**
 * @brief The DateTime struct.
 *
 * Struct used for simplifying the conversion between Gregorian and Julian
 * dates, and also to enumerate the available time systems.
 */
struct DateTime {
  enum TimeSystem {
    UTC,  //!< Coordinated Universal Time [French: Temps Universel Coordonné]
    UT1,  //!< Universal Time
    TAI,  //!< International Atomic Time [French: Temps Atomique International]
    TT,   //!< Terrestrial Time
    GPST, //!< Global Positioning System (GPS) Time
    POSIX //!< Unix/POSIX Time
  };

  uint32_t year, month, day;
  uint32_t hours, minutes, seconds, millisecs;
  TimeSystem time_system;

  friend std::ostream& operator<< (std::ostream &os, DateTime::TimeSystem ts);
};

/**
 * @brief Class to handle Julian days and provide respective Earth Orientation
 *        Parameters (EOP).
 *
 * This class provides an easy interface to convert between Julian days and
 * Gregorian dates in UTC, UT1, TAI, TT, GPST, and Unix/POSIX time, and, using
 * IersData class, provides the respective Earth Orientation Parameters (EOP).
 * The EOP parameters available are: DUT1, Earth's angular velocity, and
 * Greenwich Mean Sidereal Time (GMST).
 *
 * IMPORTANT: Complete EOP values are only available since 1 January 1992,
 *            therefore, this class only supports dates from 1 January 1992 up
 *            to 31 December 2099.
 */
class JulianDate {
  /// equivalent to C++11 using TimeSystem = DateTime::TimeSystem.
  typedef DateTime::TimeSystem TimeSystem;

public:
  static const uint32_t PosixYear;                //!< POSIX/Unix epoch year
  static const uint32_t MinYear;                  //!< Minimum year supported
  static const uint32_t MaxYear;                  //!< Maximum year supported
  static const double PosixEpoch;                 //!< 1 Jan 1970, 0h
  static const uint32_t J2000Epoch;               //!< 1 Jan 2000, 12h
  static const uint32_t Posix1992;                //!< 1 Jan 1992 (POSIX days)
  static const uint32_t HourToMs;                 //!< milliseconds in an hour
  static const uint32_t DayToMs;                  //!< milliseconds in a day
  static const Time TtToTai;                      //!< offset from TT to TAI
  static const Time TaiToGps;                     //!< offset from TAI to GPST

  /**
   * @brief Default constructor.
   */
  JulianDate (void);

  /**
   * @brief JulianDate constructor receiving the Julian days as parameter.
   *
   * This constructor has lower precision as there may be some inaccuracy at the
   * millisecond level due to double type limited precision.
   *
   * @param jd The Julian days.
   */
  JulianDate (double jd);

  /**
   * @brief JulianDate constructor receiving time since Unix/POSIX epoch.
   * @param days Full days since Unix/POSIX epoch (1 January 1970, 0h).
   * @param ms_day Milliseconds of the day (since midnight).
   */
  JulianDate (uint32_t days, uint32_t ms_day);

  /**
   * @brief JulianDate constructor receiving time as a Gregorian date.
   * @param date Gregorian calendar date.
   * @param ts Time system of the date (UTC, UT1, TAI, TT or GPST).
   */
  JulianDate (const std::string &date, TimeSystem ts = DateTime::UTC);

  /**
   * @brief GetDouble Get the Julian days.
   * @param ts Time system to consider for conversion (UTC by default).
   * @return the Julian days in time system ts.
   */
  double GetDouble (TimeSystem ts = DateTime::UTC) const;

  /**
   * @brief Get the Gregorian calendar date in current time system.
   * @return a DateTime struct with the Gregorian calendar date and time.
   */
  DateTime GetDateTime (void) const;

  /**
   * @brief Get the Gregorian calendar date in the specified time system.
   * @return a DateTime struct with the Gregorian calendar date.
   */
  DateTime GetDateTime (TimeSystem ts) const;

  /**
   * @brief Get the string representation of the Gregorian calendar date.
   *
   * The string is formatted as "YYYY-MM-DD hh:mm:ss.ms TTT", where YYYY is the
   * year, MM the month, DD the day of the month, hh the hour of the day, mm the
   * minutes of the day, ss the seconds of the day, and ms the milliseconds.
   *
   * @return a string representing the date in the Gregorian calendar.
   */
  std::string ToString (void) const;

  /**
   * @brief Get the string representation of the Gregorian calendar date in time
   *        system ts.
   *
   * The string is formatted as "YYYY-MM-DD hh:mm:ss.ms TTT", where YYYY is the
   * year, MM the month, DD the day of the month, hh the hour of the day, mm the
   * minutes of the day, ss the seconds of the day, and ms the milliseconds.
   *
   * @return a string representing the Gregorian calendar date in time system ts.
   */
  std::string ToString (TimeSystem ts) const;

  /**
   * @brief Retrieve the polar motion cofficients measured/predicted.
   * @return the polar motion values measured/predicted as IERS' Bulletin A.
   */
  std::pair<double, double> GetPolarMotion (void) const;

  /**
   * @brief Retrieve Earth's angular velocity.
   * @return the Earth's angular velocity.
   */
  double GetOmegaEarth (void) const;

  /**
   * @brief Retrieve the Greenwich Mean Sidereal Time.
   * @return the GMST using DUT1 values measured/predicted as IERS' Bulletin A.
   */
  double GetGmst (void) const;

  /**
   * @brief Set the Julian days.
   *
   * This function has lower precision as there may be some inaccuracy at the
   * millisecond level due to double type limited precision.
   *
   * @param jd The Julian days.
   */
  void SetDate (double jd);

  /**
   * @brief Set Julian days using the time since Unix/POSIX epoch.
   * @param days Full days since Unix/POSIX epoch (1 January 1970, 0h).
   * @param ms_day Milliseconds of the day (since midnight).
   */
  void SetDate (uint32_t days, uint32_t ms_day);

  /**
   * @brief Set Julian days using time in Gregorian calendar date.
   * @param date Gregorian calendar date.
   * @param ts Time system of the date (UTC, UT1, TAI, TT or GPST).
   */
  void SetDate (const std::string &date, TimeSystem ts = DateTime::UTC);

  /**
   * @brief operator + (positive time adjustment).
   * @param t time adjustment.
   * @return a JulianDate object with the adjustment incorporated.
   */
  JulianDate operator+ (const Time &t) const;

  /**
   * @brief operator += (positive time adjustment).
   * @param t time adjustment.
   */
  void operator+= (const Time &t);

  /**
   * @brief operator - (negative time adjustment).
   * @param t time adjustment.
   * @return a JulianDate object with the adjustment incorporated.
   */
  JulianDate operator- (const Time &t) const;

  /**
   * @brief operator -= (negative time adjustment).
   * @param t time adjustment.
   */
  void operator-= (const Time &t);

  /**
   * @brief operator - (time between Julian dates).
   * @param jd Julian days.
   * @return the time between both Julian dates.
   */
  Time operator- (const JulianDate &jd) const;

  /**
   * @brief operator < (compare Julian dates).
   * @param jd Julian days.
   * @return a boolean to indicate if the Julian date is prior.
   */
  bool operator< (const JulianDate &jd) const;

  /**
   * @brief operator <= (compare Julian dates).
   * @param jd Julian days.
   * @return a boolean to indicate if the Julian date is prior or equal.
   */
  bool operator<= (const JulianDate &jd) const;

  /**
   * @brief operator > (compare Julian dates).
   * @param jd Julian days.
   * @return a boolean to indicate if the Julian date is later.
   */
  bool operator> (const JulianDate &jd) const;

  /**
   * @brief operator >= (compare Julian dates).
   * @param jd Julian days.
   * @return a boolean to indicate if the Julian date is equal or later.
   */
  bool operator>= (const JulianDate &jd) const;

  /**
   * @brief operator == (compare Julian dates).
   * @param jd Julian days.
   * @return a boolean to indicate if the Julian date is equal.
   */
  bool operator== (const JulianDate &jd) const;

  /**
   * @brief operator != (compare Julian dates).
   * @param jd Julian days.
   * @return a boolean to indicate if the Julian date is different.
   */
  bool operator!= (const JulianDate &jd) const;

  /**
   * @brief operator << (print as GregorianDate).
   *
   * Print the textual representation (Gregorian calendar date) using the time
   * system specified upon creation or setting.
   *
   * @param os Output stream object.
   * @param jd Julian date object.
   * @return the output stream.
   */
  friend std::ostream& operator<< (std::ostream &os, const JulianDate &jd);

private:
  /**
   * @brief Check if it is a leap year.
   *
   * This function assumes that the year is in the [1992, 2099] range.
   *
   * @param year the year to check.
   * @return a boolean indicating whether it is a leap year.
   */
  static bool IsLeapYear (uint32_t year);

  /**
   * @brief The difference between TAI and UTC time systems.
   * @param daysInPosix Full days since Unix/POSIX epoch.
   * @return the difference between TAI and UTC (number of leap seconds).
   */
  static Time TaiMinusUtc (uint32_t daysInPosix);

  /**
   * @brief Retrieve the DUT1 (UT1 - UTC) value for a given day.
   * @param daysInPosix number of full days since Unix/POSIX epoch.
   * @return the DUT1 measured/predicted as of IERS Bulletin A.
   */
  static Time Dut1 (uint32_t daysInPosix);

  /**
   * @brief Convert Julian days into Gregorian calendar.
   * @param days Full Julian days.
   * @param ms_day Milliseconds of the day (since midnight).
   * @return a DateTime struct with the date.
   */
  static DateTime GregorianDate (
    uint32_t days, uint32_t ms_day
  );

  /**
   * @brief Returns the offset from UTC for a given time.
   * @param daysInPosix Full days since Unix/POSIX epoch.
   * @param ts Time system from.
   * @return the offset from UTC.
   */
  static Time OffsetFromUtc (
    uint32_t daysInPosix, TimeSystem ts
  );

  /**
   * @brief Returns the offset to UTC for a given time.
   * @param daysInPosix Full days since Unix/POSIX epoch.
   * @param ms_day Milliseconds of the day (since midnight).
   * @param ts Time system to.
   * @return the offset to UTC.
   */
  static Time OffsetToUtc (
    uint32_t daysInPosix, uint32_t ms_day, TimeSystem ts
  );

  /**
   * @brief Convert into Gregorian calendar.
   * @return a DateTime struct with the date.
   */
  DateTime GregorianDate (void) const;

  /**
   * @brief Returns the offset from UTC.
   *
   * Returns the offset from UTC to its internal time system as of its internal
   * time.
   *
   * @return the offset from UTC.
   */
  Time OffsetFromUtc (void) const;

  /**
   * @brief Returns the offset to UTC.
   *
   * Returns the offset to UTC from its internal time system as of its internal
   * time.
   *
   * @return the offset to UTC.
   */
  Time OffsetToUtc (void) const;

  uint32_t m_days;                                //!< full days since epoch.
  uint32_t m_ms_day;                              //!< milliseconds of the day.
  TimeSystem m_time_scale;                        //!< external time system.
};

}

#endif /* JULIANDATE_H */
