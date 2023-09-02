//
// Created by colli on 5/31/2022.
//

#include "ColorSpaces.h"

#include <algorithm>
#include <iostream>

std::vector<uint8_t> ColorSpaces::RGBToYCbCr(const std::vector<uint8_t>& input){
    std::vector<uint8_t> output;
    output.reserve(input.size());

    for(int i = 0; i < input.size(); i += 3){
        float Y = 16 + 65.738 * input[i] / 256 + 129.057 * input[i + 1] / 256 + 25.064 * input[i + 2] / 256;
        float Cb = 128 - 37.945 * input[i] / 256 - 74.494 * input[i + 1] / 256 + 112.439 * input[i + 2] / 256;
        float Cr = 128 + 112.439 * input[i] / 256 - 94.154 * input[i + 1] / 256 - 18.285 * input[i + 2] / 256;
        //std::cout << (int)std::max(std::min(Y, 255.f), 0.f) << " ";
        //std::cout << (int)std::max(std::min(Y, 255.f), 0.f) << " ";
        //std::cout << (int)std::max(std::min(Cr, 255.f), 0.f) << std::endl;
        output.push_back((uint8_t)std::max(std::min(Y, 255.f), 0.f));
        output.push_back((uint8_t)std::max(std::min(Cb, 255.f), 0.f));
        output.push_back((uint8_t)std::max(std::min(Cr, 255.f), 0.f));
    }

    return output;
}

std::vector<uint8_t> ColorSpaces::YCbCrToRGB(const std::vector<uint8_t>& input){
    std::vector<uint8_t> output;
    output.reserve(input.size());

    for(int i = 0; i < input.size(); i += 3){
        float Cb = input[i + 1];// - 128;
        float Cr = input[i + 2];// - 128;
        float R = 298.082 * input[i] / 256 + 408.583 * Cr / 256 - 222.921;
        float G = 298.082 * input[i] / 256 - 100.291 * Cb / 256 - 208.120 * Cr / 256 + 135.576;
        float B = 298.082 * input[i] / 256 + 516.412 * Cb / 256 - 276.836;

        output.push_back((uint8_t)std::max(std::min(R, 255.f), 0.f));
        output.push_back((uint8_t)std::max(std::min(G, 255.f), 0.f));
        output.push_back((uint8_t)std::max(std::min(B, 255.f), 0.f));
    }

    return output;
}