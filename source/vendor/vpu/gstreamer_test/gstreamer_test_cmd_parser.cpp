#include "gstreamer_test_cmd_parser.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

using std::cout;
using std::endl;
using std::string;
using std::vector;

static string readFile(const char* filename) {
  std::ifstream ifs(filename);
  if (ifs.fail()) {
    return string();
  }
  std::stringstream s;
  s << ifs.rdbuf();
  return s.str();
}

static vector<string> splitString(const string& text, const string& delimiter) {
  vector<string> tokens;
  if (text.empty()) {
    tokens.push_back(text);
    return tokens;
  }
  size_t textLength = text.length();
  int delimiterLength = delimiter.length();
  size_t last = 0;
  while (true) {
    size_t current = text.find(delimiter, last);
    if (current == string::npos) current = textLength;
    string token = text.substr(last, current - last);
    if (!token.empty()) tokens.push_back(token);
    last = current + delimiterLength;
    if (current >= textLength || last >= textLength) break;
  }
  return tokens;
}

static inline bool startsWith(const string& s, const string& prefix) {
  return s.rfind(prefix, 0) == 0;
}

static string strip(const string& str) {
  size_t b = str.find_first_not_of(" \t\r\n");
  size_t e = str.find_last_not_of(" \t\r\n");

  if (string::npos == b) {
    return string();
  }
  return str.substr(b, e - b + 1);
}

static inline bool contains(std::map<string, cmd_action_type_t> m,
                            const string& k) {
  return m.find(k) != m.end();
}

static cmd_action_type_t getAction(const string& str) {
  static std::map<string, cmd_action_type_t> actions;
  if (actions.empty()) {
    actions["wait"] = CMD_ACTION_WAIT;
    actions["play"] = CMD_ACTION_PLAY;
    actions["pause"] = CMD_ACTION_PAUSE;
    actions["seek"] = CMD_ACTION_SEEK;
    actions["wait_load"] = CMD_ACTION_WAIT_LOAD;
    actions["wait_eos"] = CMD_ACTION_WAIT_EOS;
    actions["next"] = CMD_ACTION_NEXT;
    actions["quit"] = CMD_ACTION_QUIT;
    actions["change_speed"] = CMD_ACTION_SPEED;
    actions["preroll"] = CMD_ACTION_PREROLL;
    actions["loop"] = CMD_ACTION_LOOP;
  }
  if (contains(actions, str)) {
    return actions[str];
  } else {
    return CMD_ACTION_NONE;
  }
}

static bool isInt(const string& str) {
  return string::npos == str.find_first_not_of("0123456789");
}

static bool isDecimal(const string& str) {
  bool validChars = (string::npos == str.find_first_not_of("0123456789."));
  bool atMostOneDot = (str.find_first_of('.') == str.find_last_of('.'));
  return validChars && atMostOneDot;
}

static const char* parseSpeed(const string& str) { return str.c_str(); }

static int parseNum(const string& str) {
  if (!isDecimal(str)) {
    return -1;
  }

  return std::stoi(str);
}

static int64_t parseTime(const string& str) {
  vector<string> splitMinutes = splitString(str, ":");
  if (splitMinutes.size() > 2) {
    return -1;
  }
  bool hasMinutes = splitMinutes.size() > 1;
  if (hasMinutes) {
    if (!isInt(splitMinutes[0]) || !isDecimal(splitMinutes[1])) {
      return -1;
    }
    int m = std::stoi(splitMinutes[0]);
    double seconds = std::stod(splitMinutes[1]);
    return static_cast<int64_t>((m * 60 + seconds) * 1000);
  }
  if (!isDecimal(str)) {
    return -1;
  }
  if (str.find('.') == string::npos) {
    return std::stoll(str);
  } else {
    double seconds = std::stod(str);
    return static_cast<int64_t>(seconds * 1000);
  }
}

static cmd_action_t parseCommandAction(const string& str) {
  string s = strip(str);
  string act = splitString(s, " ")[0];
  string param;
  size_t spacePos = s.find(' ');
  if (string::npos != spacePos) {
    param = strip(s.substr(spacePos + 1));
  }
  cmd_action_t action = {CMD_ACTION_NONE, {0}};
  action.action = getAction(act);
  switch (action.action) {
    case CMD_ACTION_WAIT:
    case CMD_ACTION_SEEK: {
      int64_t t = parseTime(param);
      if (t < 0) {
        action.action = CMD_ACTION_NONE;
        return action;
      }
      action.data.ul = static_cast<uint64_t>(t);
      break;
    }
    case CMD_ACTION_SPEED: {
      char c = parseSpeed(param)[0];
      if ((c != '+') && (c != '-')) {
        action.action = CMD_ACTION_NONE;
        return action;
      }
      action.data.c = c;
      break;
    }
    case CMD_ACTION_LOOP:
    case CMD_ACTION_PREROLL: {
      int num_frame = parseNum(param);
      if (num_frame < 0) {
        action.action = CMD_ACTION_NONE;
        return action;
      }
      action.data.i = num_frame;
      break;
    }
    default:
      break;
  }
  return action;
}

static media_command_t parseOneMediaCommand(const string& line) {
  if (startsWith(line, "#")) {
    // skip comment
    return media_command_t();
  }
  media_command_t cmd;
  vector<string> parts = splitString(line, "!");
  for (vector<string>::iterator it = parts.begin(); it != parts.end(); it++) {
    cmd_action_t action = parseCommandAction(*it);
    if (action.action == CMD_ACTION_NONE) {
      // parse failed
      return media_command_t();
    }
    cmd.push_back(action);
  }
  return cmd;
}

vector<media_command_t> parseMediaCommands(const char* path) {
  // string content = readFile(path);
  string content("play ! wait_eos ! quit\n"
                 "play ! pause ! seek 2000 ! play ! wait_eos ! quit\n"
                 "play ! pause ! seek 5000 ! play ! wait_eos ! quit\n"
                 "play ! wait 3000 ! pause ! seek 5000 ! play ! quit\n"
                 "play ! wait 3000 ! seek 5000 ! wait_eos ! quit\n"
                 "play ! seek 5000 ! wait_eos ! quit\n"
                  );
  vector<string> lines = splitString(content, "\n");
  vector<media_command_t> commands;
  for (vector<string>::iterator it = lines.begin(); it != lines.end(); it++) {
    media_command_t cmd = parseOneMediaCommand(*it);
    if (!cmd.empty()) {
      commands.push_back(cmd);
    } else
      cout << *it << endl;
  }
  return commands;
}
