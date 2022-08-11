#pragma once

#include <gst/gst.h>
#include <signal.h>
#include <string.h>
#include <time.h>

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "glib.h"
#include "gstreamer_test_utils.h"

using namespace std;

void GetFilesFromDirectory(vector<string> &testFiles, string dirpath);

int RunGstLaunch(string command);

int RunGstInspect(void);

int parseGstPlugins(const char *path, map<int, string> &pluginsMap);

int parseGstCommands(const char *path, vector<string> &cmds);
