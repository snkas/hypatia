/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2020 ETH Zurich
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
 * Author: Simon
 */

#include "log-update-helper.h"

LogUpdateHelper::LogUpdateHelper() {
    m_last_update_value = -1;
    m_interval_alpha_value = -1;
    m_last_interval_check_time = -1;
    m_interval_alpha_left_time = -1;
    m_interval_beta_left_time = -1;
}

void LogUpdateHelper::Update(int64_t time, int64_t value) {

    // Time must be positive
    if (time < 0) {
        throw std::invalid_argument("Negative time is not permitted.");
    }

    // No logging in the past
    if (time < m_last_interval_check_time) {
        throw std::invalid_argument("Cannot log a value for a time in the past.");
    }

    // Check if there is a new interval once per time moment
    if (time > m_last_interval_check_time) {

        // If no first interval is yet started, start interval alpha
        if (m_interval_alpha_left_time == -1) {
            m_interval_alpha_left_time = time;
        } else {

            // If no second interval is yet started, start interval beta
            if (m_interval_beta_left_time == -1) {
                m_interval_beta_left_time = time;
                m_interval_alpha_value = m_last_update_value;

            // If both interval alpha and beta has been started, the value of interval beta
            // is determined, as such interval alpha can be pushed to the log if it is different
            } else {

                // The value of interval beta has now been determined
                int64_t m_interval_beta_value = m_last_update_value;

                // If interval alpha and beta have the same value, merge
                if (m_interval_alpha_value == m_interval_beta_value) {
                    m_interval_beta_left_time = time;

                } else { // If not, save

                    // Save the alpha interval
                    m_log_time_value.push_back(std::make_tuple(m_interval_alpha_left_time, m_interval_beta_left_time, m_interval_alpha_value));

                    // And make beta interval become alpha
                    m_interval_alpha_left_time = m_interval_beta_left_time;
                    m_interval_alpha_value = m_interval_beta_value;
                    m_interval_beta_left_time = time;

                }

            }

        }

        // Check once per time moment for new intervals
        m_last_interval_check_time = time;

    }

    // Continuously update it
    m_last_update_value = value;
    
}

const std::vector<std::tuple<int64_t, int64_t, int64_t>>& LogUpdateHelper::Finalize(int64_t time) {

    // Time must be positive
    if (time < 0) {
        throw std::invalid_argument("Negative time is not permitted.");
    }

    // No logging in the past
    if (time < m_last_interval_check_time) {
        throw std::invalid_argument("Cannot finalize at a time in the past.");
    }

    // Only have for one time moment, as such only alpha
    if (m_interval_alpha_left_time != -1 && m_interval_beta_left_time == -1) {
        if (m_interval_alpha_left_time != time) {
            m_log_time_value.push_back(std::make_tuple(m_interval_alpha_left_time, time, m_last_update_value));
        }

    // If there is a beta interval, save the (guaranteed non-empty) alpha interval and the beta interval
    } else if (m_interval_alpha_left_time != -1 && m_interval_beta_left_time != -1) {

        // If the last two intervals can be merged
        if (m_interval_alpha_value == m_last_update_value) {
            m_log_time_value.push_back(std::make_tuple(m_interval_alpha_left_time, time, m_interval_alpha_value));

        // If the last two intervals have different values, save separately
        } else {
            m_log_time_value.push_back(std::make_tuple(m_interval_alpha_left_time, m_interval_beta_left_time, m_interval_alpha_value));
            if (m_interval_beta_left_time != time) {
                m_log_time_value.push_back(std::make_tuple(m_interval_beta_left_time, time, m_last_update_value));
            }
        }

    }

    // Return final resulting log
    return m_log_time_value;
}
