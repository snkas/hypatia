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

#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <string>
#include <vector>

/**
 * @brief The EopParameters struct for Earth Orientation Parameters.
 *
 * Stores polar motion values (xp, yp), DUT1 (UT1-UTC), and length-of-day (LOD).
 */
struct EopParameters {
  double xp, yp, dut1, lod;
};

// safe readline for reading files created in a platform that uses different
// newline characters
// Mac OS: '\r', Linux: '\n', Windows: '\r\n'
std::istream& ReadLine(std::istream& in, std::string& str) {
  bool done = false;
  bool car_ret = false;

  str.clear();

  while(!done) {
    char c = static_cast<char>(in.peek());

    switch(c) {
      case '\n': in.ignore(1); return in;
      case '\r': in.ignore(1); car_ret = true; break;
      case EOF: return in;
      default:
        // if this is a Mac OS file
        if(car_ret)
          return in;

        str += in.get();
    }
  }

  return in;
}

void PrintCopyright(std::ofstream &f) {
  f <<
"/* -*-  Mode: C++; c-file-style: \"gnu\"; indent-tabs-mode:nil; -*- */\n"
"/*\n"
" * Copyright (c) 2016 INESC TEC\n"
" *\n"
" * This program is free software; you can redistribute it and/or modify\n"
" * it under the terms of the GNU General Public License version 2 as\n"
" * published by the Free Software Foundation;\n"
" *\n"
" * This program is distributed in the hope that it will be useful,\n"
" * but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
" * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
" * GNU General Public License for more details.\n"
" *\n"
" * You should have received a copy of the GNU General Public License\n"
" * along with this program; if not, write to the Free Software\n"
" * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n"
" *\n"
" * Author: Pedro Silva  <pmms@inesctec.pt>\n"
" *\n"
" */" << std::endl << std::endl;
}

void PrintAutoGenInfo(std::ofstream& f) {
  f << "/*\n" <<
    " * IMPORTANT: This file was generated automatically. Do not change it!" <<
    "\n" <<
    " *            Instead, update files 'tai-utc.dat', and 'finals.data' on" <<
    "\n" <<
    " *            src/satellite/model/data/ folder using the latest files\n" <<
    " *            provided at:\n" <<
    " * http://www.usno.navy.mil/USNO/earth-orientation/eo-info/general/bullc"
    << "\n * and \n" <<
    " * http://www.usno.navy.mil/USNO/earth-orientation/eo-products/weekly or\n"
    " * http://www.iers.org/IERS/EN/DataProducts/EarthOrientationData/eop.html"
    "\n */"
    << std::endl;
}

void PrintStartNamespace(std::ofstream& f) {
  f << "namespace ns3 {\n" << std::endl;
}

void PrintEndNamespace(std::ofstream& f) {
  f << "}\n" << std::endl;
}

void PrintIncludes(std::ofstream& f, const std::string& type) {
  if(type == "header") {
  }
  else {
    f << "#include \"iers-data.h\"" << std::endl << std::endl;
  }

  f << "#include <stdint.h>\n" << "#include <vector>\n"
    << std::endl;
}

void PrintLeapSeconds(
  std::ofstream& f, const std::string& type, const std::string& visibility,
  const std::vector<uint32_t>& leap
)
{
  if(type == "header") {
    if(visibility == "public")
      f << "  static const uint32_t BaseLeapSeconds;" << std::endl
        << "  static const uint32_t CurLeapSeconds;" << std::endl
        << "  static const std::vector<uint32_t> LeapSeconds;" << std::endl;
    else
      f << "  static const uint32_t __leap_secs[];" << std::endl;
  }
  else {
    f << "const uint32_t IersData::BaseLeapSeconds = 10;" << std::endl
      << "const uint32_t IersData::CurLeapSeconds = " << leap.size()+10
      << ";" << std::endl
      << "const uint32_t IersData::__leap_secs[] = {\n" << "  " << leap[0];

    for(uint32_t i = 1; i < leap.size(); ++i) {
      f << ", ";

      if((i & 0x07) == 0)
        f << "\n  ";

      f << leap[i];
    }

    f << "\n};\n"
      << "// Modified Julian Date for when leap seconds were added in UTC\n"
      << "const std::vector<uint32_t> IersData::LeapSeconds(\n"
      << "  __leap_secs, __leap_secs + " << leap.size() << "\n"
      << ");\n" << std::endl << std::endl;
  }
}

void PrintEopParams(std::ofstream& f, const EopParameters& eop) {
  f << "{" << eop.xp << "," << eop.yp << "," << eop.dut1 << "," << eop.lod
    << "}";
}

void PrintEopParameters(
  std::ofstream& f, const std::string& type, const std::string& visibility,
  const std::vector<EopParameters>& eop
)
{
  if(type == "header") {
    if(visibility == "public")
      f << "  struct EopParameters {\n"
        << "    // radians, radians, seconds, milliseconds\n"
        << "    double xp, yp, dut1, lod;\n"
        << "  };\n\n"
        << "  static const std::vector<EopParameters> EopValues;"
        << std::endl;
    else
      f << "  static const EopParameters __eop_params[];" << std::endl;
  }
  else {
    f << "const IersData::EopParameters IersData::__eop_params[] = {\n" << "  ";
    PrintEopParams(f, eop[0]);

    for(uint32_t i = 1; i < eop.size(); ++i) {
      f << ", ";

      if((i & 0x01) == 0)
        f << "\n  ";

      PrintEopParams(f, eop[i]);
    }

    f << "\n};\n"
      << "// Daily EOP parameters since 01 Jan 1992 UTC\n"
      << "const std::vector<IersData::EopParameters> IersData::EopValues(\n"
      << "  __eop_params, __eop_params + " << eop.size() << "\n"
      << ");\n" << std::endl << std::endl;
  }
}

std::vector<uint32_t> ReadLeapSeconds(std::ifstream& f) {
  std::vector<uint32_t> v;
  std::string line;
  uint32_t linenum = 0;

  while(ReadLine(f, line)) {
    std::istringstream iss(line);
    uint32_t jd;

    // ignore the first 14 lines
    if(++linenum <= 14)
      continue;

    if(line.empty())
      break;

    iss.ignore(line.size(), ' ');                               // ignore year
    iss.ignore(line.size(), ' ');                               // ignore month
    iss.ignore(line.size(), '=');                               // ignore day
    iss.ignore(line.size(), ' ');                               // ignore =JD

    iss >> jd;

    // time relative to Unix epoch for when leap second was introduced
    v.push_back(jd - 2440587);
  }

  return v;
}

std::vector<EopParameters> ReadEopParameters(std::ifstream& f) {
  std::vector<EopParameters> v;
  std::string line;

  while(ReadLine(f, line)) {
    const double ArcSec2Rad = 4.8481368110954e-06;
    std::istringstream iss;
    EopParameters eop;
    double dev_null;

    // if there are no more valid parameters to be read
    if(line.empty() || line.size() < 185 || line[16] == ' ')
      break;

    iss.str(line.substr(18));                           // move to polar motion
    iss >> eop.xp;                                      // read polar motion x
    iss >> dev_null;                                    // ignore xp error
    iss >> eop.yp;                                      // read polar motion y

    iss.str(line.substr(58));                           // move to DUT1
    iss >> eop.dut1;                                    // read DUT1
    iss >> dev_null;                                    // ignore DUT1 error

    iss.str(line.substr(79, 7));                        // just LOD field

    if(iss.str() == "       ")                          // LOD not filled
      eop.lod = 0;
    else                                                // LOD filled
      iss >> eop.lod;

    eop.xp *= ArcSec2Rad;                               // convert to radians
    eop.yp *= ArcSec2Rad;                               // convert to radians

    v.push_back(eop);
  }

  return v;
}

int main(int argc, char* argv[]) {
  const std::string usage = "<header | source> path";
  std::ofstream out;
  std::ifstream leap, eop;
  std::string filename, base_path = "src/satellite/model/data/";
  std::vector<uint32_t> leap_seconds;
  std::vector<EopParameters> eop_params;

  if(argc != 3) {
    std::cerr << "Usage: " << argv[0] << usage << std::endl;

    return -1;
  }

  std::string opt(argv[1]);
  std::string path(argv[2]);

  path = (path[path.size()-1] == '/' ? path : (path + "/"));

  if(opt != "header" && opt != "source") {
    std::cerr << "Usage: " << argv[0] << " " << usage << std::endl;

    return -1;
  }

  filename = path + (opt == "header" ? "iers-data.h" : "iers-data.cc");
  out.open(filename.c_str());

  if(!out.is_open()) {
    std::cerr << "Unable to create file '" << filename << "'" << std::endl;

    return -1;
  }

  leap.open((base_path + "tai-utc.dat").c_str());

  if(!leap.is_open()) {
    std::cerr << "Unable to open file 'tai-utc.dat'" << std::endl;

    out.close();

    return -1;
  }

  eop.open((base_path + "finals.data").c_str());

  if(!eop.is_open()) {
    std::cerr << "Unable to open file 'finals.data'" << std::endl;

    out.close();
    leap.close();

    return -1;
  }

  PrintCopyright(out);

  if(opt == "header")
    out << "#ifndef IERS_DATA_H" << std::endl
        << "#define IERS_DATA_H" << std::endl << std::endl;

  PrintIncludes(out, opt);
  PrintAutoGenInfo(out);
  PrintStartNamespace(out);

  leap_seconds = ReadLeapSeconds(leap);
  eop_params = ReadEopParameters(eop);

  if(opt == "header") {
    out << "class IersData {\n"
        << "public:\n";

    PrintEopParameters(out, opt, "public", eop_params);
    PrintLeapSeconds(out, opt, "public", leap_seconds);

    out << "\nprotected:\n";

    PrintEopParameters(out, opt, "protected", eop_params);
    PrintLeapSeconds(out, opt, "protected", leap_seconds);

    out << "};\n" << std::endl;
  }
  else if(opt  == "source") {
    PrintEopParameters(out, opt, "irrelevant", eop_params);
    PrintLeapSeconds(out, opt, "irrelevant", leap_seconds);
  }

  PrintEndNamespace(out);

  if(opt == "header")
    out << "#endif // IERS_DATA_H" << std::endl << std::endl;

  out.close();
  leap.close();
  eop.close();

  return 0;
}
