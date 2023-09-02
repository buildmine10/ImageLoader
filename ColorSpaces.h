//
// Created by colli on 5/31/2022.
//

#ifndef IMAGELOADER_COLORSPACES_H
#define IMAGELOADER_COLORSPACES_H

#include <vector>
#include <cstdint>

namespace ColorSpaces{
    std::vector<uint8_t> RGBToYCbCr(const std::vector<uint8_t>& input);
    std::vector<uint8_t> YCbCrToRGB(const std::vector<uint8_t>& input);
}

#endif //IMAGELOADER_COLORSPACES_H
