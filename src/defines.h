#ifndef DEFINES_H_INCLUDED
#define DEFINES_H_INCLUDED

#include <iostream>
#include <sstream>
#include <math.h>

using namespace std;

#include <irrlicht.h>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


//! Системная группа
#define DEBUG_INFO                      true
#define DEVICE_MOUSE_PRESS_DELAY        15

//! Параметры
#define PARAM_MBORIGINAL_COUNT          4
#define PARAM_CHUNK_SIZE                64
#define PARAM_CHUNK_HEIGHT              64
#define PARAM_BLOCK_SIZE                1.0

//! Номера ошибок
enum
{
    ERR_DEVICE    =         1,
    ERR_DEVICE_DROP
};

//! Стороны света
enum
{
    diN   =   1,
    diS,
    diW,
    diE,
    diU,
    diD
};
#endif // DEFINES_H_INCLUDED
