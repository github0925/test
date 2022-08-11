/*
 * jputest.c
 *
 * Copyright (C) 2020 Semidrive Technology Co., Ltd.
 *
 * Description:
 *
 * Revision Histrory:
 * -----------------
 * 1.0, 21/12/2020  chentianming <tianming.chen@semidrive.com> create this file
 *
 */
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <strings.h>
#include "jpuapi.h"
#include "jpulog.h"
#include "jpuapifunc.h"
#include "main_helper.h"

#define DEFAULT_OUTPUT_PATH         "output.jpg"
#define TEST_LOG_FILE               "test_log.txt"
#define DEFAULT_STREAM_SIZE         0x200000

/*
 * brief struct  test_config
 *       Description of test config information
 * param is_encode  flag for encode
 * param dec_config  decode config
 * param enc_config  encode config
 */
struct test_config {
	BOOL is_encode;
	DecConfigParam dec_config;
	EncConfigParam enc_config;
};

static void Help(const char *programName)
{
	JLOG(INFO, "------------------------------------------------------------------------------\n");
	JLOG(INFO, " CODAJ12 Decoder/Encoder config ");
	JLOG(INFO, "------------------------------------------------------------------------------\n");
	JLOG(INFO, "%s [options] --input=jpg_file_path\n", programName);
	JLOG(INFO, "-h                      help\n");
	JLOG(INFO, "-e                      enable encoder config\n");
	JLOG(INFO, "                        -e options must be behind progranmName\n");
	JLOG(INFO, "-n                      decode/encode frame num\n");
	JLOG(INFO, "--input=FILE            decode:jpeg filepath; encode:yuv file(when no cfg file)\n");
	JLOG(INFO, "--output=FILE           decode:output file path; encode:jpeg file(when no cfg file)\n");
	JLOG(INFO, "--stream-endian=ENDIAN  bitstream endianness. refer to datasheet Chapter 4.\n");
	JLOG(INFO, "--frame-endian=ENDIAN   pixel endianness of 16bit input source. refer to datasheet Chapter 4.\n");
	JLOG(INFO, "--pixelj=JUSTIFICATION  16bit-pixel justification. 0(default) - msb justified, 1 - lsb justified in little-endianness\n");
	JLOG(INFO, "--bs-size=SIZE          bitstream buffer size in byte\n");
	JLOG(INFO, "--roi=x,y,w,h           ROI region\n");
	JLOG(INFO, "--subsample             conversion sub-sample(ignore case): NONE, 420, 422, 444\n");
	JLOG(INFO, "--ordering              conversion ordering(ingore-case): NONE, NV12, NV21, YUYV, YVYU, UYVY, VYUY, AYUV\n");
	JLOG(INFO, "                        NONE - planar format\n");
	JLOG(INFO, "                        NV12, NV21 - semi-planar format for all the subsamples.\n");
	JLOG(INFO, "                                     If subsample isn't defined or is none, the sub-sample depends on jpeg information\n");
	JLOG(INFO, "                                     The subsample 440 can be converted to the semi-planar format. It means that the encoded sub-sample should be 440.\n");
	JLOG(INFO, "                        YUVV..VYUY - packed format. subsample be ignored.\n");
	JLOG(INFO, "                        AYUV       - packed format. subsample be ignored.\n");
	JLOG(INFO, "--rotation              0, 90, 180, 270\n");
	JLOG(INFO, "--mirror                0(none), 1(V), 2(H), 3(VH)\n");
	JLOG(INFO, "--scaleH                Horizontal downscale: 0(none), 1(1/2), 2(1/4), 3(1/8)\n");
	JLOG(INFO, "--scaleV                Vertical downscale  : 0(none), 1(1/2), 2(1/4), 3(1/8)\n");
	JLOG(INFO, "--bsmode                default 0  frame; !0 fix_size \n");
	JLOG(INFO, "--cfg FILE              input cfg file only for encode\n");
	JLOG(INFO, "--slice-height=height   the vertical height of a slice. multiple of 8 alignment. 0 is to set the height of picture\n");
	JLOG(INFO, "--enable-slice-intr     enable get the interrupt at every slice encoded\n");
	JLOG(INFO, "--quality=PERCENTAGE    quality factor(0..100)\n");
#ifdef SUPPORT_12BIT
	JLOG(INFO, "--pixelj=JUSTIFICATION  16bit-pixel justification. 0(default) - msb justified, 1 - lsb justified in little-endianness\n");
#endif
	JLOG(INFO, "--enable-tiledMode      enable tiled mode (default linear mode)\n");
	exit(1);
}

static BOOL test_decode(struct test_config *config)
{
	BOOL ret = TRUE;

	/* check params */
	if ((config->dec_config.iHorScaleMode || config->dec_config.iVerScaleMode) && config->dec_config.roiEnable) {
		JLOG(ERR, "Invalid operation mode : ROI cannot work with the scaler\n");
		return 1;
	}

	if (config->dec_config.packedFormat && config->dec_config.roiEnable) {
		JLOG(ERR, "Invalid operation mode : ROI cannot work with the packed format conversion\n");
		return 1;
	}

	if (config->dec_config.roiEnable && (config->dec_config.rotation || config->dec_config.mirror)) {
		JLOG(ERR, "Invalid operation mode : ROI cannot work with the PPU.\n");
	}

	if (config->dec_config.packedFormat == PACKED_FORMAT_422_YUYV ||
		config->dec_config.packedFormat == PACKED_FORMAT_422_UYVY ||
		config->dec_config.packedFormat == PACKED_FORMAT_422_YVYU ||
		config->dec_config.packedFormat == PACKED_FORMAT_422_VYUY ||
		config->dec_config.packedFormat == PACKED_FORMAT_444) {
		if (config->dec_config.subsample == FORMAT_420 ||
			config->dec_config.subsample == FORMAT_422 ||
			config->dec_config.subsample == FORMAT_444) {
			JLOG(ERR, "Invalid operation mode : subsample cannot be enabled to do subsample to \
					420 or 420 or 444 if ordering is YUYV or UYVY or YVYU or VYUY or AYUV\n");
			return 1;
		}
	}

	if (config->dec_config.feedingMode == 0)
		config->dec_config.feedingMode = FEEDING_METHOD_FRAME_SIZE;
	else
		config->dec_config.feedingMode = FEEDING_METHOD_FIXED_SIZE;

	ret = TestDecoder(&config->dec_config);
	return ret;
}

static BOOL test_encode(struct test_config *config)
{
	BOOL ret = TRUE;
	strcpy(config->enc_config.bitstreamFileName, DEFAULT_OUTPUT_PATH);
	ret = TestEncoder(&config->enc_config);
	return ret;
}

int main(Int32 argc, char **argv)
{
	struct option longOpt[] = {
		{ "stream-endian",      required_argument, NULL, 0 },
		{ "frame-endian",       required_argument, NULL, 0 },
		{ "output",             required_argument, NULL, 0 },
		{ "input",              required_argument, NULL, 0 },
		{ "pixelj",             required_argument, NULL, 0 },
		{ "bs-size",            required_argument, NULL, 0 },
		{ "roi",                required_argument, NULL, 0 },
		{ "subsample",          required_argument, NULL, 0 },
		{ "ordering",           required_argument, NULL, 0 },
		{ "rotation",           required_argument, NULL, 0 },
		{ "mirror",             required_argument, NULL, 0 },
		{ "scaleH",             required_argument, NULL, 0 },
		{ "scaleV",             required_argument, NULL, 0 },
		{ "slice-height",       required_argument, NULL, 0 },
		{ "enable-slice-intr",  required_argument, NULL, 0 },
		{ "bsmode",             required_argument, NULL, 0 },
		{ "cfg",                required_argument, NULL, 0 },
		{ "quality",            required_argument, NULL, 0 },
		{ "enable-tiledMode",   required_argument, NULL, 0 },
		{ "12bit",              no_argument,       NULL, 0 },
		{ NULL,                 no_argument,       NULL, 0 },
	};
	int ret = 1, c = 0, l = 0;
	char *shortOpt    = (char *)"hen:";
	struct test_config config = {0};
	config.dec_config.subsample = FORMAT_MAX;
	config.dec_config.bsSize = DEFAULT_STREAM_SIZE;
	config.enc_config.bsSize = DEFAULT_STREAM_SIZE;

	while ((c = getopt_long(argc, argv, shortOpt, longOpt, &l)) != -1) {
		switch (c) {
		case 'h':
			Help(argv[0]);
			break;
		case 'e':
			config.is_encode = TRUE;
			break;
		case 'n':
			config.dec_config.outNum = atoi(optarg);
			config.enc_config.outNum = config.dec_config.outNum;
			break;
		case 0:
			if (!config.is_encode) {
				if (ParseDecTestLongArgs((void *) & (config.dec_config), longOpt[l].name, optarg) == FALSE) {
					Help(argv[0]);
				}
			} else {
				if (ParseEncTestLongArgs((void *) & (config.enc_config), longOpt[l].name, optarg) == FALSE) {
					Help(argv[0]);
				}
			}
			break;
		default:
			Help(argv[0]);
			break;
		}
	}

	InitLog(TEST_LOG_FILE);

	if (config.is_encode) {
		JLOG(INFO, "start encode now ...\n");
		ret = test_encode(&config);
	} else {
		JLOG(INFO, "start decode now ...\n");
		ret = test_decode(&config);
	}

	DeInitLog();
	return ret == TRUE ? 0 : 1;
}

