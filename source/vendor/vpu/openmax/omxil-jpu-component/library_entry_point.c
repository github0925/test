#include <omx_mjpeg_utils.h>
#include <omx_mjpegdec_component.h>
#include <st_static_component_loader.h>

/** The library entry point. It must have the same name for each
 * library of the components loaded by the ST static component loader.
 *
 * This function fills the version, the component name and if existing also the roles
 * and the specific names for each role. This base function is only an explanation.
 * For each library it must be implemented, and it must fill data of any component
 * in the library
 *
 * @param stComponents pointer to an array of components descriptors.If NULL, the
 * function will return only the number of components contained in the library
 *
 * @return number of components contained in the library
 */

#define NUMBER_OF_COMPONENTS 2

int omx_component_library_Setup(stLoaderComponentType **stComponents)
{
  DEBUG(DEB_LEV_FUNCTION_NAME, "In %s \n", __func__);
  if (stComponents == NULL)
  {
    DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s \n", __func__);
    return NUMBER_OF_COMPONENTS; // Return Number of Component/s
  }
  stComponents[0]->componentVersion.s.nVersionMajor = 1;
  stComponents[0]->componentVersion.s.nVersionMinor = 1;
  stComponents[0]->componentVersion.s.nRevision = 1;
  stComponents[0]->componentVersion.s.nStep = 1;

  stComponents[0]->name = calloc(1, OMX_MAX_STRINGNAME_SIZE);
  if (stComponents[0]->name == NULL)
  {
    return OMX_ErrorInsufficientResources;
  }
  strcpy(stComponents[0]->name, JPU_DECODER_NAME);
  stComponents[0]->name_specific_length = 1;
  stComponents[0]->constructor = omx_mjpegdec_component_Constructor;

  stComponents[0]->name_specific = calloc(stComponents[0]->name_specific_length, sizeof(char *));
  stComponents[0]->role_specific = calloc(stComponents[0]->name_specific_length, sizeof(char *));

  stComponents[0]->name_specific[0] = calloc(1, OMX_MAX_STRINGNAME_SIZE);
  if (stComponents[0]->name_specific[0] == NULL)
  {
    return OMX_ErrorInsufficientResources;
  }
  stComponents[0]->role_specific[0] = calloc(1, OMX_MAX_STRINGNAME_SIZE);
  if (stComponents[0]->role_specific[0] == NULL)
  {
    return OMX_ErrorInsufficientResources;
  }

  strcpy(stComponents[0]->name_specific[0], JPU_DECODER_NAME);
  strcpy(stComponents[0]->role_specific[0], JPU_DECODER_ROLE);

  stComponents[1]->componentVersion.s.nVersionMajor = 1;
  stComponents[1]->componentVersion.s.nVersionMinor = 1;
  stComponents[1]->componentVersion.s.nRevision = 1;
  stComponents[1]->componentVersion.s.nStep = 1;

  stComponents[1]->name = calloc(1, OMX_MAX_STRINGNAME_SIZE);
  if (stComponents[1]->name == NULL)
  {
    return OMX_ErrorInsufficientResources;
  }
  strcpy(stComponents[1]->name, JPU_ENCODER_NAME);
  stComponents[1]->name_specific_length = 1;
  stComponents[1]->constructor = NULL; //omx_mjpegenc_component_Constructor;

  stComponents[1]->name_specific = calloc(stComponents[1]->name_specific_length, sizeof(char *));
  stComponents[1]->role_specific = calloc(stComponents[1]->name_specific_length, sizeof(char *));

  stComponents[1]->name_specific[0] = calloc(1, OMX_MAX_STRINGNAME_SIZE);
  if (stComponents[1]->name_specific[0] == NULL)
  {
    return OMX_ErrorInsufficientResources;
  }
  stComponents[1]->role_specific[0] = calloc(1, OMX_MAX_STRINGNAME_SIZE);
  if (stComponents[1]->role_specific[0] == NULL)
  {
    return OMX_ErrorInsufficientResources;
  }

  strcpy(stComponents[1]->name_specific[0], JPU_ENCODER_NAME);
  strcpy(stComponents[1]->role_specific[0], JPU_ENCODER_ROLE);

  DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s \n", __func__);
  return 2;
}
