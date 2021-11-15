#include "processor.h"

#include <armadillo>

#include "linux_parser.h"

float Processor::Utilization() {
long unsigned totalJiffiesStart = LinuxParser::Jiffies();
long unsigned activeJiffiesStart = LinuxParser::ActiveJiffies();

usleep(10000); // microseconds --> 10 milliseconds

long unsigned totalJiffiesEnd = LinuxParser::Jiffies();
long unsigned activeJiffiesEnd = LinuxParser::ActiveJiffies();

long totalDelta = totalJiffiesEnd - totalJiffiesStart;
long activeDelta = activeJiffiesEnd - activeJiffiesStart;

if (totalDelta != 0) {
  return activeDelta / (1.0f * totalDelta);  // float conversion
}
return 0.0;
}