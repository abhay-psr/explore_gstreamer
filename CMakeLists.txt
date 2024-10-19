cmake_minimum_required(VERSION 3.10)
project("explore_gstreamer")

find_package(PkgConfig REQUIRED)
pkg_check_modules(gst REQUIRED IMPORTED_TARGET gstreamer-1.0)

add_executable(basic-t1 basic_tutorials/t1.c)
target_link_libraries(basic-t1 PkgConfig::gst)

add_executable(basic-t2 basic_tutorials/t2.c)
target_link_libraries(basic-t2 PkgConfig::gst)
add_executable(basic-t2_1 basic_tutorials/t2_1.c)
target_link_libraries(basic-t2_1 PkgConfig::gst)

add_executable(basic-t3 basic_tutorials/t3.c)
target_link_libraries(basic-t3 PkgConfig::gst)
