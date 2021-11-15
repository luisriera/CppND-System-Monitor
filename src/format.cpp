#include <string>
#include <sstream>
#include <iomanip>

#include "format.h"

using std::string;


string Format::ElapsedTime(long seconds) {
  std::stringstream time;
  int HH, MM, SS;

  HH = (seconds / 3600) % 100; // Number of hours returns to 0 after 99
  MM = (seconds / 60) % 60;
  SS = seconds % 60;
  time << std::setw(2) << std::setfill('0') << HH << ":"
       << std::setw(2) << std::setfill('0') << MM << ":"
       << std::setw(2) << std::setfill('0') << SS;
  return time.str();
}