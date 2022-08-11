/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited. All Rights Reserved.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */
/*
 * Key codes definition. Generated from the key layout file
 * and conf file.
 */
#ifndef WAYLAND_KEYCODES_H
#define WAYLAND_KEYCODES_H

#ifdef __cplusplus
extern "C" {
#endif

    enum {
        YKEY_ESCAPE = 1,
        YKEY_1 = 2,
        YKEY_2 = 3,
        YKEY_3 = 4,
        YKEY_4 = 5,
        YKEY_5 = 6,
        YKEY_6 = 7,
        YKEY_7 = 8,
        YKEY_8 = 9,
        YKEY_9 = 10,
        YKEY_0 = 11,
        YKEY_MINUS = 12,
        YKEY_EQUALS = 13,
        YKEY_DEL = 14,//Backspace
        YKEY_TAB = 15,
        YKEY_Q = 16,
        YKEY_W = 17,
        YKEY_E = 18,
        YKEY_R = 19,
        YKEY_T = 20,
        YKEY_Y = 21,
        YKEY_U = 22,
        YKEY_I = 23,
        YKEY_O = 24,
        YKEY_P = 25,
        YKEY_LEFT_BRACKET = 26,
        YKEY_RIGHT_BRACKET = 27,
        YKEY_DPAD_CENTER = 28,
        YKEY_CTRL_LEFT = 29,
        YKEY_A = 30,
        YKEY_S = 31,
        YKEY_D = 32,
        YKEY_F = 33,
        YKEY_G = 34,
        YKEY_H = 35,
        YKEY_J = 36,
        YKEY_K = 37,
        YKEY_L = 38,
        YKEY_SEMICOLON = 39,
        YKEY_APOSTROPHE = 40,
        YKEY_GRAVE = 41,
        YKEY_SHIFT_LEFT = 42,
        YKEY_BACKSLASH = 43,
        YKEY_Z = 44,
        YKEY_X = 45,
        YKEY_C = 46,
        YKEY_V = 47,
        YKEY_B = 48,
        YKEY_N = 49,
        YKEY_M = 50,
        YKEY_COMMA = 51,
        YKEY_PERIOD = 52,
        YKEY_SLASH = 53,
        YKEY_SHIFT_RIGHT = 54,
        YKEY_NUMPAD_MULTIPLY = 55,
        YKEY_ALT_LEFT = 56,
        YKEY_SPACE = 57,
        YKEY_CAPS_LOCK = 58,
        YKEY_F1 = 59,
        YKEY_F2 = 60,
        YKEY_F3 = 61,
        YKEY_F4 = 62,
        YKEY_F5 = 63,
        YKEY_F6 = 64,
        YKEY_F7 = 65,
        YKEY_F8 = 66,
        YKEY_F9 = 67,
        YKEY_F10 = 68,
        YKEY_NUM_LOCK = 69,
        YKEY_SCROLL_LOCK = 70,
        YKEY_NUMPAD_7 = 71,
        YKEY_NUMPAD_8 = 72,
        YKEY_NUMPAD_9 = 73,
        YKEY_NUMPAD_SUBTRACT = 74,
        YKEY_NUMPAD_4 = 75,
        YKEY_NUMPAD_5 = 76,
        YKEY_NUMPAD_6 = 77,
        YKEY_NUMPAD_ADD = 78,
        YKEY_NUMPAD_1 = 79,
        YKEY_NUMPAD_2 = 80,
        YKEY_NUMPAD_3 = 81,
        YKEY_NUMPAD_0 = 82,
        YKEY_NUMPAD_DOT = 83,
        YKEY_FOCUS = 84,
        YKEY_CAMERA = 85,
        //YKEY_BACKSLASH = 86,
        YKEY_F11 = 87,
        YKEY_F12 = 88,
        YKEY_AIRPLANE_MODE = 88,
        YKEY_RO = 89,
        YKEY_NUMPAD_COMMA = 95,
        YKEY_NUMPAD_ENTER = 96,
        YKEY_CTRL_RIGHT = 97,
        YKEY_NUMPAD_DIVIDE = 98,
        YKEY_SYSRQ = 99,
        YKEY_ALT_RIGHT = 100,
        YKEY_HOMEPAGE = 102,
        YKEY_MOVE_HOME = 102,
        YKEY_DPAD_UP = 103,
        YKEY_PAGE_UP = 104,
        YKEY_DPAD_LEFT = 105,
        YKEY_DPAD_RIGHT = 106,
        YKEY_MOVE_END = 107,
        YKEY_DPAD_DOWN = 108,
        YKEY_PAGE_DOWN = 109,
        YKEY_INSERT = 110,
        YKEY_FORWARD_DEL = 111,
        YKEY_MUTE = 112,
        YKEY_VOLUME_MUTE = 113,
        YKEY_VOLUME_DOWN = 114,
        YKEY_VOLUME_UP = 115,
        YKEY_POWER_WAKE = 116,
        YKEY_NUMPAD_EQUALS = 117,
        YKEY_BREAK = 119,
        //YKEY_NUMPAD_COMMA = 121,
        YKEY_META_LEFT = 125,
        YKEY_META_RIGHT = 126,
        YKEY_MENU = 139,
        YKEY_SETUP = 141, // SETUP
        YKEY_SLEEP = 142,
        YKEY_WAKEUP = 143,
        YKEY_BACK = 158,
        YKEY_MEDIA_NEXT = 163,
        YKEY_MEDIA_PLAY_PAUSE = 164,
        YKEY_MEDIA_PREVIOUS = 165,
        YKEY_MEDIA_STOP = 166,
        YKEY_MEDIA_RECORD = 167,
        YKEY_MEDIA_REWIND = 168,
        YKEY_PHONE = 169, // PHONE
        YKEY_HOME = 172,
        YKEY_BRIGHTNESS_DOWN  = 224,
        YKEY_BRIGHTNESS_UP = 225,
        YKEY_F13 = 183, //360 for car
        YKEY_F14 = 184, //Music for car
        YKEY_ACCOUNT = 185,
        YKEY_MAINTAIN = 186,
        YKEY_HYBIRD = 187,
        YKEY_COMMUNICATE = 188,
        YKEY_MEDIA_PLAY = 200,
        YKEY_MEDIA_PAUSE = 201,
        YKEY_MEDIA_FAST_FORWARD = 208,
        YKEY_CAPS_Q = 216,
        YKEY_CAPS_W = 217,
        YKEY_CAPS_E = 218,
        YKEY_CAPS_R = 219,
        YKEY_CAPS_T = 220,
        YKEY_CAPS_Y = 221,
        YKEY_CAPS_U = 222,
        YKEY_CAPS_I = 223,
        YKEY_CAPS_O = 173,
        YKEY_CAPS_P = 174,
        YKEY_MEDIA_HEADSETHOOK = 226,
        YKEY_CAPS_A = 230,
        YKEY_CAPS_S = 231,
        YKEY_CAPS_D = 232,
        YKEY_CAPS_F = 233,
        YKEY_CAPS_G = 234,
        YKEY_CAPS_H = 235,
        YKEY_CAPS_J = 236,
        YKEY_CAPS_K = 237,
        YKEY_CAPS_L = 238,
        YKEY_CAPS_Z = 244,
        YKEY_CAPS_X = 245,
        YKEY_CAPS_C = 246,
        YKEY_CAPS_V = 247,
        YKEY_CAPS_B = 248,
        YKEY_CAPS_N = 249,
        YKEY_CAPS_M = 250,
        YKEY_N_POLE = 251,
        YKEY_S_POLE = 252,
        YKEY_BTN_LEFT = 0x110, //272
        YKEY_BTN_RIGHT = 0x111, //273
        YKEY_BTN_MIDDLE = 0x112, //274
        YKEY_GAME_X = 0x133, //307
        YKEY_GAME_Y = 0x134,
        YKEY_GAME_L1 = 0x136,
        YKEY_GAME_R1 = 0x137,
        YKEY_GAME_L2 = 0x138,
        YKEY_GAME_R2 = 0x139, //313
        YKEY_TUNER = 0x182,             // TUNER
        YKEY_CHANNELUP = 0x192,         // CHANNEL UP
        YKEY_CHANNELDOWN = 0x193,       // CHANNEL DOWN
        YKEY_STAR = 522,                // 0x20A
        YKEY_POUND = 523,
        YKEY_CALL = 524,
        YKEY_CUSTOM_BEGIN = 0x240,
        YKEY_VOICE_RECOGNITION = 0x240, // CONTROL VOICE RECOGNITION
        YKEY_MODE_SOURCE = 0x241,       // MODE/SOURCE
        YKEY_CLIMATE = 0x242,           // AIR CONDITION
        YKEY_DEF = 0x243,               // FRONT WINDOW
        YKEY_RDEF = 0x244,              // BACK WINDOW
        YKEY_APP_SWITCH = 0x244,
        YKEY_CAR = 0x245,             //
        YKEY_SAFTY = 0x245,
        YKEY_INKANET = 0x246,           //
        YKEY_VOICE_ASSIST = 0x246,
        YKEY_ICALL = 0x247,
        YKEY_BCALL = 0x248,
        YKEY_EDGE_SHORT_SQUEEZE = 0x247,
        YKEY_EDGE_LONG_SQUEEZE = 0x248,
        YKEY_VOICE_RECOGNITION_START = 0x249,
        YKEY_VOICE_RECOGNITION_STOP = 0x250,
        YKEY_CUSTOME_L = 0x24B,
        YKEY_CUSTOME_R = 0x24C,
        YKEY_TAKE_PHOTO = 0x251,
        YKEY_CUSTOME_END,
    };

#ifdef __cplusplus
}
#endif

#endif
