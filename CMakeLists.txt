cmake_minimum_required(VERSION 3.10)
project("explore_gstreamer")

find_package(PkgConfig REQUIRED)
pkg_check_modules(gst REQUIRED IMPORTED_TARGET gstreamer-1.0)

add_executable(basic-t1 basic_tutorials/t1.c)
target_link_libraries(basic-t1 PkgConfig::gst)

