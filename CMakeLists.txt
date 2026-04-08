cmake_minimum_required(VERSION 3.10)

project(ConvolutionTests)

set(CMAKE_C_STANDARD 99)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenCV REQUIRED core imgproc imgcodecs)

if(OpenCV_FOUND)
    message(STATUS "OpenCV found: ${OpenCV_VERSION}")
else()
    message(FATAL_ERROR "OpenCV not found. Please install libopencv-dev.")
endif()

add_executable(opencv_reference tests/opencv_reference.cpp)

target_link_libraries(opencv_reference ${OpenCV_LIBS})

add_executable(grayscale src/grayscale.c)

target_link_libraries(grayscale m)

add_executable(test_compare tests/test.c)

target_link_libraries(test_compare m)
