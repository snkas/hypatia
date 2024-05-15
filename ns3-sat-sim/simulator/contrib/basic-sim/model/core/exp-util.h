/**
 * Copyright (c) 2020 snkas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#ifndef EXP_UTIL_H
#define EXP_UTIL_H

#include <string>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <cinttypes>
#include <vector>
#include <set>
#include <string>
#include <regex>
#include <cstring>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>

/**
 * Direct format() for a string in one line (e.g., for exceptions).
 *
 * Licensed under CC0 1.0.
 *
 * Source:
 * https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf/26221725#26221725
 *
 * @tparam Args         Argument types (e.g., char*, int)
 * @param format        String format (e.g., "%s%d")
 * @param args          Argument (e.g., "test", 8)
 *
 * @return Formatted string
 */
template<typename ... Args>
std::string format_string( const std::string& format, Args ... args ) {
    size_t size = snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    std::unique_ptr<char[]> buf( new char[ size ] );
    snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}

// String manipulations
std::string right_trim(std::string s);
std::string left_trim(std::string s);
std::string trim(std::string s);
bool ends_with(const std::string& str, const std::string& suffix);
bool starts_with(const std::string& str, const std::string& prefix);
std::string remove_start_end_double_quote_if_present(std::string s);
std::vector<std::string> split_string(std::string line, const std::string delimiter);
std::vector<std::string> split_string(std::string line, const std::string delimiter, size_t expected);

// Parsing values
int64_t parse_int64(const std::string& str);
int64_t parse_positive_int64(const std::string& str);
int64_t parse_geq_one_int64(const std::string& str);
double parse_double(const std::string& str);
double parse_positive_double(const std::string& str);;
double parse_double_between_zero_and_one(const std::string& str);
bool parse_boolean(const std::string& str);
std::set<std::string> parse_set_string(const std::string line);
std::set<int64_t> parse_set_positive_int64(const std::string line);
std::vector<std::string> parse_list_string(const std::string line);
std::vector<int64_t> parse_list_positive_int64(const std::string line);
std::vector<std::pair<std::string, std::string>> parse_map_string(const std::string str);

// Sets
void all_items_are_less_than(const std::set<int64_t>& s, const int64_t number);
std::set<int64_t> direct_set_intersection(const std::set<int64_t>& s1, const std::set<int64_t>& s2);
std::set<int64_t> direct_set_union(const std::set<int64_t>& s1, const std::set<int64_t>& s2);

// Configuration reading
std::map<std::string, std::string> read_config(const std::string& filename);
std::string get_param_or_fail(const std::string& param_key, std::map<std::string, std::string>& config);
std::string get_param_or_default(const std::string& param_key, std::string default_value, std::map<std::string, std::string>& config);

// Unit conversion
double byte_to_megabit(int64_t num_bytes);
double nanosec_to_sec(int64_t num_seconds);
double nanosec_to_millisec(int64_t num_seconds);
double nanosec_to_microsec(int64_t num_seconds);

// File system
bool file_exists(std::string filename);
void remove_file_if_exists(std::string filename);
bool dir_exists(std::string dirname);
void remove_dir_if_exists(std::string dirname);
void mkdir_if_not_exists(std::string dirname);
std::vector<std::string> read_file_direct(const std::string& filename);

#endif //EXP_UTIL_H
