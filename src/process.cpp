#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) : pid_(pid){};


int Process::Pid()  { return pid_; }


float Process::CpuUtilization() {
  float act_time = float(LinuxParser::ActiveJiffies(Pid()) ) / 100.0f;
  float up_time =  LinuxParser::UpTime(Pid()) ;
  float cpu_time = (act_time / sysconf(_SC_CLK_TCK)) / up_time;
  return cpu_time;
}


string Process::Command() { return LinuxParser::Command(pid_); }

string Process::Ram() const { return LinuxParser::Ram(pid_); }

string Process::User() { return LinuxParser::User(pid_); }

long int Process::UpTime() { return LinuxParser::UpTime(pid_); }

bool Process::operator < (Process const& a) const {
  return stol(Ram()) < stol(a.Ram());
}