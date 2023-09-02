cmake_minimum_required(VERSION 3.20)
project(ImageLoader)

set(CMAKE_CXX_STANDARD 14)

add_executable(ImageLoader main.cpp stb_image.h stb_image_write.h QOI/QOIEncoder.cpp QOI/QOIEncoder.h QOI/QOI.cpp QOI/QOI.h QOI/QOIDecoder.cpp QOI/QOIDecoder.h ColorSpaces.cpp ColorSpaces.h QOI/QOIDecoderStream.cpp QOI/QOIDecoderStream.h)
