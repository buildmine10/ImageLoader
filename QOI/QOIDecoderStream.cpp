//
// Created by colli on 5/31/2022.
//

#include "QOIDecoderStream.h"


#include <iostream>
#include <cmath>
#include <bitset>
#include <list>

#include <stdexcept>

#include "../stb_image_write.h"

void QOIDecoderStream::decodeByte(const uint8_t &byte) {
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

void QOIDecoderStream::readHeader(const uint8_t &byte) {
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


        if(header.channels < 3 || header.channels > 4) {
            throw std::runtime_error("Unable to load QOI Image. It has an invalid number of channels.");
        }

        buffer.clear();
        task = TAG;
    }
}

void QOIDecoderStream::readTag(const uint8_t &byte) {
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

void QOIDecoderStream::readRGB(const uint8_t &byte) {
    if(header.channels != 3){
        throw std::runtime_error("Unable to decode byte the QOI image does not have 3 channels.");
    }

    buffer.push_back(byte);
    if(buffer.size() == 3){
        data.push(buffer[0]);
        data.push(buffer[1]);
        data.push(buffer[2]);

        uint8_t hash = QOI::colorHash(buffer[0], buffer[1], buffer[2]);
        seenColors[hash] = {buffer[0], buffer[1], buffer[2], 255};
        lastColor = {buffer[0], buffer[1], buffer[2], 255};

        buffer.clear();
        task = TAG;
    }
}

void QOIDecoderStream::readRGBA(const uint8_t &byte) {
    if(header.channels != 4){
        throw std::runtime_error("Unable to decode byte the QOI image does not have 4 channels.");
    }

    buffer.push_back(byte);
    if(buffer.size() == 4){
        data.push(buffer[0]);
        data.push(buffer[1]);
        data.push(buffer[2]);
        data.push(buffer[3]);

        uint8_t hash = QOI::colorHash(buffer[0], buffer[1], buffer[2], buffer[3]);
        seenColors[hash] = {buffer[0], buffer[1], buffer[2], buffer[3]};
        lastColor = {buffer[0], buffer[1], buffer[2], buffer[3]};

        buffer.clear();
        task = TAG;
    }
}

void QOIDecoderStream::readIndex(const uint8_t &byte) {
    QOI::IndexChunk chunk(0);
    chunk.data = byte;

    std::array<uint8_t, 4> color = seenColors[chunk.getIndex()];
    lastColor = color;

    if(header.channels == 3){
        data.push(color[0]);
        data.push(color[1]);
        data.push(color[2]);
    }else{
        data.push(color[0]);
        data.push(color[1]);
        data.push(color[2]);
        data.push(color[3]);
    }

    buffer.clear();
    task = TAG;
}

void QOIDecoderStream::readDiff(const uint8_t &byte) {
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
        data.push(color[0]);
        data.push(color[1]);
        data.push(color[2]);
    }else{
        data.push(color[0]);
        data.push(color[1]);
        data.push(color[2]);
        data.push(color[3]);
    }

    buffer.clear();
    task = TAG;
}

void QOIDecoderStream::readLuma(const uint8_t &byte) {
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
            data.push(color[0]);
            data.push(color[1]);
            data.push(color[2]);
        }else{
            data.push(color[0]);
            data.push(color[1]);
            data.push(color[2]);
            data.push(color[3]);
        }

        buffer.clear();
        task = TAG;
    }
}

void QOIDecoderStream::readRun(const uint8_t &byte) {
    QOI::RunChunk chunk(0);
    chunk.data = byte;


    uint8_t hash = QOI::colorHash(lastColor);
    seenColors[hash] = lastColor;

    for(int i = 0; i < chunk.getLength(); i++){
        if(header.channels == 3){
            data.push(lastColor[0]);
            data.push(lastColor[1]);
            data.push(lastColor[2]);
        }else{
            data.push(lastColor[0]);
            data.push(lastColor[1]);
            data.push(lastColor[2]);
            data.push(lastColor[3]);
        }
    }

    buffer.clear();
    task = TAG;
}


QOIDecoderStream::QOIDecoderStream(const std::string &fileName) {
    file.open(fileName, std::ios::binary);

    while(data.empty()){
        uint8_t byte;
        file.read((char*)&byte, 1);
        decodeByte(byte);
    }
}

std::array<uint8_t, 4> QOIDecoderStream::getNextPixel() {
    static std::list<uint8_t> byteBuffer;
    static int endOfFileCounter = 0;
    bool isFileEnding = false;
    std::array<uint8_t, 4> color = {0, 0, 0, 255};

    while(byteBuffer.size() < 9 && !file.eof()){
        uint8_t byte;
        file.read((char*)&byte, 1);
        byteBuffer.push_back(byte);

        if(byte == 0){
            endOfFileCounter++;
        }else if(endOfFileCounter >= 7 && byte == 1){
            endOfFileCounter++;
            file.close();
            isFileEnding = true;
        }else{
            endOfFileCounter = 0;
        }
    }

    if(endOfFileCounter >= 7 && !file.eof()){
        uint8_t byte;
        file.read((char*)&byte, 1);
        byteBuffer.push_back(byte);

        if(byte == 0){
            endOfFileCounter++;
        }else if(endOfFileCounter >= 7 && byte == 1){
            endOfFileCounter++;
            file.close();
            isFileEnding = true;
        }else{
            endOfFileCounter = 0;
        }
    }
    if(isFileEnding){
        while(byteBuffer.size() > 8){
            decodeByte(byteBuffer.front());
            byteBuffer.pop_front();
        }
    }
    //std::cout << endOfFileCounter << std::endl;

    if(!data.empty()){
        color[0] = data.front(); data.pop();
        color[1] = data.front(); data.pop();
        color[2] = data.front(); data.pop();
        if(header.channels == 4){
            color[3] = data.front(); data.pop();
        }
        //std::cout << "bye" << std::endl;
    }else{
        //std::cout << std::bitset<8>(byteBuffer.front()) << std::endl;
        decodeByte(byteBuffer.front());
        byteBuffer.pop_front();

        return getNextPixel();
    }

    if((isFileEnding || file.eof()) && data.empty()){
        isEnd = true;
    }
    return color;
}

bool QOIDecoderStream::eof() {
    return isEnd;
}

int QOIDecoderStream::getWidth() {
    return header.width;
}

int QOIDecoderStream::getHeight() {
    return header.height;
}
