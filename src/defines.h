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
#define DEBUG_INFO                      false
#define DEVICE_MOUSE_PRESS_DELAY        15
#define PARAM_MULTITEX_COUNT            16

//! Параметры генератора ландшафта
#define PERLIN_NOISE_OCTAVES            8
#define PERLIN_MAP_SIZE                 PARAM_WORLD_SIZE_X*PARAM_CHUNK_SIZE//256     //Размер карты высот

//! Параметры
/*
    Что мы имеем? Мы имеем мир (WORLD), разделенный на секции (SECTIONS),
    которые в свою очередь делятся на куски (CHUNKS), сотсоящие из блоков (BLOCKS).

    Размерность WORLD вычисляется по щирине, глубине и высоте (X,Z,Y)

    Размерность SECTIONS вычисляется только по ширине и глубине (X,Z). В данный момент секция имеет высоту мира.
    Каждая секция это отдельный Меш, отдельный Нод.

    Размерность CHUNKS вычисляется по ширине, глубине и высоте (X,Z,Y).
*/
#define PARAM_MBORIGINAL_COUNT          4
#define PARAM_WORLD_SIZE_X              8      //Количество чанков в мире по оси Х
#define PARAM_WORLD_SIZE_Z              8      //Количество чанков в мире по оси Z
#define PARAM_WORLD_SIZE_Y              1      //количество чанков в мире по вертикали (оси Y)
#define PARAM_CHUNK_SIZE                32      //Количество кубов, помещающихся в один чанк по x и z
#define PARAM_CHUNK_HEIGHT              32      //Количество кубов, помещающихся в один чанк по вертикали
#define PARAM_BLOCK_SIZE                1.0     //Размерность блока
#define PARAM_SECTIONS                  4       //Размерность секций.

//! Настройки клавиш
#define KEYS_WIREWRAME                  KEY_F1
#define KEYS_BOUNDING_BOX               KEY_F2
#define KEYS_HALF_TRANSPARENT           KEY_F3
#define KEYS_PLAYER_WALK_FWD            KEY_KEY_W
#define KEYS_PLAYER_WALK_BCK            KEY_KEY_S
#define KEYS_PLAYER_STRAFE_L            KEY_KEY_A
#define KEYS_PLAYER_STRAFE_R            KEY_KEY_D
#define KEYS_PLAYER_JUMP                KEY_SPACE

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
