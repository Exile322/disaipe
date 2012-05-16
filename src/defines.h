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
#define PARAM_MULTITEX_COUNT            16

//! Параметры
#define PARAM_MBORIGINAL_COUNT          4
#define PARAM_WORLD_SIZE_X              2      //Количество чанков в мире по оси Х
#define PARAM_WORLD_SIZE_Z              2      //Количество чанков в мире по оси Z
#define PARAM_WORLD_SIZE_Y              1      //количество чанков в мире по вертикали (оси Y)
#define PARAM_CHUNK_SIZE                48      //Количество кубов, помещающихся в один чанк по x и z
#define PARAM_CHUNK_HEIGHT              48      //Количество кубов, помещающихся в один чанк по вертикали
#define PARAM_BLOCK_SIZE                1.0     //Размерность блока

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
