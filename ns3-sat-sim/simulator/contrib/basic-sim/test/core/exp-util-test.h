/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-simulation.h"
#include "ns3/test.h"
#include "../test-helpers.h"

using namespace ns3;

////////////////////////////////////////////////////////////////////////////////////////

class ExpUtilStringsTestCase : public TestCase {
public:
    ExpUtilStringsTestCase() : TestCase("exp-util strings") {};

    void DoRun() {

        // Format string
        ASSERT_EQUAL(format_string("%d", 65), "65");
        ASSERT_EQUAL(format_string("%s", ""), "");
        ASSERT_EQUAL(format_string("%s %s", "abc", "def"), "abc def");
        ASSERT_EQUAL(format_string("%s%d", "abc", 4636), "abc4636");

        // Trimming
        ASSERT_EQUAL(trim("abc"), "abc");
        ASSERT_EQUAL(trim(" abc"), "abc");
        ASSERT_EQUAL(trim("abc "), "abc");
        ASSERT_EQUAL(trim("  abc "), "abc");
        ASSERT_EQUAL(trim("\t abc"), "abc");
        ASSERT_EQUAL(left_trim("abc "), "abc ");
        ASSERT_EQUAL(right_trim(" abc"), " abc");

        // Quote removal
        ASSERT_EQUAL(remove_start_end_double_quote_if_present("abc"), "abc");
        ASSERT_EQUAL(remove_start_end_double_quote_if_present("\"abc"), "\"abc");
        ASSERT_EQUAL(remove_start_end_double_quote_if_present("abc\""), "abc\"");
        ASSERT_EQUAL(remove_start_end_double_quote_if_present("\"abc\""), "abc");
        ASSERT_EQUAL(remove_start_end_double_quote_if_present("\"abc\""), "abc");
        ASSERT_EQUAL(remove_start_end_double_quote_if_present("\" abc\""), " abc");
        ASSERT_EQUAL(remove_start_end_double_quote_if_present("\"\""), "");
        ASSERT_EQUAL(remove_start_end_double_quote_if_present("\""), "\"");

        // Starting / ending
        ASSERT_TRUE(starts_with("abc", "abc"));
        ASSERT_TRUE(starts_with("abc", "ab"));
        ASSERT_FALSE(starts_with("abc", "bc"));
        ASSERT_TRUE(starts_with("abc", ""));
        ASSERT_TRUE(ends_with("abc", "abc"));
        ASSERT_FALSE(ends_with("abc", "ab"));
        ASSERT_TRUE(ends_with("abc", "bc"));
        ASSERT_TRUE(ends_with("abc", ""));

        // Split string
        std::string str = ",a,,b,c,d;e,";
        std::vector <std::string> spl = split_string(str, ",");
        ASSERT_EQUAL(spl.size(), 7);
        ASSERT_EQUAL(spl[0], "");
        ASSERT_EQUAL(spl[1], "a");
        ASSERT_EQUAL(spl[2], "");
        ASSERT_EQUAL(spl[3], "b");
        ASSERT_EQUAL(spl[4], "c");
        ASSERT_EQUAL(spl[5], "d;e");
        ASSERT_EQUAL(spl[6], "");
        spl = split_string(str, ";");
        ASSERT_EQUAL(spl.size(), 2);
        ASSERT_EQUAL(spl[0], ",a,,b,c,d");
        ASSERT_EQUAL(spl[1], "e,");
        spl = split_string(str, ",,");
        ASSERT_EQUAL(spl.size(), 2);
        ASSERT_EQUAL(spl[0], ",a");
        ASSERT_EQUAL(spl[1], "b,c,d;e,");
        str = "";
        spl = split_string(str, ",");
        ASSERT_EQUAL(spl.size(), 1);
        ASSERT_EQUAL(spl[0], "");

    }
};

class ExpUtilParsingTestCase : public TestCase {
public:
    ExpUtilParsingTestCase() : TestCase("exp-util parsing") {};

    void DoRun() {

        // Int64
        ASSERT_EQUAL(parse_int64("0"), 0);
        ASSERT_EQUAL(parse_int64("1"), 1);
        ASSERT_EQUAL(parse_int64("-1"), -1);
        ASSERT_EQUAL(parse_int64("5848484"), 5848484);
        ASSERT_EQUAL(parse_int64("-9"), -9);
        ASSERT_EXCEPTION(parse_int64("3.5"));
        ASSERT_EXCEPTION(parse_int64("-0.00001"));
        ASSERT_EXCEPTION(parse_int64("-8888.0"));
        ASSERT_EXCEPTION(parse_int64("5e-1"));

        // Positive in64
        ASSERT_EQUAL(parse_positive_int64("1"), 1);
        ASSERT_EQUAL(parse_positive_int64("0"), 0);
        ASSERT_EXCEPTION(parse_positive_int64(""));
        ASSERT_EXCEPTION(parse_positive_int64("-6"));
        ASSERT_EXCEPTION(parse_positive_int64("3.5"));

        // Positive int64 >= 1
        ASSERT_EQUAL(parse_geq_one_int64("345"), 345);
        ASSERT_EQUAL(parse_geq_one_int64("1"), 1);
        ASSERT_EXCEPTION(parse_geq_one_int64("0"));
        ASSERT_EXCEPTION(parse_geq_one_int64("1.1"));

        // Double
        ASSERT_EQUAL(parse_double("0.0"), 0.0);
        ASSERT_EQUAL(parse_double("-0.0"), 0.0);
        ASSERT_EQUAL(parse_double("6.89"), 6.89);
        ASSERT_EQUAL(parse_double("99"), 99.0);
        ASSERT_EQUAL(parse_double("-0.00001"), -0.00001);
        ASSERT_EQUAL(parse_double("-8888"), -8888);
        ASSERT_EQUAL(parse_double("5e-1"), 0.5);
        ASSERT_EXCEPTION(parse_double("abc"));
        ASSERT_EXCEPTION(parse_double("58.2abc"));
        ASSERT_EXCEPTION(parse_double("a58.2"));
        ASSERT_EXCEPTION(parse_double("a58.289.a"));

        // Positive double
        ASSERT_EQUAL(parse_positive_double("0.0"), 0.0);
        ASSERT_EQUAL(parse_positive_double("-0.0"), 0.0);
        ASSERT_EQUAL(parse_positive_double("6.89"), 6.89);
        ASSERT_EQUAL(parse_positive_double("99"), 99.0);
        ASSERT_EXCEPTION(parse_positive_double("-0.00001"));
        ASSERT_EXCEPTION(parse_positive_double("-8888"));

        // Positive double in [0, 1]
        ASSERT_EQUAL(parse_double_between_zero_and_one("0.0"), 0.0);
        ASSERT_EQUAL(parse_double_between_zero_and_one("0.6"), 0.6);
        ASSERT_EQUAL(parse_double_between_zero_and_one("1.0"), 1.0);
        ASSERT_EXCEPTION(parse_double_between_zero_and_one("-0.00001"));
        ASSERT_EXCEPTION(parse_double_between_zero_and_one("1.00001"));

        // Boolean
        ASSERT_TRUE(parse_boolean("1"));
        ASSERT_TRUE(parse_boolean("true"));
        ASSERT_FALSE(parse_boolean("0"));
        ASSERT_FALSE(parse_boolean("false"));
        ASSERT_EXCEPTION(parse_boolean("y"));
        ASSERT_EXCEPTION(parse_boolean("n"));
        ASSERT_EXCEPTION(parse_boolean("yes"));
        ASSERT_EXCEPTION(parse_boolean("no"));
        ASSERT_EXCEPTION(parse_boolean("falselytrue"));

        // Set string
        std::set <std::string> set_a = parse_set_string("set(a, b, 1245, , 77)");
        ASSERT_EQUAL(set_a.size(), 5);
        ASSERT_EXCEPTION(parse_set_string("seta, b, 1245, , 77)"));
        ASSERT_EXCEPTION(parse_set_string("a, b, c"));
        ASSERT_EXCEPTION(parse_set_string("set(a, b, c"));
        ASSERT_EXCEPTION(parse_set_string("set(a, a)"));

        // Set positive int64
        std::set<int64_t> set_b = parse_set_positive_int64("set(100, 0, 1, 56)");
        ASSERT_EQUAL(set_b.size(), 4);
        ASSERT_EXCEPTION(parse_set_positive_int64("set(-1, 5, 6)"));
        ASSERT_EXCEPTION(parse_set_positive_int64("set(0, 5, 0.5)"));
        ASSERT_EXCEPTION(parse_set_positive_int64("set(3, 3)"));
        ASSERT_EXCEPTION(parse_set_positive_int64("set(3, 03)"));

        // List string
        std::vector<std::string> list_a = parse_list_string("list(a, b, c, 77, e)");
        ASSERT_EQUAL(list_a.size(), 5);
        list_a = parse_list_string("list()");
        ASSERT_EQUAL(list_a.size(), 0);
        list_a = parse_list_string("list(123)");
        ASSERT_EQUAL(list_a.size(), 1);
        ASSERT_EXCEPTION(parse_list_string("lista,b,c)"));
        ASSERT_EXCEPTION(parse_list_string("c)"));
        ASSERT_EXCEPTION(parse_list_string("list(a, b, c"));

        // List positive int64
        std::vector<int64_t> list_b = parse_list_positive_int64("list(3, 5, 0, 24, 24)");
        ASSERT_EQUAL(list_b.size(), 5);
        list_b = parse_list_positive_int64("list()");
        ASSERT_EQUAL(list_b.size(), 0);
        list_b = parse_list_positive_int64("list(0)");
        ASSERT_EQUAL(list_b.size(), 1);
        ASSERT_EXCEPTION(parse_list_positive_int64("list(-1,0,3,5,225)"));
        ASSERT_EXCEPTION(parse_list_positive_int64("list1,2,3)"));
        ASSERT_EXCEPTION(parse_list_positive_int64("c)"));
        ASSERT_EXCEPTION(parse_list_positive_int64("list(a, 324)"));
        ASSERT_EXCEPTION(parse_list_positive_int64("list(3.5)"));

        // Map string
        std::vector<std::pair<std::string, std::string>> map_a;

        map_a = parse_map_string("map()");
        ASSERT_EQUAL(map_a.size(), 0);

        map_a = parse_map_string("map(a: 3abc)");
        ASSERT_EQUAL(map_a.size(), 1);
        ASSERT_PAIR_EQUAL(map_a[0], std::make_pair(std::string("a"), std::string("3abc")));

        map_a = parse_map_string("map(b: 8,a: 9;)");
        ASSERT_EQUAL(map_a.size(), 2);
        ASSERT_PAIR_EQUAL(map_a[0], std::make_pair(std::string("b"), std::string("8")));
        ASSERT_PAIR_EQUAL(map_a[1], std::make_pair(std::string("a"), std::string("9;")));

        map_a = parse_map_string("map(c: 99, 8: 9, a  f: 9,h:,:i)");
        ASSERT_EQUAL(map_a.size(), 5);
        ASSERT_PAIR_EQUAL(map_a[0], std::make_pair(std::string("c"), std::string("99")));
        ASSERT_PAIR_EQUAL(map_a[1], std::make_pair(std::string("8"), std::string("9")));
        ASSERT_PAIR_EQUAL(map_a[2], std::make_pair(std::string("a  f"), std::string("9")));
        ASSERT_PAIR_EQUAL(map_a[3], std::make_pair(std::string("h"), std::string("")));
        ASSERT_PAIR_EQUAL(map_a[4], std::make_pair(std::string(""), std::string("i")));

        map_a = parse_map_string("map(i:, :i)");
        ASSERT_EQUAL(map_a.size(), 2);
        ASSERT_PAIR_EQUAL(map_a[0], std::make_pair(std::string("i"), std::string("")));
        ASSERT_PAIR_EQUAL(map_a[1], std::make_pair(std::string(""), std::string("i")));

        map_a = parse_map_string("map(:)");
        ASSERT_EQUAL(map_a.size(), 1);
        ASSERT_PAIR_EQUAL(map_a[0], std::make_pair(std::string(""), std::string("")));

        ASSERT_EXCEPTION(parse_map_string("map"));
        ASSERT_EXCEPTION(parse_map_string("map("));
        ASSERT_EXCEPTION(parse_map_string("map)"));
        ASSERT_EXCEPTION(parse_map_string("map(a:b,a:d)"));
        ASSERT_EXCEPTION(parse_map_string("map(a :b,a:d)"));
        ASSERT_EXCEPTION(parse_map_string("map(a:b, a:d)"));
        ASSERT_EXCEPTION(parse_map_string("map( a :b, a:d)"));
        ASSERT_EXCEPTION(parse_map_string("map( a :b, a:b)"));
        ASSERT_EXCEPTION(parse_map_string("map(:i,:i)"));
        ASSERT_EXCEPTION(parse_map_string("map(i:,i:)"));
        ASSERT_EXCEPTION(parse_map_string("map(:,:)"));

    }
};

class ExpUtilSetsTestCase : public TestCase {
public:
    ExpUtilSetsTestCase() : TestCase("exp-util sets") {};

    void DoRun() {
        bool caught = false;

        std::set <int64_t> a;
        a.insert(5);
        a.insert(9);
        a.insert(12);
        std::set <int64_t> b;
        b.insert(9);
        b.insert(3);
        b.insert(9999);
        all_items_are_less_than(a, 13);
        caught = false;
        try {
            all_items_are_less_than(a, 12);
        } catch (std::exception &e) {
            caught = true;
        }
        ASSERT_TRUE(caught);
        ASSERT_EQUAL(direct_set_intersection(a, b).size(), 1);
        ASSERT_EQUAL(direct_set_union(a, b).size(), 5);

    }
};

class ExpUtilConfigurationReadingTestCase : public TestCase {
public:
    ExpUtilConfigurationReadingTestCase() : TestCase("exp-util configuration-reading") {};

    void DoRun() {

        // Normal
        std::ofstream config_file;
        config_file.open("temp.file");
        config_file << "a=b" << std::endl;
        config_file << "c=" << std::endl;
        config_file << "#x=y" << std::endl;
        config_file << "" << std::endl;
        config_file << "7=9" << std::endl;
        config_file.close();
        std::map <std::string, std::string> config = read_config("temp.file");
        ASSERT_EQUAL(config.size(), 3);
        ASSERT_EQUAL(get_param_or_fail("a", config), "b");
        ASSERT_EQUAL(get_param_or_fail("c", config), "");
        ASSERT_EQUAL(get_param_or_default("x", "abcd", config), "abcd");
        ASSERT_EQUAL(get_param_or_default("c", "abcd", config), "");
        ASSERT_EQUAL(get_param_or_fail("7", config), "9");
        ASSERT_EXCEPTION(get_param_or_fail("8", config));
        remove_file_if_exists("temp.file");

        // Empty
        config_file.open("temp.file");
        config_file << "" << std::endl;
        config_file.close();
        config = read_config("temp.file");
        ASSERT_EQUAL(config.size(), 0);
        remove_file_if_exists("temp.file");

        // Duplicate key
        config_file.open("temp.file");
        config_file << "a=b" << std::endl;
        config_file << "a =b" << std::endl;
        config_file.close();
        ASSERT_EXCEPTION(read_config("temp.file"));

        // Non-existent file
        remove_file_if_exists("temp.file");
        ASSERT_EXCEPTION(read_config("temp.file"));

    }
};

class ExpUtilUnitConversionTestCase : public TestCase {
public:
    ExpUtilUnitConversionTestCase() : TestCase("exp-util unit-conversion") {};

    void DoRun() {
        ASSERT_EQUAL_APPROX(byte_to_megabit(10000000), 80, 0.000001);
        ASSERT_EQUAL_APPROX(nanosec_to_sec(10000000), 0.01, 0.000001);
        ASSERT_EQUAL_APPROX(nanosec_to_millisec(10000000), 10.0, 0.000001);
        ASSERT_EQUAL_APPROX(nanosec_to_microsec(10000000), 10000.0, 0.000001);
    }
};

class ExpUtilFileSystemTestCase : public TestCase {
public:
    ExpUtilFileSystemTestCase() : TestCase("exp-util file-system") {};

    void DoRun() {

        // File system: file
        remove_file_if_exists("temp.file");
        ASSERT_FALSE(file_exists("temp.file"));
        std::ofstream the_file;
        the_file.open ("temp.file");
        the_file << "Content" << std::endl;
        the_file << "Other" << std::endl;
        the_file.close();
        std::vector<std::string> lines = read_file_direct("temp.file");
        ASSERT_EQUAL(lines.size(), 2);
        ASSERT_EQUAL(trim(lines[0]), "Content");
        ASSERT_EQUAL(trim(lines[1]), "Other");
        ASSERT_TRUE(file_exists("temp.file"));
        remove_file_if_exists("temp.file");
        ASSERT_FALSE(file_exists("temp.file"));

        // File does not exist
        ASSERT_EXCEPTION(read_file_direct("temp.file"));

        // File system: dir
        mkdir_if_not_exists("temp.dir");
        ASSERT_TRUE(dir_exists("temp.dir"));
        remove_dir_if_exists("temp.dir");
        ASSERT_FALSE(dir_exists("temp.dir"));

    }
};

////////////////////////////////////////////////////////////////////////////////////////
