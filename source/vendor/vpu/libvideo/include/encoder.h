/*******************************************************************************
 *           Copyright (C) 2020 Semidrive Technology Ltd. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * FileName : encoder.h
 * Version  : 1.0.0
 * Purpose  : external apis for encoder
 * Authors  : wei.fan
 * Date     : 2021-06-25
 * Notes    :
 *
 ******************************************************************************/

#ifndef SEMIDRIVE_ENCODER
#define SEMIDRIVE_ENCODER

#ifdef __cplusplus
extern  "C"  {
#endif /* __cplusplus */

#include <stdbool.h>
#include <vputypes.h>

typedef enum {
    DIR_NONE,
    DIR_VER,
    DIR_HOR,
    DIR_DER_HOR
} MirDir;

typedef enum {
    ANGLE_NONE,
    ANGLE_90 = 90,
    ANGLE_180 = 180,
    ANGLE_270 = 270,
    ANGLE_MAX
} RotAngle;

typedef enum {
    CODEC_AVC,
    CODEC_MAX
} CodecType;

typedef enum {
    YUV420Planar,   //Three arrays Y,U,V.
    YUV420SemiPlanar, //Two arrays, one is all Y, the other is U and V
    NV21,
    NV12,
    FRAME_MAX
} YUVFormat;

typedef enum {
    RC_CQP = 0,  //RC is off
    RC_CBR,      //Constant Bit Rate
    RC_ABR,      //Average Bit Rate
    RC_CQM = 4,  //Constant Quality Mode
    RC_MAX
} RCMode;

typedef struct {
    unsigned int bitRate;
    unsigned int frameRate; // real frameRate = frameRate & 0xFFFF/(frameRate>>16 + 1)
    unsigned int profile;   // -1, means default value
    unsigned int level;     // -1 , means defautl value
    unsigned int picWidth;
    unsigned int picHeight;
    YUVFormat srcFormat;
    RCMode rcMode;
} EncBaseConfig;

typedef struct {
    unsigned char* buf; //[IN]: buf used to save head info
    unsigned int bufSize; //[IN]: buf size of this buf
    unsigned int bitstreamSize; //[OUT]:  the head info data size gotten from VPU
} EncHeaderData;

typedef struct {
    unsigned char* buf; //[IN] the buf used to save yuv data
    unsigned int dataSize; //[Out] the data size stored in buf
    YUVFormat format; // the YUV format
} EncInputFrame;

typedef struct {
    unsigned char * buf;     //[IN] the buf used to save encoded data
    unsigned int bufSize;  //[IN]] the buf siz
    unsigned int bitstreamSize;//[Out] the encoded data size
} EncOutputBuffer;

/**************************************************************************/ /*!
@Function   changeBitRate
@Description change the bitrate of bitstream
it should be called in the same thread with encodeOneFrame;

@Input pEncContext:  encoder context pointer
****************************************************************************/
void changeBitRate(void* pEncContext, unsigned int bitRate);
/**************************************************************************/ /*!
@Function  changeFrameRate
@Description change the frame rate of encoded bitstream
it should be called in the same thread with encodeOneFrame;

@Input pEncContext:  encoder context pointer
****************************************************************************/
void changeFrameRate(void* pEncContext, unsigned char frameRate);
/**************************************************************************/ /*!
@Function isEncoderReady
@Description check if the encoder is in idle

@Input pEncContext:  encoder context pointer
@Return true : encoder is in idle state
****************************************************************************/
bool isEncoderReady(void* pEncContext);
/**************************************************************************/ /*!
@Function   setFrameRotationAngle
@Description set frame rotation angle
it should be called in the same thread with encodeOneFrame;
@Input pEncContext:  encoder context pointer
@Input RotAngle : the rotation angle
****************************************************************************/
void setFrameRotationAngle(void* pEncContext, RotAngle rotAngle);
/**************************************************************************/ /*!
@Function   setFrameMirroDirection
@Description  change the frame to mirror direction
it should be called in the same thread with encodeOneFrame;

@Input pEncContext:  encoder context pointer
@Input mirDir:  Mirorr direction (VER, HOR, VER_HOR)
****************************************************************************/
void setFrameMirDir(void* pEncContext, MirDir mirDir);
/**************************************************************************/ /*!
@Function   requestIFrame
@Description force the encoder to generate one I frame , when it is called,
the next encoding process would generate I frame once.
it should be called in the same thread with encodeOneFrame;

@Input pEncContext:  encoder context pointer
****************************************************************************/
void requestIFrame(void* pEncContext);
/**************************************************************************/ /*!
@Function   getBitRate
@Description  get the bit rate used in encoder

@Input pEncContext:  encoder context pointer
@Return  the bitrate used in encoder
****************************************************************************/
unsigned int getBitRate(void* pEncContext);
/**************************************************************************/ /*!
@Function   getFrameRate
@Description  get the frame rate used in encoder

@Input pEncContext:  encoder context pointer
@Return  the framerate used in encoder
****************************************************************************/
unsigned char getFrameRate(void* pEncContext);
/**************************************************************************/ /*!
@Function    getRatePolicy
@Description  get the rate control mode used in encoder

@Input pEncContext:  encoder context pointer
@Return  the rate control mode used in encoder
****************************************************************************/
unsigned int getRatePolicy(void* pEncContext);
/**************************************************************************/ /*!
@Function    encInit
@Description create an encoder instance and intialize it .

@Input eCodec, CodecType:  AVC/H264
@Input pEncBaseConfig , the pointer which points to the EncBaseConfig,

@Return  an pointer which points to EncContext
****************************************************************************/
void* encInit(CodecType eCodec, EncBaseConfig* pEncBaseConfig);
/**************************************************************************/ /*!
@Function   encOneFrame
@Description  encoding one frame ,which is passed by addOneSrcFrame

@Input pEncContext:  encoder context pointer
@Input srcFrameIdx:  it means the source frame buffer which is being used in vpu
it is the return valule of addOneSrcFrame
@Return
 0 means encoding is completed
 -1 means encoding failed because of timeout
****************************************************************************/
int encOneFrame(void* pEncContext, int srcFrameIdx);
/**************************************************************************/ /*!
@Function    encUninit
@Description close the encoder created by encInit.

@Input  pEncContext: encoder context pointer
@Return
 true means the encoder is closed.
 false means encoder is still in encoding.
 it SHOULD be called again after getOneBitsStreamFrame is called.
****************************************************************************/
bool encUninit(void* pEncContext);
/**************************************************************************/ /*!
@Function   addOneSrcFrame
@Description fill yuv data into encoder

@Input pEncContext: encoder context pointer
@Input pInputFrame: the input frame which is needed to encode
@Return
 the index of source frame buffer which store the yuv data in vpu
****************************************************************************/
int addOneSrcFrame(void* pEncContext, EncInputFrame *pInputFrame);
/**************************************************************************/ /*!
@Function  getOneBitstreamFrame
@Description get the endcoded data from encoder

@Input pEncContext: encoder context pointer
@Input  pOutputBuffer: the output buffer which is used to store the encoded data

@Return
 -1, the data is not read out.
 0, the data is read out from vpu
****************************************************************************/
int getOneBitstreamFrame(void* pEncContext, EncOutputBuffer* pOutputBuffer);
/**************************************************************************/ /*!
@Function   getHeaderInfo
@Description get sps/pps info for this bitstream , now we just supported avc/h264
codec

@Input pEncContext: encoder context pointer
@Input  pHeaderData: the output buffer which is used to store the sps/pps data
the buf should be big enough
@Return
 -1, the data is not read out.
 0, the data is read out from vpu
****************************************************************************/
int getHeaderInfo(void* pEncContext, EncHeaderData* pHeaderData);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
