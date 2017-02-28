cmake_minimum_required (VERSION 3.0)
project (AppleCameraDemo)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

set(OpenCV_DIR ${CMAKE_CURRENT_SOURCE_DIR}/usr/lib/opencv)

find_package(OpenCV REQUIRED)
find_package(Gphoto2 REQUIRED)
find_package(libraw REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${GPHOTO2_INCLUDE_DIR})
include_directories(${LibRaw_INCLUDE_DIR})

add_executable(camera_demo
    camera_demo.cpp
)

target_link_libraries(camera_demo ${OpenCV_LIBS})
target_link_libraries(camera_demo ${GPHOTO2_LIBRARY})
target_link_libraries(camera_demo ${LibRaw_LIBRARIES})
