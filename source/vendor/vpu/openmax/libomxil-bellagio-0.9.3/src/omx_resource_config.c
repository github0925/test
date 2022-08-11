/*******************************************************************************
 *           Copyright (C) 2020 Semidrive Technology Ltd. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * FileName : omx_resource_config.c
 * Version  : 1.0.0
 * Purpose  : omx component resource configs
 * Authors  : jun.jin
 * Date     : 2020-07-29
 * Notes    :
 *
 ******************************************************************************/


/*---------------------------- include head file -----------------------------*/
#include <string.h>
#include <OMX_Types.h>
#include "omx_resource_config.h"
#include "omx_comp_debug_levels.h"

#define VPU_IDX_CODA 0   // index for coda
#define VPU_IDX_WAVE 1   // index for wave

#define PROP_RESOURCE_MANAGER "sys.omx.res.manager"

// define the related componet resources
static comp_resouce comp_resource_configs[] = {
    {"OMX.vpu.video_decoder.avc",   VPU_IDX_CODA, 16},
    {"OMX.vpu.video_decoder.vp9",   VPU_IDX_WAVE, 16},
    {"OMX.vpu.video_decoder.vp8",   VPU_IDX_CODA, 16},
    {"OMX.vpu.video_decoder.h263",  VPU_IDX_CODA, 16},
    {"OMX.vpu.video_decoder.hevc",  VPU_IDX_WAVE, 16},
    {"OMX.vpu.video_decoder.mpeg2", VPU_IDX_CODA, 16},
    {"OMX.vpu.video_decoder.mpeg4", VPU_IDX_CODA, 16},
    {"OMX.vpu.video_encoder.avc",   VPU_IDX_CODA, 16},
    {"OMX.vpu.video_encoder.h263",  VPU_IDX_CODA, 16},
    {"OMX.vpu.video_encoder.mpeg4", VPU_IDX_CODA, 16},
};


// the weights are defined wiht fps 30, if the actual fps is not 30, it will be recalculated in the code
// limit by weights
static comp_weight_configs limited_weight_configs[] = {
    {800,  480,  0.1},
    {1280, 720,  0.15},
    {1920, 1080, 0.25},
    {2560, 1440, 0.3},
    {3840, 2160, 0.75},
    {4096, 2304, 0.75},
};


// for these componsts, limited by the num
const char *unlimited_components[] = {
    "OMX.vpu.video_decoder.vp9",
    "OMX.vpu.video_decoder.hevc"
};


static comp_weight_configs unlimited_weight_configs[] = {
    {800,  480,  0.1},
    {1280, 720,  0.1},
    {1920, 1080, 0.1},
    {2560, 1440, 0.1},
    {3840, 2160, 0.1},
    {4096, 2304, 0.1},
};

static int  resGetWeightConfigIndex(comp_weight_configs *weight_array, int config_len, int width, int height);
static comp_weight_configs* resGetRelateWeightTable(const char *name, int *num);
static OMX_BOOL resManagerEnable();

float resNormalizeWeight(float weight)
{
    if(weight > 1.0f) {
        weight = 1.0f;
    } else if(weight < 0.0f) {
        weight = 0.0f;
    }

    return weight;
}


float resGetCorrespondWeight(const char *name, int width, int height, int framerate)
{
    int selected = 0, weight_num = 0;
    float weight = 0.0f;
    comp_weight_configs *component_weight = NULL;

    if(!resManagerEnable()) {
        //if res manager is not enabled, return with 0.0f, so the caller can by pass the limit of weight
        return weight;
    }
    component_weight = resGetRelateWeightTable(name, &weight_num);
    selected = resGetWeightConfigIndex(component_weight, weight_num, width, height);
    weight = component_weight[selected].weight;

    DEBUG(DEB_LEV_FULL_SEQ, "%s current componet is %s, before with weight %f, framerate = %d, width = %d",  __func__, name, weight, framerate, width);
    // if current framerate is big than 30fps, a extra factor will be multiplied
    if((framerate != 0) && (framerate <= 120) && (framerate != 30)) {
        weight = (framerate / 30.0f)  * weight;
    }

    DEBUG(DEB_LEV_FULL_SEQ, "%s current componet is %s, with weight %f",  __func__, name, weight);

    return resNormalizeWeight(weight);
}


int resGetCorrespondCoreIndex(const char *name)
{
    int i = 0, core_index = 0, res_num = 0;
    res_num = sizeof(comp_resource_configs)/sizeof(comp_resouce);

    for(i = 0; i < res_num; i++) {
        if(!strcmp(comp_resource_configs[i].name, name)) {
            core_index = comp_resource_configs[i].core_index;
            break;
        }
    }

    return core_index;
}


void resGetCorrespondConfig(const char *name, int *max_num)
{
    int i = 0, res_num = 0;
    res_num = sizeof(comp_resource_configs)/sizeof(comp_resouce);

    // max_num defalut to 4, and capacity default to 1.0f
     *max_num  = 4;

    for(i = 0; i < res_num; i++) {
        if(!strcmp(comp_resource_configs[i].name, name)) {
            *max_num  = comp_resource_configs[i].max_num;
            break;
        }
    }
}


comp_weight_configs* resGetRelateWeightTable(const char *name, int *num)
{
    int i = 0;
    int unlimited_num = sizeof(unlimited_components)/sizeof(char*);

    for(i = 0; i < unlimited_num; i++) {
        if(!strcmp(unlimited_components[i], name)) {
            DEBUG(DEB_LEV_FULL_SEQ, "%s current componet is %s, not limited",  __func__, name);
            break;
        }
    }

    if(i < unlimited_num) {
        *num = sizeof(unlimited_weight_configs)/sizeof(comp_weight_configs);
        return unlimited_weight_configs;
    } else {
        *num = sizeof(limited_weight_configs)/sizeof(comp_weight_configs);
        return limited_weight_configs;
    }
}


int  resGetWeightConfigIndex(comp_weight_configs *weight_array, int config_len, int width, int height __attribute__((unused)))
{
    int i = 0;
    int weight_index = config_len - 1; // defalut to the max resolution

    // find the first one greater than or equal to the current resolution
    for(i = 0; i < config_len;  i++) {
        if(weight_array[i].width >= width) {
            weight_index = i;
            break;
        }
    }

    DEBUG(DEB_LEV_FULL_SEQ, "%s return with index = %d",  __func__, weight_index);
    return weight_index;
}

OMX_BOOL resManagerEnable()
{
#ifdef ANDROID
    char value[PROPERTY_VALUE_MAX] = {0};

    // default is true, if user want to disable, should init this prop value to false
    property_get(PROP_RESOURCE_MANAGER, value, "true");
    DEBUG(DEB_LEV_SIMPLE_SEQ, "%s %s\n", PROP_RESOURCE_MANAGER, value);

    if(!strcmp(value, "true")) {
        return OMX_TRUE;
    } else {
        return OMX_FALSE;
    }
#else
    return OMX_TRUE;
#endif
}
