//
// Created by colli on 5/31/2022.
//

#ifndef IMAGELOADER_QOIDECODER_H
#define IMAGELOADER_QOIDECODER_H


#include "QOI.h"
#include <string>


class QOIDecoder {
private:
    enum Tasks{
        HEADER,
        TAG,
        RGB,
        RGBA,
        INDEX,
        DIFF,
        LUMA,
        RUN
    };

    QOI::QOIHeader header;
    std::array<std::array<uint8_t, 4>, 64> seenColors = {{0, 0, 0, 0}};
    std::array<uint8_t, 4> lastColor = {0, 0, 0, 255};
    Tasks task = HEADER;

    std::vector<uint8_t> buffer;

    void readHeader(const uint8_t& byte);
    void readTag(const uint8_t& byte);
    void readRGB(const uint8_t& byte);
    void readRGBA(const uint8_t& byte);
    void readIndex(const uint8_t& byte);
    void readDiff(const uint8_t& byte);
    void readLuma(const uint8_t& byte);
    void readRun(const uint8_t& byte);

public:
    std::vector<uint8_t> data;//stored as a flattened array [y][x][c]

    explicit QOIDecoder(const std::string& fileName);
    QOIDecoder() = default;

    void decodeByte(const uint8_t& byte);

    void saveAsPNG(const std::string& fileName);
    void saveAsBMP(const std::string& fileName);
};


#endif //IMAGELOADER_QOIDECODER_H
