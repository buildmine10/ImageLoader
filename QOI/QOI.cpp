//
// Created by colli on 5/31/2022.
//

#include "QOI.h"

#include <iostream>
#include <cmath>
#include <bitset>
#include <fstream>
#include <stdexcept>

std::array<uint8_t, 14> QOI::QOIHeader::getData() const {
    std::array<uint8_t, 14> out;

    out[0] = 'q';
    out[1] = 'o';
    out[2] = 'i';
    out[3] = 'f';

    out[7] = ((uint8_t*)(&width))[0];
    out[6] = ((uint8_t*)(&width))[1];
    out[5] = ((uint8_t*)(&width))[2];
    out[4] = ((uint8_t*)(&width))[3];

    out[11] = ((uint8_t*)(&height))[0];
    out[10] = ((uint8_t*)(&height))[1];
    out[9] = ((uint8_t*)(&height))[2];
    out[8] = ((uint8_t*)(&height))[3];

    out[12] = channels;
    out[13] = colorspace;

    return out;
}


uint8_t QOI::IndexChunk::getIndex() {
    return data;
}

void QOI::IndexChunk::setIndex(uint8_t index) {
    if(index > 63){
        //std::cout << "Collin, you did a stupid. There are only 64 allowed indices." << std::endl;
    }

    data = 0b00111111 & index | 0b00000000;
}

QOI::IndexChunk::IndexChunk(uint8_t index) {
    setIndex(index);
}



bool QOI::DifferenceChunk::setDifference(int16_t dr, int16_t dg, int16_t db) {
    switch(dr){
        case -2:
            dr = 0b00000000;
            break;
        case -1:
            dr = 0b00010000;
            break;
        case 0:
            dr = 0b00100000;
            break;
        case 1:
            dr = 0b00110000;
            break;
        default:
            dr = 0b00100000;
            //std::cout << "The difference ranges from -2 to 1. dr out of range" << std::endl;
            return false;
            break;
    }

    switch(dg){
        case -2:
            dg = 0b00000000;
            break;
        case -1:
            dg = 0b00000100;
            break;
        case 0:
            dg = 0b00001000;
            break;
        case 1:
            dg = 0b00001100;
            break;
        default:
            dg = 0b00001000;
            //std::cout << "The difference ranges from -2 to 1. dg out of range" << std::endl;
            return false;
            break;
    }

    switch(db){
        case -2:
            db = 0b0000000;
            break;
        case -1:
            db = 0b0000001;
            break;
        case 0:
            db = 0b00000010;
            break;
        case 1:
            db = 0b0000011;
            break;
        default:
            db = 0b00000010;
            //std::cout << "The difference ranges from -2 to 1. db out of range" << std::endl;
            return false;
            break;
    }

    data = dr | dg | db | 0b01000000;
    return true;
}

std::array<int8_t, 3> QOI::DifferenceChunk::getDifference() {
    int8_t dr = ((0b00110000 & data) >> 4) - 2;
    int8_t dg = ((0b00001100 & data) >> 2) - 2;
    int8_t db = (0b00000011 & data) - 2;
    return {dr, dg, db};
}



std::array<int8_t, 3> QOI::LumaChunk::getDifference() {
    int8_t dg = (0b0000000000111111 & data) - 32;

    int8_t dr_dg = ((0b1111000000000000 & data) >> 12) - 8;

    int8_t db_dg = ((0b0000111100000000 & data) >> 8) - 8;

    return {(int8_t)(dr_dg + dg), dg, (int8_t)(db_dg + dg)};
}

bool QOI::LumaChunk::setDifference(int16_t dr, int16_t dg, int16_t db) {
    if(dg > 31 || dg < -32){
        //std::cout << "The dg difference ranges from -32 to 31. dg out of range" << std::endl;
        return false;
    }
    uint16_t greenDiff = (dg + 32) << 0;

    int8_t drdg = dr - dg;
    if(drdg < -8 || drdg > 7){
        //std::cout << "The dr-dg difference ranges from -8 to 7. dr-dg out of range" << std::endl;
        return false;
    }
    uint16_t dr_dg = ((drdg) + 8) << 12;

    int8_t dbdg = db - dg;
    if(dbdg < -8 || dbdg > 7){
        //std::cout << "The db-dg difference ranges from -8 to 7. db-dg out of range" << std::endl;
        return false;
    }
    uint16_t db_dg = ((dbdg) + 8) << 8;


    data =  0b0000000010000000 | greenDiff | dr_dg | db_dg;
    //std::bitset<16> a(data);
    //std::cout << a << std::endl;
    return true;
}



uint8_t QOI::RunChunk::getLength() {
    return (0b00111111 & data) + 1;
}

bool QOI::RunChunk::setLength(uint8_t length) {
    if(length > 62 || length < 1){
        return false;
    }
    data = 0b11000000 | length - 1;
    //std::cout << std::bitset<8>(data) << std::endl;
    return true;
}

QOI::RunChunk::RunChunk(uint8_t length) {
    setLength(length);
}


QOI::ColorRGBChunk::ColorRGBChunk(uint8_t *color) {
    r = color[0];
    g = color[1];
    b = color[2];
}

QOI::ColorRGBAChunk::ColorRGBAChunk(uint8_t *color) {
    r = color[0];
    g = color[1];
    b = color[2];
    a = color[3];
}



uint8_t QOI::colorHash(std::array<uint8_t, 3> color) {
    return (color[0] * 3 + color[1] * 5 + color[2] * 7) % 64;
}

uint8_t QOI::colorHash(std::array<uint8_t, 4> color) {
    return (color[0] * 3 + color[1] * 5 + color[2] * 7 + color[3] * 11) % 64;
}

uint8_t QOI::colorHash(const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& a){
    return (r * 3 + g * 5 + b * 7 + a * 11) % 64;
}




