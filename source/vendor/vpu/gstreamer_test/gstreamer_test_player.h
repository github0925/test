#pragma once

#include <glib.h>
#include <gst/gst.h>
#include <stdbool.h>

/* load xml parse element name. */
void load_hw_decoder_elements();

int on_prepared(const char *test_file);

int on_start(const char *test_file);

int on_pause();

int on_resume();

int on_stop();

int on_flush();

int on_seek(gint64);

int on_delay(int sec);

int on_wait_eos();

int on_destory();

int on_speed_player(char speed);

int on_preroll_frame(int frame);