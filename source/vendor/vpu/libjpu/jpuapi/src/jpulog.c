//------------------------------------------------------------------------------
// File: log.c
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------

#include "jpuconfig.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "jpulog.h"
#if defined(linux) || defined(__linux) || defined(ANDROID)
#include <time.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>   // for read()
static struct termios initial_settings, new_settings;
static int peek_character = -1;
#endif

#define SUPPORT_LOG_SAVE
#define OUTPUT_JPU_LOG_TO_STDERR "OUTPUT_JPU_LOG_TO_STDERR"
#define OUTPUT_JPU_LOG_TO_FILE "OUTPUT_JPU_LOG_TO_FILE"

static int log_colors[MAX_LOG_LEVEL] = {
	0,
	TERM_COLOR_R | TERM_COLOR_G | TERM_COLOR_B | TERM_COLOR_BRIGHT, //INFO
	TERM_COLOR_R | TERM_COLOR_B | TERM_COLOR_BRIGHT, //WARN
	TERM_COLOR_R | TERM_COLOR_BRIGHT, // ERR
	TERM_COLOR_R | TERM_COLOR_G | TERM_COLOR_B //TRACE
};

static unsigned log_decor = LOG_HAS_TIME | LOG_HAS_FILE | LOG_HAS_MICRO_SEC |
                            LOG_HAS_NEWLINE |
                            LOG_HAS_SPACE | LOG_HAS_COLOR;
static int max_log_level =  MAX_LOG_LEVEL;
static FILE *fpLog  = NULL;

BOOL InitLog(const char *path)
{
	if (path != NULL) {
		if ((fpLog = fopen(path, "w")) == NULL) {
			printf("Failed to open log file(%s)\n", path);
			return FALSE;
		}
	}

	return TRUE;
}

void DeInitLog()
{
	if (fpLog != NULL) {
		fclose(fpLog);
		fpLog = NULL;
	}
}

void SetLogColor(int level, int color)
{
	log_colors[level] = color;
}

int GetLogColor(int level)
{
	return log_colors[level];
}

void SetLogDecor(int decor)
{
	log_decor = decor;
}

int GetLogDecor()
{
	return log_decor;
}

void SetMaxLogLevel(int level)
{
	max_log_level = level;
}
int GetMaxLogLevel()
{
	return max_log_level;
}

void LogMsg(int level, const char *format, ...)
{
	va_list ptr;
	char * str = NULL;
	char logBuf[MAX_PRINT_LENGTH] = {0};
	static int once = 0;
	static int output_to_std = 0;

	if (level > max_log_level)
		return;

	if (once && !output_to_std && !fpLog)
	{
#ifndef ANDROID
		return; //reduce log time
#endif
	}

	va_start( ptr, format );
	vsnprintf( logBuf, MAX_PRINT_LENGTH, format, ptr );
	va_end(ptr);
#ifdef ANDROID
	if (level == ERR) {
#ifdef LOGE
		LOGE("%s", logBuf);
#endif
#ifdef ALOGE
		ALOGE("%s", logBuf);
#endif
	} else {
#ifdef LOGI
		LOGI("%s", logBuf);
#endif
#ifdef ALOGI
		ALOGI("%s", logBuf);
#endif
	}

	puts(logBuf);
#else
	if (!once)
	{
		once = 1;
		str = getenv(OUTPUT_JPU_LOG_TO_STDERR);

		if (str != NULL)
			output_to_std = 1;

		str = getenv(OUTPUT_JPU_LOG_TO_FILE);
		if (str != NULL && *str != '\0')
			InitLog(str);
	}

	if (output_to_std)
		fprintf(stderr, logBuf);
#endif
#ifdef SUPPORT_LOG_SAVE

	if ((log_decor & LOG_HAS_FILE) && fpLog) {
		fwrite(logBuf, strlen(logBuf), 1, fpLog);
		fflush(fpLog);
	}

#endif
}

static double timer_frequency_;

#if defined(linux) || defined(__linux) || defined(ANDROID)
struct timeval tv_start;
struct timeval tv_end;
#endif

void timer_start()
{
#if defined(linux) || defined(__linux) || defined(ANDROID)
#else
	timer_init();
#endif
#if defined(linux) || defined(__linux) || defined(ANDROID)
	gettimeofday(&tv_start, NULL);
#else
#endif
}

void timer_stop()
{
#if defined(linux) || defined(__linux) || defined(ANDROID)
	gettimeofday(&tv_end, NULL);
#else
#endif
}

double timer_elapsed_ms()
{
	double ms;
	ms = timer_elapsed_us() / 1000.0;
	return ms;
}

double timer_elapsed_us()
{
	double elapsed = 0;
#if defined(linux) || defined(__linux) || defined(ANDROID)
	double start_us = 0;
	double end_us = 0;
	end_us = tv_end.tv_sec * 1000 * 1000 + tv_end.tv_usec;
	start_us = tv_start.tv_sec * 1000 * 1000 + tv_start.tv_usec;
	elapsed =  end_us - start_us;
#else
#endif
	return elapsed;
}

int timer_is_valid()
{
	return timer_frequency_ != 0;
}

double timer_frequency()
{
	return timer_frequency_;
}

#if defined(linux) || defined(__linux) || defined(ANDROID)

void init_keyboard()
{
	tcgetattr(0, &initial_settings);
	new_settings = initial_settings;
	new_settings.c_lflag &= ~ICANON;
	new_settings.c_lflag &= ~ECHO;
	new_settings.c_cc[VMIN] = 1;
	new_settings.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &new_settings);
	peek_character = -1;
}

void close_keyboard()
{
	tcsetattr(0, TCSANOW, &initial_settings);
	fflush(stdout);
}

int kbhit()
{
	unsigned char ch;
	int nread;

	if (peek_character != -1) return 1;

	new_settings.c_cc[VMIN] = 0;
	tcsetattr(0, TCSANOW, &new_settings);
	nread = read(0, &ch, 1);
	new_settings.c_cc[VMIN] = 1;
	tcsetattr(0, TCSANOW, &new_settings);

	if (nread == 1) {
		peek_character = (int)ch;
		return 1;
	}

	return 0;
}

int getch()
{
	int val;
	char ch;

	if (peek_character != -1) {
		val = peek_character;
		peek_character = -1;
		return val;
	}

	read(0, &ch, 1);
	return ch;
}
#endif  //#if defined(linux) || defined(__linux) || defined(ANDROID)

