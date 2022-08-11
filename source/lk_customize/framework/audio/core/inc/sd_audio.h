/**
 * @file sd_audio.h
 * @brief common audio parameters
 * @details define some common audio parameters such as i2s interface, sample rate,
 *          channel number etc.
 * @mainpage Mainpage
 * @author Su Zhenxiu
 * @version 2.0
 * @brief declare some common audio data and audio transfer parameters.
 * @date 2020-07-29
 */

#ifndef __SD_AUDIO_H__
#define __SD_AUDIO_H__

#include <sys/types.h>
#include <kernel/event.h>
#ifndef BIT
#define BIT(x) (1 << (x))
#endif
#ifndef GEN_MASK
#if IS_64BIT
#define GEN_MASK(h, l) (((~0UL) - (1UL << (l)) + 1) & (~0UL >> (64 - 1 - (h))))
#else
#define GEN_MASK(h, l) (((~0UL) - (1UL << (l)) + 1) & (~0UL >> (32 - 1 - (h))))
#endif
#endif

/***************** pcm mode params ops *********************************/
#define SD_AUDIO_PCM_MODE_SET(mode, mask, val)                                 \
    (((mode) & (~mask)) | (val & mask))
#define SD_AUDIO_PCM_MODE_GET(mode, mask) ((mode) & (mask))

/***************** pcm format parameters ********************/

/** i2s interface mode */
enum sd_audioflow_i2s_interface_mode_t {
    SD_AUDIO_I2S_INTERFACE_MODE_NO_INIT =
        0,                          ///< i2s interface mode not initialized
    SD_AUDIO_I2S_STANDARD_PHILLIPS, ///< i2s standard phillips interface mode
    SD_AUDIO_I2S_LEFT_JUSTIFIED,    ///< i2s left justified interface mode
    SD_AUDIO_I2S_RIGHT_JUSTIFIED,   ///< i2s right justified interface mode
    SD_AUDIO_I2S_DSP_A, ///< i2s dsp mode-A: second falling edge after ws
                        ///< change.
    SD_AUDIO_I2S_DSP_B, ///< i2s dsp mode-B: first falling edge after ws change.
};

/******************* device hardware parameters ***********************/

// transfer control mode
#define SD_AUDIO_TRANSFER_MODE_ENABLE                                          \
    (0x0f) ///< transfer mode setting bits position
#define SD_AUDIO_TRANSFER_CODEC_SLAVE                                          \
    (0x1) ///< set i2s driver as master, naturally codec becomes a slave
#define SD_AUDIO_TRANSFER_CODEC_MASTER                                         \
    (0x2) ///< set i2s driver as slave, naturally codec becomes a master

// transfer media mode
#define SD_AUDIO_TRANSFER_MEDIA_MODE_ENABLE                                    \
    (0xf0) ///< transfer media mode setting bits position
#define SD_AUDIO_TRANSFER_WITH_DMA (0x1 << 4) ///< transfer with dma
#define SD_AUDIO_TRANSFER_WITH_CPU (0x2 << 4) ///< transfer with interrupt

// transfer direction mode
#define SD_AUDIO_DIR_MODE_ENABLE                                               \
    (0xff00) ///< transfer direction setting bits position
#define SD_AUDIO_DIR_MODE_TRANSMIT (0x1 << 8) ///< set device to transmitter
#define SD_AUDIO_DIR_MODE_RECEIVE (0x2 << 8) ///< set device to receiver
#define SD_AUDIO_DIR_MODE_FULL_DUPLEX                                          \
    (0x4 << 8) ///< set device to full duplex mode: transmit and receive
               ///< synchronously
#define SD_AUDIO_DIR_MODE_MULTI_CHAN_TRANSMIT                                  \
    (0x8 << 8) ///< set device to transmit with more than 2 channels
#define SD_AUDIO_DIR_MODE_MULTI_CHAN_RECEIVE                                   \
    (0x10 << 8) ///< set device to receive with more than 2 channels
#define SD_AUDIO_DIR_MODE_MULTI_CHAN_FULL_DUPLEX                               \
    (0x20 << 8) ///< set device to full duplex mode: transmit and receive
                ///< synchronously with more than 2 channels

// tx channel number
#define SD_AUDIO_CHANNEL_NUM_TX_ENABLE                                         \
    (0xff0000) ///< tx channel number setting bits position
#define SD_AUDIO_CHANNEL_NUM_TX_MONO (0x1 << 16) ///< set tx channel number to 1
#define SD_AUDIO_CHANNEL_NUM_TX_STEREO                                         \
    (0x2 << 16) ///< set tx channel number to 2
#define SD_AUDIO_CHANNEL_NUM_TX_4CHANS                                         \
    (0x4 << 16) ///< set tx channel number to 4
#define SD_AUDIO_CHANNEL_NUM_TX_8CHANS                                         \
    (0x8 << 16) ///< set tx channel number to 8
#define SD_AUDIO_CHANNEL_NUM_TX_16CHANS                                        \
    (0x10 << 16) ///< set tx channel number to 16
// rx channel number
#define SD_AUDIO_CHANNEL_NUM_RX_ENABLE                                         \
    (0xff000000) ///< rx channel number setting bits position
#define SD_AUDIO_CHANNEL_NUM_RX_MONO (0x1 << 24) ///< set rx channel number to 1
#define SD_AUDIO_CHANNEL_NUM_RX_STEREO                                         \
    (0x2 << 24) ///< set rx channel number to 2
#define SD_AUDIO_CHANNEL_NUM_RX_4CHANS                                         \
    (0x4 << 24) ///< set rx channel number to 4
#define SD_AUDIO_CHANNEL_NUM_RX_8CHANS                                         \
    (0x8 << 24) ///< set rx channel number to 8
#define SD_AUDIO_CHANNEL_NUM_RX_16CHANS                                        \
    (0x10 << 16) ///< set rx channel number to 16

/// sample rate
enum sd_audioflow_sample_rate_t {
    SD_AUDIO_SR_NO_INIT = 0,     ///< audio sample rate not initialized
    SD_AUDIO_SR_5512 = 5512,     ///< set sample rate to 5512
    SD_AUDIO_SR_8000 = 8000,     ///< set sample rate to 8000
    SD_AUDIO_SR_11025 = 11025,   ///< set sample rate to 11025
    SD_AUDIO_SR_16000 = 16000,   ///< set sample rate to 16000
    SD_AUDIO_SR_22050 = 22050,   ///< set sample rate to 22050
    SD_AUDIO_SR_32000 = 32000,   ///< set sample rate to 32000
    SD_AUDIO_SR_44100 = 44100,   ///< set sample rate to 44100
    SD_AUDIO_SR_48000 = 48000,   ///< set sample rate to 48000
    SD_AUDIO_SR_64000 = 64000,   ///< set sample rate to 64000
    SD_AUDIO_SR_88200 = 88200,   ///< set sample rate to 88200
    SD_AUDIO_SR_96000 = 96000,   ///< set sample rate to 96000
    SD_AUDIO_SR_176400 = 176400, ///< set sample rate to 176400
    SD_AUDIO_SR_192000 = 192000, ///< set sample rate to 192000
};

/// sample width
enum sd_audioflow_sample_width_t {
    SD_AUDIO_SAMPLE_WIDTH_NO_INIT =
        0,                        ///< audio sample resolution not initialized
    SD_AUDIO_SAMPLE_WIDTH_8BITS,  ///< set sample width to 8bits
    SD_AUDIO_SAMPLE_WIDTH_16BITS, ///< set sample width to 16bits
    SD_AUDIO_SAMPLE_WIDTH_20BITS, ///< set sample width to 20bits
    SD_AUDIO_SAMPLE_WIDTH_24BITS, ///< set sample width to 24bits
    SD_AUDIO_SAMPLE_WIDTH_32BITS, ///< set sample width to 32bits
};

/// slot width
enum sd_audioflow_slot_width_t {
    SD_AUDIO_SLOT_WIDTH_NO_INIT = 0, ///< audio slot width not initialized
    SD_AUDIO_SLOT_WIDTH_8BITS,       ///< set slot width to 8bits
    SD_AUDIO_SLOT_WIDTH_16BITS,      ///< set slot width to 16bits
    SD_AUDIO_SLOT_WIDTH_20BITS,      ///< set slot width to 20bits
    SD_AUDIO_SLOT_WIDTH_24BITS,      ///< set slot width to 24bits
    SD_AUDIO_SLOT_WIDTH_32BITS,      ///< set slot width to 32bits
};

/****************************pcm trigger
 * command********************************/
// trigger cmd
#define SD_AUDIO_PCM_TRIGGER_PLAYBACK_START 0x0 ///< trigger pcm playback start
#define SD_AUDIO_PCM_TRIGGER_CAPTURE_START 0x1 ///< trigger pcm capture start
#define SD_AUDIO_PCM_TRIGGER_PLAYBACK_STOP 0x2 ///< trigger pcm playback stop
#define SD_AUDIO_PCM_TRIGGER_CAPTURE_STOP 0x3 ///< trigger pcm capture stop

/***********************return val define******************************/

// return val
#define SD_AUDIO_NO_ERROR (0x0) ///< audio related operation run successfully
#define SD_AUDIO_ERR_NO_INIT (0x1) ///< audio args not initialized
#define SD_AUDIO_ERR_INVALID_OR_UNKNOWN_ARGS                                   \
    (0x2) ///< audio args match invalid or unknown

/***********************pcm param
 * struct******************************************/
// TODO: delete in future.
struct audio_int_transfer_ctrl {
    char *src_addr;
    char *dst_addr;
    int tx_len;
    int rx_len;
    event_t tx_comp;
    event_t rx_comp;
};

/** pcm dataflow description */
typedef struct {
    u_int32_t
        mode; ///< transfer control mode, direction mode and channel number
    enum sd_audioflow_sample_rate_t sample_rate; ///< audio sample rate
    enum sd_audioflow_sample_width_t resolution; ///< audio sample resolution
    enum sd_audioflow_i2s_interface_mode_t standard; ///< i2s interface mode
    enum sd_audioflow_slot_width_t slot_width;       ///< pcm data slot width
    struct audio_int_transfer_ctrl xctrl; ///< tx/rx cpu-transfer-mode info
} pcm_params_t;

/** ip controller info */
struct dev_controller_info {
    paddr_t addr; ///< register base address
    int bus;      ///< bus number
    int irq_num;  ///< interrput number
};

/******************************************* codec ctrl part
 * *****************************************************/

enum audio_codec_input {
    AUDIO_CODEC_SET_INPUT_AS_DEFAULT = 0,
    AUDIO_CODEC_SET_INPUT_AS_LINE_IN,
    AUDIO_CODEC_SET_INPUT_AS_MIC_IN,
};

enum audio_codec_output {
    AUDIO_CODEC_SET_OUTPUT_AS_DEFAULT = 0,
    AUDIO_CODEC_SET_OUTPUT_AS_LINE_OUT,
    AUDIO_CODEC_SET_OUTPUT_AS_PHONE_OUT,
};
enum audio_volume_type {
    AUDIO_VOL_INPUT = 0,
    AUDIO_VOL_LINEIN = AUDIO_VOL_INPUT,
    AUDIO_VOL_OUTPUT,
    AUDIO_VOL_LINEOUT = AUDIO_VOL_OUTPUT,
    AUDIO_VOL_HEADPHONE,
    AUDIO_VOL_HEADSET_MIC,

};

/** codec control info */

struct audio_codec_dev {
    uint8_t id;       ///< codec cluster number
    uint8_t i2c_addr; ///< i2c w/r address
    char domain[16];  // res domain
    uint32_t i2c_res_num;
    void *i2c_handle; ///< i2c resource num which to get i2c instance;
    enum audio_codec_input input_path;
    enum audio_codec_output output_path;
    int hphone_out_vol;
    int line_in_vol;
};

/** codec control driver interface  */
struct au_codec_dev_ctrl_interface {
    bool (*initialize)(struct audio_codec_dev *dev);
    bool (*start_up)(struct audio_codec_dev
                         *dev); ///< reset controller, set some common register;
    bool (*set_volume)(
        struct audio_codec_dev *dev, int volume_percent,
        enum audio_volume_type
            vol_type); ///< set volume both line out and phone out
    bool (*set_format)(struct audio_codec_dev dev,
                       pcm_params_t pcm_info); ///< set ms, interface mode;
    bool (*set_hw_params)(
        struct audio_codec_dev dev,
        pcm_params_t pcm_info); ///< set sample rate, slot width;
    bool (*trigger)(struct audio_codec_dev dev,
                    int cmd); ///< trigger start/stop and dir:playback/capture
    bool (*shutdown)(struct audio_codec_dev dev); ///< power down
    bool (*set_input_path)(
        struct audio_codec_dev *dev,
        uint32_t input_path); ///< choose input path: line in or mic in;
    bool (*set_output_path)(
        struct audio_codec_dev *dev,
        uint32_t output_path); ///< choose output path: line out or phone out
};

#endif
