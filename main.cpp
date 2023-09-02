#include <iostream>
#include "QOI/QOIEncoder.h"
#include "QOI/QOIDecoder.h"
#include "QOI/QOIDecoderStream.h"
#include <bitset>
#include "ColorSpaces.h"

#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#undef STB_IMAGE_IMPLEMENTATION
#undef STB_IMAGE_WRITE_IMPLEMENTATION


long long mandelbrot(long double x, long double y);

std::array<uint8_t, 3> hueToRGB(float hue);

int main() {
    /*
    int width, height, nrChannels;

    uint8_t* data = stbi_load("IMG_0739.JPEG", &width, &height, &nrChannels, 0);


    std::cout << nrChannels << std::endl;
    QOIEncoder hi(width, height, nrChannels);

    for(int i = 0; i < width * height * nrChannels; i += nrChannels){
        if(nrChannels == 3){
            hi.encodePixel(data[i], data[i + 1], data[i + 2]);
        }else if(nrChannels == 4){
            hi.encodePixel(data[i], data[i + 1], data[i + 2], data[i + 3]);
        }

    }

    hi.endEncoding();

    hi.saveToFile("out");
    */

    QOIDecoderStream decoder("Mandelbrot.qoi");
    QOIEncoder encoder(decoder.getWidth(), decoder.getHeight(), 4);

    int i = 0;
    while(!decoder.eof()){
        i++;
        //if(i > 4000000 - 125)
            //std::cout << i << std::endl;
        encoder.encodePixel(decoder.getNextPixel());
    }
    encoder.encodePixel(0, 0, 0, 0);
    std::cout << i << std::endl;
    encoder.saveToFile("out.qoi");

    /*
    int width = 2000;
    int height = 2000;
    QOIEncoder hi(width, height, 3);

    long long counter = 0;
    for(int y = 0; y < height; y++){
        long double pY = (long double)y / height;
        for(int x = 0; x < width; x++){
            long double pX = (long double)x / width;
            long long value = mandelbrot((1 - pX) * -2 + pX * 1, (1 - pY) * -1.5 + pY * 1.5);
            auto color = hueToRGB(value);
            //std::cout << value << std::endl;
            hi.encodePixel(color[0], color[1], color[2]);

            if(counter % 1000000 == 0){
                std::cout << (double)counter / ((double)width * height) << std::endl;
            }

            counter++;
        }
    }
    hi.endEncoding();
    hi.saveToFile("output/out");

    */




    //QOIDecoderStream bye("output/out1.qoi");
    return 0;
}

std::array<uint8_t, 3> hueToRGB(float hue){
    float r, g, b;
    hue = std::fmod(hue, 360);
    float x = 1 - std::abs(std::fmod(hue / 60, 2) - 1);
    if(hue >= 0 && hue < 60){
        r = 1;
        g = x;
        b = 0;
    }else if(hue >= 60 && hue < 120){
        r = x;
        g = 1;
        b = 0;
    }else if(hue >= 120 && hue < 180){
        r = 0;
        g = 1;
        b = x;
    }else if(hue >= 180 && hue < 240){
        r = 0;
        g = x;
        b = 1;
    }else if(hue >= 240 && hue < 300){
        r = x;
        g = 0;
        b = 1;
    }else{
        r = 1;
        g = 0;
        b = x;
    }

    return {(uint8_t)(r * 255), (uint8_t)(g * 255), (uint8_t)(b * 255)};
}

long long mandelbrot(long double x, long double y){
    long double r = 0;
    long double i = 0;

    int maxIterations = 300;

    if(x <= 0 && std::sqrt((x + 0.1) * (x + 0.1) + y * y) < 0.6){
        return maxIterations;
    }
    if(x >= 0 && std::sqrt((x - 0.) * (x - 0.) + (y + 0.25) * (y + 0.25)) < 0.325){
        return maxIterations;
    }

    if(x >= 0 && std::sqrt((x - 0.) * (x - 0.) + (y - 0.25) * (y - 0.25)) < 0.325){
        return maxIterations;
    }

    if(std::sqrt((x + 1) * (x + 1) + y * y) < 0.25){
        return maxIterations;
    }
    if(std::sqrt((x + 1.309) * (x + 1.309) + y * y) < 0.059){
        return maxIterations;
    }
    if(std::sqrt((x + 1.309 + 0.059 + 0.0131) * (x + 1.309 + 0.059 + 0.0131) + y * y) < 0.013){
        return maxIterations;
    }


    for(int j = 0; j < maxIterations; j++){
        long double tr = r * r - i * i + x;
        long double ti = 2 * r * i + y;
        r = tr;
        i = ti;

        if(r * r + i * i > 4){
            return j;
        }
    }

    return maxIterations;
}