//
// Created by colli on 5/31/2022.
//

#ifndef IMAGELOADER_QOIENCODER_H
#define IMAGELOADER_QOIENCODER_H

#include "QOI.h"

class QOIEncoder {
private:
    std::vector<uint8_t> data;
    int runLength = 0;
    QOI::QOIHeader header;
    std::array<std::array<uint8_t, 4>, 64> seenColors = {{0, 0, 0, 0}};
    std::array<uint8_t, 4> lastColor = {0, 0, 0, 255};
    bool isEncodingOver = false;
    long long posX = 0;
    long long posY = 0;

public:

    QOIEncoder(const int& width, const int& height, const int& nrChannels);

    void encodePixel(const uint8_t& r, const uint8_t& g, const uint8_t& b);
    void encodePixel(const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& a);
    void encodePixel(const std::array<uint8_t, 3>& color);
    void encodePixel(const std::array<uint8_t, 4>& color);
    void endEncoding();

    void saveToFile(const std::string& fileName);

};


#endif //IMAGELOADER_QOIENCODER_H
