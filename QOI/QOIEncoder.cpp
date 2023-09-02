//
// Created by colli on 5/31/2022.
//

#include "QOIEncoder.h"

#include <iostream>
#include <cmath>
#include <bitset>
#include <fstream>
#include <stdexcept>

void QOIEncoder::encodePixel(const uint8_t &r, const uint8_t &g, const uint8_t &b) {
    encodePixel(std::array<uint8_t, 3>{r, g, b});
}

void QOIEncoder::encodePixel(const uint8_t &r, const uint8_t &g, const uint8_t &b, const uint8_t &a) {
    encodePixel(std::array<uint8_t, 4>{r, g, b, a});
}

void QOIEncoder::encodePixel(const std::array<uint8_t, 3>& _color) {
    if(isEncodingOver){
        return;
    }
    posX++;
    if(posX >= header.width){
        posY++;
        posX = 0;
    }

    if(header.channels != 3){
        throw std::runtime_error("Unable to add pixel. It must have 3 color channels.");
    }


    std::array<uint8_t, 4> color = {_color[0], _color[1], _color[2], 255};
    uint8_t hash = QOI::colorHash(color);

    if(lastColor == color){
        runLength++;
        if(runLength == 62){
            QOI::RunChunk chunk(runLength);
            data.insert(data.end(), (uint8_t*)&chunk, (uint8_t*)&chunk + sizeof(chunk));
            runLength = 0;
        }
        goto end;
    }else if(runLength != 0){
        QOI::RunChunk chunk(runLength);
        data.insert(data.end(), (uint8_t*)&chunk, (uint8_t*)&chunk + sizeof(chunk));
        runLength = 0;
    }


    //if not a run then compare against the seenColors
    if(seenColors[hash][0] == color[0] && seenColors[hash][1] == color[1] && seenColors[hash][2] == color[2]){
        //std::cout << (int)imageData[y][x][0] << ", " << (int)imageData[y][x][1] << ", " << (int)imageData[y][x][2] << std::endl;
        QOI::IndexChunk chunk(hash);
        data.insert(data.end(), (uint8_t*)&chunk, (uint8_t*)&chunk + sizeof(chunk));
        goto end;
    }


    {
        std::array<int16_t, 3> difference = {
                (int16_t)(color[0] - lastColor[0]),
                (int16_t)(color[1] - lastColor[1]),
                (int16_t)(color[2] - lastColor[2])
        };

        //std::cout << (int)difference[0] << ", " << (int)difference[1] << ", " << (int)difference[2] << std::endl;
        {
            QOI::DifferenceChunk chunk;
            if(chunk.setDifference(difference[0], difference[1], difference[2])){
                //std::cout << (int)imageData[y][x][0] << ", " << (int)imageData[y][x][1] << ", " << (int)imageData[y][x][2] << std::endl;
                data.insert(data.end(), (uint8_t*)&chunk, (uint8_t*)&chunk + sizeof(chunk));
                goto end;
            }
        }

        {
            QOI::LumaChunk chunk;
            if(chunk.setDifference(difference[0], difference[1], difference[2])){
                data.insert(data.end(), (uint8_t*)&chunk, (uint8_t*)&chunk + sizeof(chunk));
                goto end;
            }
        }
    }

    //if none of the other encoding methods worked then just list the color
    {
        //std::cout << "hi" << std::endl;
        QOI::ColorRGBChunk chunk(color.data());
        data.insert(data.end(), (uint8_t*)&chunk, (uint8_t*)&chunk + sizeof(chunk));
        goto end;
    }



    end:
    //std::cout << (int)imageData[y][x][0] << ", " << (int)imageData[y][x][1] << ", " << (int)imageData[y][x][2] << std::endl;
    lastColor = color;
    seenColors[hash] = color;

    if(posY >= header.height){
        endEncoding();
        return;
    }
}

void QOIEncoder::encodePixel(const std::array<uint8_t, 4>& color) {
    if(isEncodingOver){
        return;
    }
    posX++;
    if(posX >= header.width){
        posY++;
        posX = 0;
    }
    if(header.channels != 4){
        throw std::runtime_error("Unable to add pixel. It must have 4 color channels.");
    }
    uint8_t hash = QOI::colorHash(color);


    if(lastColor == color){
        runLength++;
        if(runLength == 62){
            QOI::RunChunk chunk(runLength);
            data.insert(data.end(), (uint8_t*)&chunk, (uint8_t*)&chunk + sizeof(chunk));
            runLength = 0;
        }
        goto end;
    }else if(runLength != 0){
        QOI::RunChunk chunk(runLength);
        data.insert(data.end(), (uint8_t*)&chunk, (uint8_t*)&chunk + sizeof(chunk));
        runLength = 0;
    }


    //if not a run then compare against the seenColors
    if(seenColors[hash] == color){
        //std::cout << (int)imageData[y][x][0] << ", " << (int)imageData[y][x][1] << ", " << (int)imageData[y][x][2] << std::endl;
        QOI::IndexChunk chunk(hash);
        data.insert(data.end(), (uint8_t*)&chunk, (uint8_t*)&chunk + sizeof(chunk));
        goto end;
    }

    //difference chunks
    if(lastColor[3] == color[3]){
        std::array<int16_t, 3> difference = {
                (int16_t)(color[0] - lastColor[0]),
                (int16_t)(color[1] - lastColor[1]),
                (int16_t)(color[2] - lastColor[2])
        };

        //std::cout << (int)difference[0] << ", " << (int)difference[1] << ", " << (int)difference[2] << std::endl;
        {
            QOI::DifferenceChunk chunk;
            if(chunk.setDifference(difference[0], difference[1], difference[2])){
                //std::cout << (int)imageData[y][x][0] << ", " << (int)imageData[y][x][1] << ", " << (int)imageData[y][x][2] << std::endl;
                data.insert(data.end(), (uint8_t*)&chunk, (uint8_t*)&chunk + sizeof(chunk));
                goto end;
            }
        }

        {
            QOI::LumaChunk chunk;
            if(chunk.setDifference(difference[0], difference[1], difference[2])){
                data.insert(data.end(), (uint8_t*)&chunk, (uint8_t*)&chunk + sizeof(chunk));
                goto end;
            }
        }
    }

    //if none of the other encoding methods worked then just list the color
    {
        //std::cout << "hi" << std::endl;
        std::array<uint8_t, 4> temp = color;
        QOI::ColorRGBAChunk chunk(temp.data());
        data.insert(data.end(), (uint8_t*)&chunk, (uint8_t*)&chunk + sizeof(chunk));
        goto end;
    }



    end:
    //std::cout << (int)imageData[y][x][0] << ", " << (int)imageData[y][x][1] << ", " << (int)imageData[y][x][2] << std::endl;
    lastColor = color;
    seenColors[hash] = color;
    if(posY >= header.height){
        std::cout << "hi" << std::endl;
        endEncoding();
        return;
    }
}

void QOIEncoder::endEncoding() {
    if(!isEncodingOver){
        isEncodingOver = true;
        if(runLength != 0){
            QOI::RunChunk chunk(runLength);
            data.insert(data.end(), (uint8_t*)&chunk, (uint8_t*)&chunk + sizeof(chunk));
            runLength = 0;
        }
    }

}

void QOIEncoder::saveToFile(const std::string &fileName) {
    uint8_t ending[8] = {0, 0, 0, 0, 0, 0, 0, 1};

    std::ofstream file(fileName, std::ios::binary );

    if(!isEncodingOver){
        endEncoding();
    }

    file.write((char*)header.getData().data(), 14);
    file.write((char*)data.data(), data.size());
    file.write((char*)ending, 8);

    file.close();
}

QOIEncoder::QOIEncoder(const int &width, const int &height, const int &nrChannels) {
    header.width = width;
    header.height = height;
    header.channels = nrChannels;
    header.colorspace = 1;

    if(nrChannels < 3 || nrChannels > 4){
        throw std::runtime_error("QOI images must have either 3 of 4 color channels");
    }
}
