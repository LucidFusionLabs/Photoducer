# $Id: CMakeLists.txt 1320 2014-10-26 08:12:52Z justin $

lfl_project(image)
lfl_add_target(image EXECUTABLE SOURCES image.cpp
               LINK_LIBRARIES ${LFL_APP_LIB} ${GTEST_LIB} ${LFL_APP_FRAMEWORK} ${LFL_APP_GRAPHICS}
               app_null_audio app_null_camera app_opencv_matrix app_null_fft app_simple_loader
               app_simple_resampler app_iconv_convert app_stdregex_regex app_null_sregex
               app_libpng_png app_null_jpeg app_null_gif app_null_ogg app_libcss_css app_null_ttf
               app_null_ssl app_null_js app_null_tu ${LFL_APP_OS})
set(LFL_APP_LIB_FILES ${OPENCV_LIB_FILES})
lfl_post_build_start(image image LImage)
