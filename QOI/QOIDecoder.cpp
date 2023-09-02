//
// Created by colli on 5/31/2022.
//

#include "QOIDecoder.h"

#include <iostream>
#include <cmath>
#include <bitset>
#include <fstream>
#include <stdexcept>
#include <queue>

#include "../stb_image_write.h"

void QOIDecoder::decodeByte(const uint8_t &byte) {
    switch (task) {
        case HEADER:
            readHeader(byte);
            break;
        case TAG:
            readTag(byte);
            break;
        case RGB:
            readRGB(byte);
            break;
        case RGBA:
            readRGBA(byte);
            break;
        case INDEX:
            readIndex(byte);
            break;
        case DIFF:
            readDiff(byte);
            break;
        case LUMA:
            readLuma(byte);
            break;
        case RUN:
            readRun(byte);
            break;
    }
}

void QOIDecoder::readHeader(const uint8_t &byte) {
    buffer.push_back(byte);
    if(buffer.size() == 14){
        uint8_t temp[4];
        temp[3] = buffer[4];
        temp[2] = buffer[5];
        temp[1] = buffer[6];
        temp[0] = buffer[7];
        header.width = *(uint32_t*)temp;

        temp[3] = buffer[8];
        temp[2] = buffer[9];
        temp[1] = buffer[10];
        temp[0] = buffer[11];
        header.height = *(uint32_t*)temp;

        header.channels = buffer[12];
        header.colorspace = buffer[13];


        if(header.channels < 3 || header.channels > 4){
            throw std::runtime_error("Unable to load QOI Image. It has an invalid number of channels.");
        }

        data.reserve(header.height * header.width * header.channels);

        buffer.clear();
        task = TAG;
    }
}

void QOIDecoder::readTag(const uint8_t &byte) {
    //std::cout << std::bitset<8>(byte) << std::endl;
    if(byte == 0b11111110){
        //std::cout << "RGB" << std::endl;
        task = RGB;
    }else if(byte == 0b11111111){
        //std::cout << "RGBA" << std::endl;
        task = RGBA;
    }else if((byte & 0b11000000) == 0b00000000){
        //std::cout << "INDEX" << std::endl;
        task = INDEX;
        decodeByte(byte);//run again because the same byte contains more data
    }else if((byte & 0b11000000) == 0b01000000){
        //std::cout << "DIFF" << std::endl;
        task = DIFF;
        decodeByte(byte);//run again because the same byte contains more data
    }else if((byte & 0b11000000) == 0b10000000){
        //std::cout << "LUMA" << std::endl;
        task = LUMA;
        decodeByte(byte);//run again because the same byte contains more data
    }else if((byte & 0b11000000) == 0b11000000){
        //std::cout << "RUN" << std::endl;
        task = RUN;
        decodeByte(byte);//run again because the same byte contains more data
    }
}

void QOIDecoder::readRGB(const uint8_t &byte) {
    if(header.channels != 3){
        throw std::runtime_error("Unable to decode byte the QOI image does not have 3 channels.");
    }

    buffer.push_back(byte);
    if(buffer.size() == 3){
        data.insert(data.end(), buffer.begin(), buffer.end());

        uint8_t hash = QOI::colorHash(buffer[0], buffer[1], buffer[2]);
        seenColors[hash] = {buffer[0], buffer[1], buffer[2], 255};
        lastColor = {buffer[0], buffer[1], buffer[2], 255};

        buffer.clear();
        task = TAG;
    }
}

void QOIDecoder::readRGBA(const uint8_t &byte) {
    if(header.channels != 4){
        throw std::runtime_error("Unable to decode byte the QOI image does not have 4 channels.");
    }

    buffer.push_back(byte);
    if(buffer.size() == 4){
        data.insert(data.end(), buffer.begin(), buffer.end());

        uint8_t hash = QOI::colorHash(buffer[0], buffer[1], buffer[2], buffer[3]);
        seenColors[hash] = {buffer[0], buffer[1], buffer[2], buffer[3]};
        lastColor = {buffer[0], buffer[1], buffer[2], buffer[3]};

        buffer.clear();
        task = TAG;
    }
}

void QOIDecoder::readIndex(const uint8_t &byte) {
    QOI::IndexChunk chunk(0);
    chunk.data = byte;

    std::array<uint8_t, 4> color = seenColors[chunk.getIndex()];
    lastColor = color;

    if(header.channels == 3){
        data.insert(data.end(), color.begin(), color.end() - 1);
    }else{
        data.insert(data.end(), color.begin(), color.end());
    }

    buffer.clear();
    task = TAG;
}

void QOIDecoder::readDiff(const uint8_t &byte) {
    QOI::DifferenceChunk chunk;
    chunk.data = byte;
    std::array<int8_t, 3> difference = chunk.getDifference();

    std::array<uint8_t, 4> color = {
            (uint8_t)(lastColor[0] + difference[0]),
            (uint8_t)(lastColor[1] + difference[1]),
            (uint8_t)(lastColor[2] + difference[2]),
            lastColor[3]
    };

    uint8_t hash = QOI::colorHash(color);
    seenColors[hash] = color;
    lastColor = color;

    if(header.channels == 3){
        data.insert(data.end(), color.begin(), color.end() - 1);
    }else{
        data.insert(data.end(), color.begin(), color.end());
    }

    buffer.clear();
    task = TAG;
}

void QOIDecoder::readLuma(const uint8_t &byte) {
    buffer.push_back(byte);
    //std::cout << "hi" << std::endl;
    if(buffer.size() == 2){

        QOI::LumaChunk chunk;
        chunk.data = ((uint32_t)buffer[1] << 8) + buffer[0];

        std::array<int8_t, 3> difference = chunk.getDifference();

        std::array<uint8_t, 4> color = {
                (uint8_t)(lastColor[0] + difference[0]),
                (uint8_t)(lastColor[1] + difference[1]),
                (uint8_t)(lastColor[2] + difference[2]),
                lastColor[3]
        };

        uint8_t hash = QOI::colorHash(color);
        seenColors[hash] = color;
        lastColor = color;

        if(header.channels == 3){
            data.insert(data.end(), color.begin(), color.end() - 1);
        }else{
            data.insert(data.end(), color.begin(), color.end());
        }

        buffer.clear();
        task = TAG;
    }
}

void QOIDecoder::readRun(const uint8_t &byte) {
    QOI::RunChunk chunk(0);
    chunk.data = byte;


    uint8_t hash = QOI::colorHash(lastColor);
    seenColors[hash] = lastColor;

    for(int i = 0; i < chunk.getLength(); i++){
        if(header.channels == 3){
            data.insert(data.end(), lastColor.begin(), lastColor.end() - 1);
        }else{
            data.insert(data.end(), lastColor.begin(), lastColor.end());
        }
    }

    buffer.clear();
    task = TAG;
}

void QOIDecoder::saveAsPNG(const std::string &fileName) {
    stbi_write_png(fileName.c_str(), header.width, header.height, header.channels, data.data(), 0);
}

void QOIDecoder::saveAsBMP(const std::string &fileName) {
    stbi_write_bmp(fileName.c_str(), header.width, header.height, header.channels, data.data());
}


QOIDecoder::QOIDecoder(const std::string &fileName) {
    std::ifstream file(fileName, std::ios::binary);

    while(data.empty()){
        uint8_t byte;
        file.read((char*)&byte, 1);
        decodeByte(byte);
    }
    bool eof = false;
    int endOfFileCounter = 0;



    while(!eof){

        std::queue<uint8_t> byteQueue;
        while(true){
            uint8_t byte;
            file.read((char*)&byte, 1);
            byteQueue.push(byte);
            //std::cout << (int)byte << std::endl;
            if(byte == 0){
                endOfFileCounter++;
                //std::cout << endOfFileCounter << std::endl;
            }else if(endOfFileCounter >= 7 && byte == 1){
                endOfFileCounter++;
                eof = true;
            }else{
                endOfFileCounter = 0;
                break;
            }
        };

        if(!eof){
            while(!byteQueue.empty()){
                decodeByte(byteQueue.front());
                byteQueue.pop();
            }
        }else{
            while(byteQueue.size() > 8){//the last 8 bytes are the ending
                decodeByte(byteQueue.front());
                byteQueue.pop();
            }
        }



        /*
        uint8_t byte;
        file.read((char*)&byte, 1);
        decodeByte(byte);
         */
    }

    file.close();
}