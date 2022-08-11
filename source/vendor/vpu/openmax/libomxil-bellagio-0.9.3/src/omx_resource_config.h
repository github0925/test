/*******************************************************************************
 *           Copyright (C) 2020 Semidrive Technology Ltd. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * FileName : omx_resource_config.h
 * Version  : 1.0.0
 * Purpose  : omx component resource configs
 * Authors  : jun.jin
 * Date     : 2020-07-29
 * Notes    :
 *
 ******************************************************************************/


/*---------------------------- include head file -----------------------------*/


#ifndef _OMX_RESOURCE_CONFIG_H_
#define _OMX_RESOURCE_CONFIG_H_

#define MAX_COMPONENTS_CAPACITY 1.0f

typedef struct
{
    // name of components
    char  *name;

    // component host index
    int   core_index;

    // max num of components
    int   max_num;
} comp_resouce;


typedef struct
{
    // the decodeing settings of component
    int   width;
    int   height;

   // the weight of current component
    float  weight;
}comp_weight_configs;



float resGetCorrespondWeight(const char *name, int width, int height, int framerate);
void resGetCorrespondConfig(const char *name, int *max_num);
int resGetCorrespondCoreIndex(const char *name);
float resNormalizeWeight(float weight);

#endif