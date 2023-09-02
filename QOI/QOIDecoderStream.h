//
// Created by colli on 5/31/2022.
//

#ifndef IMAGELOADER_QOIDECODERSTREAM_H
#define IMAGELOADER_QOIDECODERSTREAM_H

#include "QOI.h"

#include <queue>
#include <fstream>

class QOIDecoderStream {
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

    std::ifstream file;

    void decodeByte(const uint8_t& byte);
    std::queue<uint8_t> data;//stored as a flattened array [y][x][c]
    bool isEnd = false;

public:

    explicit QOIDecoderStream(const std::string& fileName);
    QOIDecoderStream() = default;

    std::array<uint8_t, 4> getNextPixel();
    bool eof();

    int getWidth();
    int getHeight();

};


#endif //IMAGELOADER_QOIDECODERSTREAM_H
