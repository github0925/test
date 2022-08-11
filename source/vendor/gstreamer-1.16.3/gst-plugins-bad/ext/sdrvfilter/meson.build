sdrvfilter_sources = [
  'gstsdrvfilter.c',
  'gstsdrvfilterpool.c',
  'gstg2dapi.c',
  'gstdrmutils.c',
]

extra_c_args = []

optional_deps = []

libdrm_dep = dependency('libdrm', version : '>= 2.4.55', required : false)
if libdrm_dep.found()
  optional_deps += libdrm_dep
endif

gst_sdrvfilter_args = ['-DHAVE_CONFIG_H']
gstsdrvfilter = library('gstsdrvfilter',
  sdrvfilter_sources,
  c_args : gst_sdrvfilter_args + extra_c_args,
#  link_args : noseh_link_args,
  include_directories : [configinc],
  dependencies : [gstvideo_dep, gstaudio_dep, gstbase_dep, gstcontroller_dep,
                  libm, gmodule_dep, gstallocators_dep] + optional_deps,
  install : true,
  install_dir : plugins_install_dir,
)