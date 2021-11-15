#include "linux_parser.h"

#include <filesystem>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

using std::getline;
using std::ifstream;
using std::istringstream;
using std::stof;
using std::stol;
using std::string;
using std::to_string;
using std::vector;

template <typename T>
T valueByKey(string const &keyFilter, string const &filename) {
  string line, key;
  T value;

  ifstream stream(LinuxParser::kProcDirectory + filename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == keyFilter) {
          stream.close();
          return value;
        }
      }
    }
    stream.close();
  }
  return value;
};

template <typename T>
T valueFromFile(string const &filename) {
  string line;
  T value;

  ifstream stream(LinuxParser::kProcDirectory + filename);
  if (stream.is_open()) {
    getline(stream, line);
    istringstream linestream(line);
    linestream >> value;
    stream.close();
  }
  return value;
};

string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    getline(stream, line);
    istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// ********  BONUS: Update this to use std::filesystem *******
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  char name_pattern[] = "[0-9]*";  // dir name pattern

  for (const auto &entry : fs::directory_iterator(kProcDirectory)) {
    // check if entry is a dir
    if (entry.is_directory()) {
      // filesystem entry type to string
      auto new_entry = entry.path().filename().generic_string();

      // push entry to the vector if matches pattern
      if (regex_match(new_entry, std::regex(name_pattern))) {
        pids.push_back(std::stoi(new_entry));
      }
    }
  }
  return pids;
}
// ********************* BONUS DONE!! *******************

float LinuxParser::MemoryUtilization() {
  string line;
  string key, value, unit_;
  float memTotal_, memFree_;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value >> unit_;
      if (key == "MemTotal:") {
        memTotal_ = std::stof(value);
        continue;
      } else if (key == "MemFree:") {
        memFree_ = std::stof(value);
        break;
      }
    }
  }
  return (memTotal_ - memFree_) / memTotal_;
}

long LinuxParser::UpTime() {
  long upTime = valueFromFile<long>(kUptimeFilename);
  return upTime;
}

long LinuxParser::Jiffies() {
  vector<string> values = LinuxParser::CpuUtilization();
  vector<long> values_as_num(10, 0);
  long total = 0;
  vector<CPUStates> all_states = {kUser_,   kNice_,     kSystem_,  kIdle_,
                                  kIOwait_, kIRQ_,      kSoftIRQ_, kSteal_,
                                  kGuest_,  kGuestNice_};

  for (int i : all_states) {
    values_as_num[i] = stol(values[i]);
    total += values_as_num[i];
  };
  return total;
}

long LinuxParser::ActiveJiffies(int pid) {
  string line, value;
  vector<string> values;
  ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    getline(stream, line);
    istringstream linestream(line);
    int counter = 0;
    while (linestream >> value && counter <= 14) {
      values.push_back(value);
      counter++;
    }
  }
  return stol(values[13] + values[14]);
}

long LinuxParser::ActiveJiffies() {
  vector<string> jiffs = CpuUtilization();

  // adding active times
  return stol(jiffs[CPUStates::kUser_]) + stol(jiffs[CPUStates::kNice_]) +
         stol(jiffs[CPUStates::kSystem_]) + stol(jiffs[CPUStates::kIRQ_]) +
         stol(jiffs[CPUStates::kSoftIRQ_]) + stol(jiffs[CPUStates::kSteal_]);
}

long LinuxParser::IdleJiffies() {
  vector<string> jiffs = CpuUtilization();
  return stol(jiffs[CPUStates::kIdle_]) + stol(jiffs[CPUStates::kIOwait_]);
}

vector<string> LinuxParser::CpuUtilization() {
  string line, key, value;
  vector<string> values;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> key;
    while (linestream >> value) {
      values.push_back(value);
    };
  }
  return values;
}

int LinuxParser::TotalProcesses() {
  string key = "processes";
  int processes = valueByKey<int>(key, kStatFilename);
  return processes;
}

int LinuxParser::RunningProcesses() {
  string key = "procs_running";
  int processes = valueByKey<int>(key, kStatFilename);
  return processes;
}

string LinuxParser::Command(int pid) {
  string command =
      valueFromFile<string>(std::to_string(pid) + kCmdlineFilename);
  return command;
}

string LinuxParser::Ram(int pid) {
  string line, key, value;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);

  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "VmSize:") {
        return to_string(stol(value) / 1024);  // convert to MBs
      }
    }
  }
  return "0";
}

string LinuxParser::Uid(int pid) {
  string uid_ = valueByKey<string>(
      "Uid:", kProcDirectory + to_string(pid) + kStatusFilename);
  return uid_;
}

string LinuxParser::User(int pid) {
  string line, username_, shadow_, uid_;
  string pid_uid_ = Uid(pid);
  ifstream stream(kPasswordPath);
  const string delimiter = ":";
  ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> username_ >> shadow_ >> uid_) {
        if (pid_uid_ == LinuxParser::Uid(pid)) {
          return username_;
        }
      }
    }
  }
  return "no_known";
}

long LinuxParser::UpTime(int pid) {
  string line, value;
  ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  getline(filestream, line);
  std::istringstream linestream(line);
  if (filestream.is_open()) {
    for (int i = 0; i < 22; ++i) {
      linestream >> value;
    }
    return UpTime() - stol(value) / 100.0f;  // sysconf(_SC_CLK_TCK);
  }
  return 0;
}
