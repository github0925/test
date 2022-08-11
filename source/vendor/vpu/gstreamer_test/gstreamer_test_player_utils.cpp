#include "gstreamer_test_player_utils.h"

#include <dirent.h>
#include <memory.h>
#include <stdlib.h>

#define GST_LAUNCH_PATH "/usr/bin/gst-launch-1.0 "

using namespace std;

void GetFilesFromDirectory(vector<string> &testFiles, string dirpath) {
  struct dirent *entry;

  DIR *dir = opendir(dirpath.c_str());
  if (dir == NULL) {
    slog_info("Failed to opendir:%s", dirpath.c_str());
    return;
  }

  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_DIR) {
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        continue;

      string dirNew = dirpath + entry->d_name + "/";
      vector<string> tempPath;
      GetFilesFromDirectory(tempPath, dirNew);
      testFiles.insert(testFiles.end(), tempPath.begin(), tempPath.end());

    } else {
      string name = entry->d_name;
      string imgdir = dirpath + name;
      testFiles.push_back(imgdir);
    }
  }
  closedir(dir);
}

int RunGstLaunch(string command) {
  int ret = 0;
  string execbin = GST_LAUNCH_PATH + command;
  ret = system(execbin.c_str());
  return ret;
}

static void trim(string &s) {
  if (!s.empty()) {
    s.erase(0, s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
  }
}

int parseGstPlugins(const char *path, map<int, string> &pluginsMap) {
  int num = 0;
  ifstream in(path);
  string line;

  if (in) {
    while (getline(in, line)) {
      if (line.length() == 0) break;
      size_t end = line.find_last_of(':');
      size_t front = line.find_first_of(':');
      if ((front == end) || (line.find("typefindfunctions") != string::npos))
        continue;
      string element = line.substr(front + 1);
      trim(element);
      pluginsMap.insert(pair<int, string>(num, element));
      num++;
    }
  } else {
    slog_info("Not Found Gstreamer plugin file.");
    return 0;
  }
  return num;
}

int parseGstCommands(const char *path, vector<string> &cmds) {
  int num = 0;
  ifstream in(path);
  string line;

  if (in) {
    while (getline(in, line)) {
      if (line.length() == 0) break;
      cmds.push_back(line);
      num++;
    }
  } else {
    slog_info("Not Found Gstreamer command file.");
    return 0;
  }
  slog_info("Found Gstreamer command file num:%d", num);
  return num;
}
