//
// Created by colli on 5/31/2022.
//

#ifndef IMAGELOADER_QOI_H
#define IMAGELOADER_QOI_H


#include <vector>
#include <cstdint>
#include <array>
#include <string>

namespace QOI{

    struct __attribute__ ((packed)) QOIHeader {
        char magic[4] = {'q','o','i','f'};// magic bytes "qoif"
        uint32_t width;// image width in pixels (Big Endian)
        uint32_t height;// image height in pixels (Big Endian)
        uint8_t channels;// 3 = RGB, 4 = RGBA
        uint8_t colorspace;// 0 = sRGB with linear alpha
        // 1 = all channels linear
        // colorspace does not affect storage


        std::array<uint8_t, 14> getData() const;
    };

    struct __attribute__ ((packed)) ColorRGBChunk{
        const uint8_t tag = 0b11111110;
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        ColorRGBChunk(uint8_t color[3]);
    };

    struct __attribute__ ((packed)) ColorRGBAChunk{
        const uint8_t tag = 0b11111111;
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        uint8_t a = 0;
        ColorRGBAChunk(uint8_t color[4]);
    };

    struct __attribute__ ((packed)) IndexChunk{
        uint8_t getIndex();
        void setIndex(uint8_t index);
        IndexChunk(uint8_t index);
        uint8_t data = 0b00000000;
    };

    struct __attribute__ ((packed)) DifferenceChunk{
        std::array<int8_t, 3> getDifference();
        bool setDifference(int16_t dr, int16_t dg, int16_t db);
        uint8_t data = 0b01000000;
    };

    struct __attribute__ ((packed)) LumaChunk{
        std::array<int8_t, 3> getDifference();
        bool setDifference(int16_t dr, int16_t dg, int16_t db);
        uint16_t data = 0b0000000010000000;//this is Big Endian (it is managed properly)
    };

    struct __attribute__ ((packed)) RunChunk{
        uint8_t getLength();
        bool setLength(uint8_t length);
        uint8_t data = 0b11000000;
        explicit RunChunk(uint8_t length);
    };

    uint8_t colorHash(std::array<uint8_t, 3> color);
    uint8_t colorHash(std::array<uint8_t, 4> color);
    uint8_t colorHash(const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& a = 255);

}

#endif //IMAGELOADER_QOI_H
