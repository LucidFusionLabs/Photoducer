# $Id: CMakeLists.txt 1320 2014-10-26 08:12:52Z justin $

lfl_project(image)
add_definitions(${LFL_APP_DEF})
include_directories(${LFL_APP_INCLUDE})

add_executable(image image.cpp)
target_link_libraries(image ${LFL_APP_LIB} ${GTEST_LIB} ${LFL_APP_OS} ${LFL_APP_FRAMEWORK} ${LFL_APP_GRAPHICS}
                      app_null_audio app_null_camera app_opencv_matrix app_null_fft app_simple_loader
                      app_simple_resampler app_iconv_convert app_stdregex_regex app_null_sregex
                      app_libpng_png app_null_jpeg app_null_gif app_null_ogg app_libcss_css app_null_ttf
                      app_null_ssl app_null_js app_null_tu)
lfl_post_build_start(image image LImage)

include_directories(${GTEST_INCLUDE})
add_executable(gl_tests gl_tests.cpp)
add_dependencies(gl_tests image)
target_link_libraries(gl_tests ${LFL_APP_LIB} ${GTEST_LIB} app_null_os app_null_framework app_null_graphics
                      app_null_audio app_null_camera app_opencv_matrix app_null_fft app_simple_loader
                      app_simple_resampler app_iconv_convert app_stdregex_regex app_null_sregex
                      app_libpng_png app_null_jpeg app_null_gif app_null_ogg app_libcss_css app_null_ttf
                      app_null_ssl app_null_js app_null_tu)
