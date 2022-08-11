#pragma once

#include <cstdint>
#include <vector>

typedef enum {
  CMD_ACTION_NONE,
  CMD_ACTION_WAIT,
  CMD_ACTION_PLAY,
  CMD_ACTION_PAUSE,
  CMD_ACTION_SEEK,
  CMD_ACTION_WAIT_LOAD,
  CMD_ACTION_WAIT_EOS,
  CMD_ACTION_NEXT,
  CMD_ACTION_SPEED,
  CMD_ACTION_PREROLL,
  CMD_ACTION_LOOP,
  CMD_ACTION_QUIT,
} cmd_action_type_t;

typedef struct cmd_action_data {
  union {
    int i;
    unsigned u;
    int64_t l;
    uint64_t ul;
    float f;
    double d;
    char c;
  };
} cmd_action_data_t;

typedef struct {
  cmd_action_type_t action;
  cmd_action_data_t data;
} cmd_action_t;

typedef std::vector<cmd_action_t> media_command_t;

std::vector<media_command_t> parseMediaCommands(const char* path);
