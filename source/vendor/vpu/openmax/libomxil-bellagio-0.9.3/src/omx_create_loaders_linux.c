/**
  src/omx_create_loaders_linux.c

  In this file is implemented the entry point for the construction
  of every component loader in linux. In the current implementation
  only the ST static loader is called.

  Copyright (C) 2007-2010 STMicroelectronics
  Copyright (C) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).

  This library is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation; either version 2.1 of the License, or (at your option)
  any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
  details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA
  02110-1301  USA

*/

#define _GNU_SOURCE
#include <stdlib.h>
#if defined(WIN32)
#else
#include <dlfcn.h>
#endif
#include <dirent.h>
#include <string.h>
#include "component_loader.h"
#include "omx_create_loaders.h"
#include "st_static_component_loader.h"
#include "common.h"

#define OMX_LOADERS_FILENAME ".omxloaders"

#ifndef OMX_LOADERS_DIRNAME
#define OMX_LOADERS_DIRNAME "/usr/lib/omxloaders/"
#endif

#ifndef INSTALL_PATH_STR
#define INSTALL_PATH_STR "/usr/local/lib/bellagio"
#endif

/**
 * This function allocates all the structures needed by the component loaders
 * available in the system and initialize the function pointers of the loader.
 * Finally it searches for more loaders that can be pointed by the .omxloaders
 * file. If this file exists it contains a list of library containing custom
 * component loaders. Each library should contain a function named 'setup_component_loader'
 * that will initialize the custom loader. This mechanism is similar to the
 * default loading of the components.
 *
 */
int createComponentLoaders() {
	// load component loaders
	BOSA_COMPONENTLOADER *loader;
	DIR *dirp_name;
	struct dirent *dp;
	void *handle;
	void *functionPointer;
	void (*fptr)(BOSA_COMPONENTLOADER *loader);
	char *libraryFileName = NULL;
	FILE *loaderFP;
	char *omxloader_registry_filename = NULL;
	int onlyDefault = 0;
	int index_readline;
	int isFileExisting = 0;
	int isDirExisting = 0;
	OMX_ERRORTYPE ret = OMX_ErrorNone;

	omxloader_registry_filename = loadersRegistryGetFilename(OMX_LOADERS_FILENAME);

	isFileExisting = exists(omxloader_registry_filename);

	isDirExisting = exists(OMX_LOADERS_DIRNAME);

	/* test the existence of the file */
	if (!isDirExisting && !isFileExisting) {
		onlyDefault = 1;
	}

	if (onlyDefault) {
		loader = malloc(sizeof(BOSA_COMPONENTLOADER));
		if (loader == NULL) {
			DEBUG(DEB_LEV_ERR, "not enough memory for this loader\n");
			ret = OMX_ErrorInsufficientResources;
			goto DONE;
		}
		memset(loader, 0x00, sizeof(BOSA_COMPONENTLOADER));

		st_static_setup_component_loader(loader);
		BOSA_AddComponentLoader(loader);
		goto DONE;
	}
	if (isFileExisting) {
		loaderFP = fopen(omxloader_registry_filename, "r");
		// dlopen all loaders defined in .omxloaders file
		libraryFileName = malloc(MAX_LINE_LENGTH);
		if (!libraryFileName) {
			DEBUG(DEB_LEV_ERR, "not enough memory for this loader\n");
			ret = OMX_ErrorInsufficientResources;
			goto DONE;
		}
		while(1) {
			index_readline = 0;
			while(index_readline < MAX_LINE_LENGTH) {
				*(libraryFileName + index_readline) = fgetc(loaderFP);
				if ((*(libraryFileName + index_readline) == '\n') || (*(libraryFileName + index_readline) == '\0')) {
					break;
				}
				index_readline++;
			}
			*(libraryFileName + index_readline) = '\0';
			if ((index_readline >= MAX_LINE_LENGTH) || (index_readline == 0)) {
				break;
			}
#if defined(WIN32)
			handle = LoadLibrary(libraryFileName);
#else
			handle = dlopen(libraryFileName, RTLD_NOW);
#endif
		    if (!handle) {
#if defined(WIN32)
				DEBUG(DEB_LEV_ERR, "library %s dlopen error\n", libraryFileName);
#else
				DEBUG(DEB_LEV_ERR, "library %s dlopen error: %s\n", libraryFileName, dlerror());
#endif
				continue;
		    }
#if defined(WIN32)
			if ((functionPointer = (void *)GetProcAddress(handle, "setup_component_loader")) == NULL)
#else
		    if ((functionPointer = dlsym(handle, "setup_component_loader")) == NULL)
#endif
			{
#if defined(WIN32)
				DEBUG(DEB_LEV_ERR, "the library %s is not compatible\n", libraryFileName);
#else
					DEBUG(DEB_LEV_ERR, "the library %s is not compatible - %s\n", libraryFileName, dlerror());
#endif
					continue;
			}
		    fptr = functionPointer;

			loader = malloc(sizeof(BOSA_COMPONENTLOADER));
			if (loader == NULL) {
				DEBUG(DEB_LEV_ERR, "not enough memory for this loader\n");
				ret = OMX_ErrorInsufficientResources;
				goto DONE;
			}

			memset(loader, 0x00, sizeof(BOSA_COMPONENTLOADER));

			/* setup the function pointers */
			(*fptr)(loader);

			/* add loader to core */
			BOSA_AddComponentLoader(loader);
		}
		fclose(loaderFP);
		goto DONE;
	}

	if (isDirExisting) {
		dirp_name = opendir(OMX_LOADERS_DIRNAME);
		while ((dp = readdir(dirp_name)) != NULL) {
			int len = strlen(dp->d_name);
			if(len >= 3) {
				if(strncmp(dp->d_name+len-3, ".so", 3) == 0){
#if defined(WIN32)
					char lib_absolute_path[512];
#else
					char lib_absolute_path[strlen(OMX_LOADERS_DIRNAME) + len + 1];
#endif
					strcpy(lib_absolute_path, OMX_LOADERS_DIRNAME);
					strcat(lib_absolute_path, dp->d_name);
#if defined(WIN32)
					handle = LoadLibrary(libraryFileName);
#else
					handle = dlopen(lib_absolute_path, RTLD_NOW);
#endif
					if (!handle) {
#if defined(WIN32)
						DEBUG(DEB_LEV_ERR, "library %s dlopen error\n", lib_absolute_path);
#else
						DEBUG(DEB_LEV_ERR, "library %s dlopen error: %s\n", lib_absolute_path, dlerror());
#endif
						continue;
					}
#if defined(WIN32)
					if ((functionPointer = (void *)GetProcAddress(handle, "setup_component_loader")) == NULL)
#else
					if ((functionPointer = dlsym(handle, "setup_component_loader")) == NULL)
#endif
					{
#if defined(WIN32)
						DEBUG(DEB_LEV_ERR, "the library %s is not compatible\n", lib_absolute_path);
#else
						DEBUG(DEB_LEV_ERR, "the library %s is not compatible - %s\n", lib_absolute_path, dlerror());
#endif
						continue;
					}
					fptr = functionPointer;
					loader = malloc(sizeof(BOSA_COMPONENTLOADER));
					if (loader == NULL) {
						DEBUG(DEB_LEV_ERR, "not enough memory for this loader\n");
						ret = OMX_ErrorInsufficientResources;
						goto DONE;
					}
					memset(loader, 0x00, sizeof(BOSA_COMPONENTLOADER));

					/* setup the function pointers */
					(*fptr)(loader);
					/* add loader to core */
					BOSA_AddComponentLoader(loader);
#if defined(WIN32)
					FreeLibrary(handle);
#else
					dlclose(handle);
#endif
				}
			}
		}
		closedir(dirp_name);
		goto DONE;
	}
	/* add the ST static component loader */
	loader = malloc(sizeof(BOSA_COMPONENTLOADER));
	if (loader == NULL) {
		DEBUG(DEB_LEV_ERR, "not enough memory for this loader\n");
		ret = OMX_ErrorInsufficientResources;
		goto DONE;
	}
	memset(loader, 0x00, sizeof(BOSA_COMPONENTLOADER));

	st_static_setup_component_loader(loader);
	BOSA_AddComponentLoader(loader);

DONE:
	if (omxloader_registry_filename) {
		free(omxloader_registry_filename);
		omxloader_registry_filename = NULL;
	}

	if (libraryFileName) {
		free(libraryFileName);
		libraryFileName = NULL;
	}
	return ret;
}
