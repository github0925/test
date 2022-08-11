FILESEXTRAPATHS_prepend := "${THISDIR}/patch:"

SRC_URI_append := "file://0001-present-don-t-do-a-vblank-abort-on-a-pending-flip-du.patch  \
		file://0002-Allow-selection-of-the-primary-bus-device-driver.patch      \
		file://0003-glamor-improve-OpenGLES-support.patch                       \
		file://0004-glamor-if-no-DRI2-don-t-call-DRI2CloseScreen.patch          \
		file://0005-glamor-add-glsl-shaders-for-OpenGLES-3.0.patch              \
		file://0006-glamor-fix-SW-cursor-handling-for-IMG-OpenGLES3.patch       \
		file://0007-glamor-fix-a-failure-path-in-glamor_upload_picture_t.patch  \
		file://0008-modesetting-add-option-to-not-render-a-SW-cursor.patch      \
		file://0012-config-udev-Include-header-sys-sysmacros.h-to-use-ma.patch  \
                file://0013-glamor-fix-xv-can-not-work-issue-for-GLES-3.0.patch         \
"
