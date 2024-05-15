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

#include "exp-util.h"

/**
 * Trim from end of string.
 *
 * @param s     Original string
 *
 * @return Right-trimmed string
 */
std::string right_trim(std::string s) {
    s.erase(s.find_last_not_of(" \t\n\r\f\v") + 1);
    return s;
}

/**
 * Trim from start of string.
 *
 * @param s     Original string
 *
 * @return Left-trimmed string
 */
std::string left_trim(std::string s) {
    s.erase(0, s.find_first_not_of(" \t\n\r\f\v"));
    return s;
}

/**
 * Trim from start and end of string.
 *
 * @param s     Original string
 *
 * @return Left-and-right-trimmed string
 */
std::string trim(std::string s) {
    return left_trim(right_trim(s));
}

/**
 * Check if the string ends with a suffix.
 *
 * @param str       String
 * @param prefix    Prefix
 *
 * @return True iff string ends with that suffix
 */
bool ends_with(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

/**
 * Check if the string starts with a prefix.
 *
 * @param str       String
 * @param prefix    Prefix
 *
 * @return True iff string starts with that prefix
 */
bool starts_with(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

/**
 * Remove the start and end double quote (") from a string if they are both present.
 *
 * @param s     String with quotes (e.g., "abc")
 *
 * @return String without quotes (e.g., abc)
 */
std::string remove_start_end_double_quote_if_present(std::string s) {
    if (s.size() >= 2 && starts_with(s, "\"") && ends_with(s, "\"")) {
        return s.substr(1, s.size() - 2);
    } else {
        return s;
    }
}

/**
 * Split a string by the delimiter(s).
 *
 * @param line          Line (e.g., "a->b->c")
 * @param delimiter     Delimiter string (e.g., "->")
 *
 * @return Split vector (e.g., [a, b, c])
 */
std::vector<std::string> split_string(const std::string line, const std::string delimiter) {
    std::vector<std::string> result;
    std::string remainder = line;
    size_t idx = remainder.find(delimiter);
    while (idx != std::string::npos) {
        result.push_back(remainder.substr(0, idx));
        remainder = remainder.substr(idx + delimiter.size(), remainder.size());
        idx = remainder.find(delimiter);
    }
    result.push_back(remainder);
    return result;
}

/**
 * Split a string by the delimiter(s) and check that the split size is of expected size.
 * If it is not of expected size, throw an exception.
 *
 * @param line          Line (e.g., "a->b->c")
 * @param delimiter     Delimiter string (e.g., "->")
 * @param expected      Expected number (e.g., 3)
 *
 * @return Split vector (e.g., [a, b, c])
 */
std::vector<std::string> split_string(const std::string line, const std::string delimiter, size_t expected) {

    std::vector<std::string> the_split = split_string(line, delimiter);

    // It must match the expected split length, else throw exception
    if (the_split.size() != expected) {
        throw std::invalid_argument(
                format_string(
                        "String %s has a %s-split of %zu != %zu",
                        line.c_str(),
                        delimiter.c_str(),
                        the_split.size(),
                        expected
                )
        );
    }

    return the_split;
}

/**
 * Parse string into an int64, or throw an exception.
 *
 * @param str  Input string
 *
 * @return Int64
 */
int64_t parse_int64(const std::string& str) {

    // Parse using stoll -- rethrow argument to be a bit easier debuggable
    int64_t val;
    size_t i;
    try {
        val = std::stoll(str, &i);
    } catch (const std::invalid_argument&) {
        throw std::invalid_argument("Could not convert to int64: " + str);
    }

    // No remainder
    if (i != str.size()) {
        throw std::invalid_argument("Could not convert to int64: " + str);
    }

    return val;
}

/**
 * Parse string into a positive int64, or throw an exception.
 *
 * @param str  Input string
 *
 * @return Int64
 */
int64_t parse_positive_int64(const std::string& str) {
    int64_t val = parse_int64(str);
    if (val < 0) {
        throw std::invalid_argument(format_string("Negative int64 value not permitted: %" PRId64, val));
    }
    return val;
}

/**
 * Parse string into a int64 >= 1, or throw an exception.
 *
 * @param str  Input string
 *
 * @return Int64 >= 1
 */
int64_t parse_geq_one_int64(const std::string& str) {
    int64_t val = parse_positive_int64(str);
    if (val < 1) {
        throw std::invalid_argument(format_string("Value must be >= 1: %" PRId64, val));
    }
    return val;
}

/**
 * Parse string into a double, or throw an exception.
 *
 * @param str  Input string
 *
 * @return Double
 */
double parse_double(const std::string& str) {

    // Parse using stod -- rethrow argument to be a bit easier debuggable
    double val;
    size_t i;
    try {
        val = std::stod(str, &i);
    } catch (const std::invalid_argument&) {
        throw std::invalid_argument("Could not convert to double: " + str);
    }

    // No remainder
    if (i != str.size()) {
        throw std::invalid_argument("Could not convert to double: " + str);
    }

    return val;
}

/**
 * Parse string into a positive double, or throw an exception.
 *
 * @param str  Input string
 *
 * @return Positive double
 */
double parse_positive_double(const std::string& str) {
    double val = parse_double(str);
    if (val < 0) {
        throw std::invalid_argument(format_string("Negative double value not permitted: %f", val));
    }
    return val;
}

/**
 * Parse string into a double in the range of [0.0, 1.0], or throw an exception.
 *
 * @param str  Input string
 *
 * @return Double in the range of [0.0, 1.0]
 */
double parse_double_between_zero_and_one(const std::string& str) {
    double val = parse_double(str);
    if (val < 0 || val > 1) {
        throw std::invalid_argument(format_string("Double value must be [0, 1]: %f", val));
    }
    return val;
}

/**
 * Parse string into a boolean, or throw an exception.
 *
 * @param str  Input string (e.g., "true", "false", "0", "1")
 *
 * @return Boolean
 */
bool parse_boolean(const std::string& str) {
    if (str == "true" || str == "1") {
        return true;
    } else if (str == "false" || str == "0") {
        return false;
    } else {
        throw std::invalid_argument(format_string("Value could not be converted to bool: %s", str.c_str()));
    }
}

/**
 * Parse set string (i.e., set(...)) into a real set without whitespace.
 * If it is incorrect format, throw an exception.
 *
 * @param str  Input string (e.g., "set(a,b,c)")
 *
 * @return String set (e.g., {a, b, c})
 */
std::set<std::string> parse_set_string(const std::string str) {

    // Check set(...)
    if (!starts_with(str, "set(") || !ends_with(str, ")")) {
        throw std::invalid_argument(format_string( "Set %s is not encased in set(...)", str.c_str()));
    }
    std::string only_inside = str.substr(4, str.size() - 5);
    if (trim(only_inside).empty()) {
        std::set<std::string> final;
        return final;
    }
    std::vector<std::string> final = split_string(only_inside, ",");
    std::set<std::string> final_set;
    for (std::string& s : final) {
        final_set.insert(trim(s));
    }
    if (final.size() != final_set.size()) {
        throw std::invalid_argument(
                format_string(
                        "Set %s contains duplicates",
                        str.c_str()
                )
        );
    }
    return final_set;
}

/**
 * Parse string into a set of positive int64, or throw an exception.
 *
 * @param str  Input string
 *
 * @return Set of int64s
 */
std::set<int64_t> parse_set_positive_int64(const std::string str) {
    std::set<std::string> string_set = parse_set_string(str);
    std::set<int64_t> int64_set;
    for (std::string s : string_set) {
        int64_set.insert(parse_positive_int64(s));
    }
    if (string_set.size() != int64_set.size()) {
        throw std::invalid_argument(
                format_string(
                        "Set %s contains int64 duplicates",
                        str.c_str()
                )
        );
    }
    return int64_set;
}

/**
 * Parse list string (i.e., list(...)) into a real list without whitespace.
 * If it is incorrect format, throw an exception.
 *
 * @param str  Input string (e.g., "list(a,b,c)")
 *
 * @return List of strings (e.g., [a, b, c])
 */
std::vector<std::string> parse_list_string(const std::string str) {

    // Check list(...)
    if (!starts_with(str, "list(") || !ends_with(str, ")")) {
        throw std::invalid_argument(format_string( "List %s is not encased in list(...)", str.c_str()));
    }
    std::string only_inside = str.substr(5, str.size() - 6);
    if (trim(only_inside).empty()) {
        std::vector<std::string> final;
        return final;
    }
    std::vector<std::string> prelim_list = split_string(only_inside, ",");
    std::vector<std::string> final_list;
    for (std::string& s : prelim_list) {
        final_list.push_back(trim(s));
    }
    return final_list;
}

/**
 * Parse string into a list of positive int64, or throw an exception.
 *
 * @param str  Input string
 *
 * @return List of int64s
 */
std::vector<int64_t> parse_list_positive_int64(const std::string str) {
    std::vector<std::string> string_list = parse_list_string(str);
    std::vector<int64_t> int64_list;
    for (std::string s : string_list) {
        int64_list.push_back(parse_positive_int64(s));
    }
    return int64_list;
}

/**
 * Parse map string (i.e., map(...)) into key-value pair list (which can be converted into a map later on).
 * If it is incorrect format, throw an exception.
 *
 * @param str  Input string (e.g., "map(a:b,c:d)"
 *
 * @return List of key-value pairs (e.g., [(a, b), (c, d))])
 */
std::vector<std::pair<std::string, std::string>> parse_map_string(const std::string str) {

    // Check for encasing map(...)
    if (!starts_with(str, "map(") || !ends_with(str, ")")) {
        throw std::invalid_argument(format_string( "Map %s is not encased in map(...)", str.c_str()));
    }
    std::string only_inside = str.substr(4, str.size() - 5);

    // If it is empty, just return an empty vector
    if (trim(only_inside).empty()) {
        std::vector<std::pair<std::string, std::string>> final;
        return final;
    }

    // Split by comma to find all elements
    std::vector<std::string> comma_split_list = split_string(only_inside, ",");

    // Add the elements one-by-one
    std::vector<std::pair<std::string, std::string>> result;
    std::set<std::string> key_set;
    for (std::string& s : comma_split_list) {

        // Split by colon and insert into result
        std::vector<std::string> colon_split_list = split_string(s, ":", 2);
        result.push_back(std::make_pair(trim(colon_split_list[0]), trim(colon_split_list[1])));

        // Check if it was a duplicate key
        key_set.insert(trim(colon_split_list[0]));
        if (result.size() != key_set.size()) {
            throw std::invalid_argument(format_string("Duplicate key: %s", trim(colon_split_list[0]).c_str()));
        }

    }

    return result;
}

/**
 * Throw an exception if not all items are less than a number.
 *
 * @param s         Set of int64s
 * @param number    Upper bound (non-inclusive) that all items must abide by.
 */
void all_items_are_less_than(const std::set<int64_t>& s, const int64_t number) {
    for (int64_t val : s) {
        if (val >= number) {
            throw std::invalid_argument(
                    format_string(
                            "Value %" PRIu64 " > %" PRIu64 "",
                    val,
                    number
            )
            );
        }
    }
}

/**
 * Intersection of two sets.
 *
 * @param s1    Set A
 * @param s2    Set B
 *
 * @return Intersection of set A and B
 */
std::set<int64_t> direct_set_intersection(const std::set<int64_t>& s1, const std::set<int64_t>& s2) {
    std::set<int64_t> intersect;
    set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(intersect, intersect.begin()));
    return intersect;
}

/**
 * Union of two sets.
 *
 * @param s1    Set A
 * @param s2    Set B
 *
 * @return Union of set A and B
 */
std::set<int64_t> direct_set_union(const std::set<int64_t>& s1, const std::set<int64_t>& s2) {
    std::set<int64_t> s_union;
    set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(s_union, s_union.begin()));
    return s_union;
}

/**
 * Read a config properties file into a mapping.
 *
 * @param   filename    File name of the config.ini
 *
 * @return Key-value map with the configuration
*/
std::map<std::string, std::string> read_config(const std::string& filename) {

    // Where to put it in
    std::map<std::string, std::string> config;

    // Check that the file exists
    if (!file_exists(filename)) {
        throw std::runtime_error(format_string("File %s does not exist.", filename.c_str()));
    }

    // Open file
    std::string line;
    std::ifstream config_file(filename);
    if (config_file) {

        // Go over each line
        while (getline(config_file, line)) {

            // Skip commented lines
            if (trim(line).empty() || line.c_str()[0] == '#') {
                continue;
            }

            // Split on =
            std::vector<std::string> equals_split = split_string(line, "=", 2);

            // Trim whitespace
            std::string key = trim(equals_split[0]);
            std::string value = remove_start_end_double_quote_if_present(trim(equals_split[1]));

            // Check key does not exist yet
            if (config.find(key) != config.end()) {
                throw std::runtime_error(format_string("Duplicate property key: %s", key.c_str()));
            }

            // Save into config
            config[key] = value;

        }
        config_file.close();
    } else {
        throw std::runtime_error(format_string("File %s could not be read.", filename.c_str()));
    }

    return config;

}

/**
 * Get the parameter value or fail.
 *
 * @param param_key                 Parameter key
 * @param config                    Config parameters map
 *
 * @return Parameter value (if not present, an invalid_argument exception is thrown)
 */
std::string get_param_or_fail(const std::string& param_key, std::map<std::string, std::string>& config) {

    if (config.find(param_key) != config.end()) {
        return config[param_key];
    } else {
        throw std::invalid_argument(format_string("Necessary parameter '%s' is not set.", param_key.c_str()));
    }

}

/**
 * Get the parameter value or if not found, return default value.
 *
 * @param param_key                 Parameter key
 * @param default_value             Default value
 * @param config                    Config parameters map
 *
 * @return Parameter value (if not present, the default value is returned)
 */
std::string get_param_or_default(const std::string& param_key, std::string default_value, std::map<std::string, std::string>& config) {

    if (config.find(param_key) != config.end()) {
        return config[param_key];
    } else {
        return default_value;
    }

}

/**
 * Convert byte to megabit (Mbit).
 *
 * @param num_bytes     Number of bytes
 *
 * @return Number of megabit
 */
double byte_to_megabit(int64_t num_bytes) {
    return num_bytes * 8.0 / 1000.0 / 1000.0;
}

/**
 * Convert nanoseconds to seconds.
 *
 * @param num_nanosec   Number of nanoseconds
 *
 * @return Number of nanoseconds
 */
double nanosec_to_sec(int64_t num_nanosec) {
    return num_nanosec / 1e9;
}

/**
 * Convert nanoseconds to milliseconds.
 *
 * @param num_nanosec   Number of nanoseconds
 *
 * @return Number of milliseconds
 */
double nanosec_to_millisec(int64_t num_nanosec) {
    return num_nanosec / 1e6;
}

/**
 * Convert number of nanoseconds to microseconds.
 *
 * @param num_nanosec   Number of nanoseconds
 *
 * @return Number of nanoseconds
 */
double nanosec_to_microsec(int64_t num_nanosec) {
    return num_nanosec / 1e3;
}

/**
 * Check if a file exists.
 *
 * @param filename  Filename
 *
 * @return True iff the file exists
 */
bool file_exists(std::string filename) {
    struct stat st = {0};
    if (stat(filename.c_str(), &st) == 0) {
        return S_ISREG(st.st_mode);
    } else {
        return false;
    }
}

/**
 * Remove a file if it exists. If it exists but could not be removed, it throws an exception.
 *
 * @param filename  Filename to remove
 */
void remove_file_if_exists(std::string filename) {
    if (file_exists(filename)) {
        if (unlink(filename.c_str()) == -1) {
            throw std::runtime_error(format_string("File %s could not be removed.\n", filename.c_str()));
        }
    }
}

/**
 * Check if a directory exists.
 *
 * @param dirname  Directory name
 *
 * @return True iff the directory exists
 */
bool dir_exists(std::string dirname) {
    struct stat st = {0};
    if (stat(dirname.c_str(), &st) == 0) {
        return S_ISDIR(st.st_mode);
    } else {
        return false;
    }
}

/**
 * Remove a directory if it exists.
 *
 * @param dirname   Directory name
 */
void remove_dir_if_exists(std::string dirname) {
    if (dir_exists(dirname)) {
        if (rmdir(dirname.c_str()) == -1) {
            throw std::runtime_error(format_string("Directory %s could not be removed.\n", dirname.c_str()));
        }
    }
}

/**
 * Creates a directory if it does not exist.
 *
 * @param dirname  Directory name
 *
 * @return True iff the directory was created if it did not exist
 */
void mkdir_if_not_exists(std::string dirname) {
    if (!dir_exists(dirname)) {
        if (mkdir(dirname.c_str(), 0700) == -1) {
            throw std::runtime_error(format_string("Directory \"%s\" could not be made.", dirname.c_str()));
        }
    }
}

/**
 * Read the content of a file directly line-by-line into a vector of trimmed lines.
 *
 * @param   filename    File name
 *
 * @return Vector of trimmed lines
 */
std::vector<std::string> read_file_direct(const std::string& filename) {

    // Check that the file exists
    if (!file_exists(filename)) {
        throw std::runtime_error(format_string("File %s does not exist.", filename.c_str()));
    }

    // Storage
    std::vector<std::string> lines;

    // Open file
    std::string line;
    std::ifstream input_file(filename);
    if (input_file) {
        while (getline(input_file, line)) {
            lines.push_back(trim(line));
        }
        input_file.close();
    } else {
        throw std::runtime_error(format_string("File %s could not be read.", filename.c_str()));
    }

    return lines;

}
