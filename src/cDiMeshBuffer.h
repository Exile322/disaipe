/*
    Низкоуровневое управление мешбуферами
    По статье: http://irrlicht.ru/index.php?vm=15.view.12.

        1. Немного теории
            Загружая меш, мы получаем его мешбуфер, в котором хранятся данные о положении всех вершин
            и их параметров. Один мешбуфер должен иметь не более 65536 вершин, т.к. это ограничено
            аппаратно. Одна модель может содержать гораздо меньшее количество вершин, соответственно
            мешбуфер будет передавать частично пустым, что скажется на эффективности работы
            приложения. Поэтому решено объединять несколько объектов (мешей) в один мешбуфер размером
            65536 вершин.

        2. Практика
            Имеем:
                mb_original - оригинал меша (его мешбуфер), который будем загружать из модели
                mb_storage - хранилище объектов (мешей)

                Получая mb_original. высчитываем:
                    u32 v_quant - кол-во вершин в оригинальном буфере
                    u32 i_quant - кол-во индексов в оригинальном буфере
                    u32 max_doubles - максимальное количество дублей одного объекта в хранилище



*/
#ifndef CDIMESHBUFFER_H
#define CDIMESHBUFFER_H

#include <irrlicht.h>
#include <boost/array.hpp>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#include "cDevice.h"
#include "cFlags.h"
#include "cUtilities.h"
#include "cGlobalMatrix.h"

namespace Di
{
    class cDiMeshBuffer
    {
        public:
            cDiMeshBuffer(vector3di id, Di::cDevice* d, Di::cGlobalMatrix* glM,Di::cFlags* f)
            {
                dev = d;
                flags = f;
                glMatrix = glM;
                Id = id;

                mb_storage = new SMeshBuffer;

                isUsed = false;

                //настройка материала
                mb_material.setTexture(0,dev->driver->getTexture("textures/terrain.png"));
                mb_material.setFlag(EMF_LIGHTING, false);

                mb_material.TextureLayer[0].TextureWrapU = ETC_REPEAT;
                mb_material.TextureLayer[0].TextureWrapV = ETC_REPEAT;

                mb_storage->Material = mb_material;

                textureId = vector2di(3,10); //столбец,строка
            }

            virtual ~cDiMeshBuffer() {}

            SMeshBuffer* mb_original[PARAM_MBORIGINAL_COUNT];   //мешбуфер - оригинал меша
            SMeshBuffer* mb_storage;    //мешбуфер - хранилище дубликатов
            SMaterial mb_material;     //материал мешбуфера

            u32 v_quant; //кол-во вершин в оригинальном буфере
            u32 i_quant; //кол-во индексов в оригинальном буфере

            vector3di Id; //позиция чанка в мире
            vector2di textureId;

            bool isUsed;

            array<int> v_shifts; //индекс первой вершины объекта в хранилище mb_storage.vertices
            array<int> i_shifts; //индекс первой поверхности объекта в хранилище mb_storage.indicies

            boost::array<boost::array<boost::array<u32,PARAM_CHUNK_HEIGHT>,PARAM_CHUNK_SIZE>,PARAM_CHUNK_SIZE> matrix; //матрица чанка
            boost::array<boost::array<boost::array<int,PARAM_CHUNK_HEIGHT>,PARAM_CHUNK_SIZE>,PARAM_CHUNK_SIZE> NShifts; //массивы адресов вершин плоскостей
            boost::array<boost::array<boost::array<int,PARAM_CHUNK_HEIGHT>,PARAM_CHUNK_SIZE>,PARAM_CHUNK_SIZE> SShifts;
            boost::array<boost::array<boost::array<int,PARAM_CHUNK_HEIGHT>,PARAM_CHUNK_SIZE>,PARAM_CHUNK_SIZE> WShifts;
            boost::array<boost::array<boost::array<int,PARAM_CHUNK_HEIGHT>,PARAM_CHUNK_SIZE>,PARAM_CHUNK_SIZE> EShifts;
            boost::array<boost::array<boost::array<int,PARAM_CHUNK_HEIGHT>,PARAM_CHUNK_SIZE>,PARAM_CHUNK_SIZE> UShifts;
            boost::array<boost::array<boost::array<int,PARAM_CHUNK_HEIGHT>,PARAM_CHUNK_SIZE>,PARAM_CHUNK_SIZE> DShifts;

            void registerMatrix()
            {
                glMatrix->registerChunkMatrix(Id, &matrix);
                isUsed = true;
            }

            //! Генерация матрицы сферы
            void generateSphere(u32 R, u32 M,vector3di displace=vector3di(0,0,0))
            {
                //Уравнение сферы
                //(x-a)^2 + (z-b)^2 + (y-c)^2 = R^2

                for (int x=0; x < PARAM_CHUNK_SIZE; x++)
                {
                    for (int z=0; z < PARAM_CHUNK_SIZE; z++)
                    {
                        for (int y=0; y < PARAM_CHUNK_HEIGHT; y++)
                        {
                            int xs = sqrt(pow(R,2) - pow(z-displace.Z,2) - pow(y-displace.Y,2)) + displace.X;
                            int zs = sqrt(pow(R,2) - pow(x-displace.X,2) - pow(y-displace.Y,2)) + displace.Z;
                            int ys = sqrt(pow(R,2) - pow(z-displace.Z,2) - pow(x-displace.X,2)) + displace.Y;
                            if (x <= xs && z <= zs && y <= ys)
                                matrix[x][z][y] = M;

                            //заодно обнулим смещения вершин
                            NShifts[x][z][y] = -1;
                            SShifts[x][z][y] = -1;
                            WShifts[x][z][y] = -1;
                            EShifts[x][z][y] = -1;
                            UShifts[x][z][y] = -1;
                            DShifts[x][z][y] = -1;
                        }
                    }
                }

                //зарегистрируем нашу матрицу
                registerMatrix();
            }

            //! Генерация матрицы эллипсоида
            void generateEllipse(u32 xr, u32 zr, u32 yr, vector3di displace=vector3di(0,0,0))
            {
                //Уравнение эллипсоида
                //(x-a)^2/xr^2 + (z-b)^2/zr^2 + (y-c)^2/yr^2 = 1

                for (int x=0; x < PARAM_CHUNK_SIZE; x++)
                {
                    for (int z=0; z < PARAM_CHUNK_SIZE; z++)
                    {
                        for (int y=0; y < PARAM_CHUNK_HEIGHT; y++)
                        {
                            int xs = sqrt(1 - pow(z-displace.Z,2)/pow(zr,2) - pow(y-displace.Y,2)/pow(yr,2)) * pow(xr,2) + displace.X;
                            int zs = sqrt(1 - pow(x-displace.X,2)/pow(xr,2) - pow(y-displace.Y,2)/pow(yr,2)) * pow(zr,2) + displace.Z;
                            int ys = sqrt(1 - pow(z-displace.Z,2)/pow(zr,2) - pow(x-displace.X,2)/pow(xr,2)) * pow(yr,2) + displace.Y;
                            if (x <= xs && z <= zs && y <= ys)
                                matrix[x][z][y] = 1;

                            //заодно обнулим смещения вершин
                            NShifts[x][z][y] = -1;
                            SShifts[x][z][y] = -1;
                            WShifts[x][z][y] = -1;
                            EShifts[x][z][y] = -1;
                            UShifts[x][z][y] = -1;
                            DShifts[x][z][y] = -1;
                        }
                    }
                }

                //зарегистрируем нашу матрицу
                registerMatrix();
            }

            //! Генерация матрицы случайной плоскости
            void generateRandomSurface()
            {
                //генерация тестовой матрицы
                for (u32 x=0;x < PARAM_CHUNK_SIZE; x++)
                    for (u32 z=0;z < PARAM_CHUNK_SIZE; z++)
                        for (u32 y=0;y < PARAM_CHUNK_HEIGHT; y++)
                            {
                                matrix[x][z][y]=rand()%2;

                                //заодно обнулим смещения вершин
                                NShifts[x][z][y] = -1;
                                SShifts[x][z][y] = -1;
                                WShifts[x][z][y] = -1;
                                EShifts[x][z][y] = -1;
                                UShifts[x][z][y] = -1;
                                DShifts[x][z][y] = -1;
                            }

                //зарегистрируем нашу матрицу
                registerMatrix();
            }

            void generateFromHeightMap(IImage* hMap, float verticalRatio = 1.0f,vector2di displace=vector2di(0,0))
            {
                for (u32 x=0;x < PARAM_CHUNK_SIZE; x++)
                    for (u32 z=0;z < PARAM_CHUNK_SIZE; z++)
                    {
                        SColor p = hMap->getPixel(x + displace.X,z + displace.Y);

                        u32 cY = rint((p.getRed()+p.getGreen()+p.getBlue())/(verticalRatio*PARAM_CHUNK_HEIGHT));

                        if (cY > PARAM_CHUNK_HEIGHT) return;

                        matrix[x][z][cY] = 1;

                        for (u32 y=0;y < PARAM_CHUNK_HEIGHT; y++)
                        {
                            if (y < cY) matrix[x][z][y] = 1;
                            //заодно обнулим смещения вершин
                            NShifts[x][z][y] = -1;
                            SShifts[x][z][y] = -1;
                            WShifts[x][z][y] = -1;
                            EShifts[x][z][y] = -1;
                            UShifts[x][z][y] = -1;
                            DShifts[x][z][y] = -1;
                        }
                    }
            }

            //! Построение кубов по заданной матрице
            void build()
            {
                for (u32 x=0; x<PARAM_CHUNK_SIZE; x++)
                {
                    for (u32 y=0; y<PARAM_CHUNK_SIZE; y++)
                    {
                        for (u32 z=0; z<PARAM_CHUNK_HEIGHT; z++)
                        {
                            int N, S, W, E, U, D;
                            switch (x)
                            {
                                case 0:
                                    if (Id.X > 0)
                                        S = glMatrix->getMValueFromCoord(vector3di(Id.X-1,Id.Y,Id.Z),vector3di(PARAM_CHUNK_SIZE-1,z,y));
                                    else
                                        S = 0;
                                    break;
                                case PARAM_CHUNK_SIZE-1:
                                    if (Id.X < (PARAM_WORLD_SIZE_X)-1)
                                        N = glMatrix->getMValueFromCoord(vector3di(Id.X+1,Id.Y,Id.Z),vector3di(0,z,y));
                                    else
                                        N = 0;
                                    break;
                                default:
                                    break;
                            }
                            switch (y)
                            {
                                case 0:
                                    if (Id.Z > 0)
                                        W = glMatrix->getMValueFromCoord(vector3di(Id.X,Id.Y,Id.Z-1),vector3di(x,z,PARAM_CHUNK_SIZE-1));
                                    else
                                        W = 0;
                                    break;
                                case PARAM_CHUNK_SIZE-1:
                                    if (Id.Z < (PARAM_WORLD_SIZE_Z-1))
                                        E = glMatrix->getMValueFromCoord(vector3di(Id.X,Id.Y,Id.Z+1),vector3di(x,z,0));
                                    else
                                        E = 0;
                                    break;
                                default:
                                    break;
                            }
                            switch (z)
                            {
                                case 0:
                                    if (Id.Y > 0)
                                        D = glMatrix->getMValueFromCoord(vector3di(Id.X,Id.Y-1,Id.Z),vector3di(x,PARAM_CHUNK_HEIGHT-1,y));
                                    else
                                        D = 0;
                                    break;
                                case PARAM_CHUNK_HEIGHT-1:
                                    if (Id.Y < (PARAM_WORLD_SIZE_Y-1))
                                        U = glMatrix->getMValueFromCoord(vector3di(Id.X,Id.Y+1,Id.Z),vector3di(x,0,y));
                                    else
                                        U = 0;
                                    break;
                                default:
                                    break;
                            }
                            if (x!=PARAM_CHUNK_SIZE-1) N = matrix[x+1][y][z];
                            if (x!=0) S = matrix[x-1][y][z];
                            if (y!=PARAM_CHUNK_SIZE-1) E = matrix[x][y+1][z];
                            if (y!=0) W = matrix[x][y-1][z];
                            if (z!=PARAM_CHUNK_HEIGHT-1) U = matrix[x][y][z+1];
                            if (z!=0) D = matrix[x][y][z-1];
                            if (matrix[x][y][z]==1) dublicateTo(createPlaneMesh(vector3di(x,z,y),N,S,W,E,U,D),x,z,y);
                        }
                    }
                }
            }

            //!Создание нового мешбуфера, получение оригинального меша, настройка материала
            void addOriginalMesh(u32 inx, const path& _mesh)
            {
                mb_original[inx] = (SMeshBuffer*)(dev->smgr->getMesh(_mesh))->getMeshBuffer(0);
                v_quant = mb_original[inx]->getVertexCount();
                i_quant = mb_original[inx]->getIndexCount();

                if (DEBUG_INFO) cout << "Mesh loaded: vertexes=" << v_quant << " , indexes=" << i_quant << endl;

            }

            //! Добавляет в мешбуфер дубль
            void dublicateTo(u32 orig_inx, int x, int y,int z, vector3df r = vector3df(0))
            {
                if ( (mb_storage->getVertexCount() + mb_original[orig_inx]->getVertexCount()) > 65536 )
                {
                    if (DEBUG_INFO) cout << "Meshbuffer is full\n";
                    return;
                }

                //Очищаем блок
                if (orig_inx == 0)
                {
                    dublicateDel(x,y);
                    return;
                }

                matrix4 m;

                //Получаем оригинал меша с индексом [orig_inx-1], производя смещение, т.к. нулевой индекс - очищает блок
                SMeshBuffer * buf = (SMeshBuffer*)cloneMeshBuffer(mb_original[orig_inx-1]);
                m.makeIdentity();
                m.setTranslation(vector3df(-x, y, z));
                m.setRotationDegrees(r);
                dev->smgr->getMeshManipulator()->transform(buf, m);

                appendBuffer( buf );

                if (DEBUG_INFO) cout << "Mesh clonned: XYZ=" << x << "," << y << "," << z << " clones_count: " << v_shifts.size()  << " shifts_v/i=" << mb_storage->getVertexCount() << "/" << mb_storage->getIndexCount() <<endl;

                delete buf;
            }

            void dublicateTo(IMeshBuffer* meshBuffer, int x, int y,int z, vector3df r = vector3df(0))
            {
                if ( (mb_storage->getVertexCount() + meshBuffer->getVertexCount()) > 65536 )
                {
                    if (DEBUG_INFO) cout << "Meshbuffer is full\n";
                    return;
                }

                matrix4 m;

                //Получаем оригинал клонируемого меша
                SMeshBuffer * buf = (SMeshBuffer*)cloneMeshBuffer(meshBuffer);
                vector3df pos = vector3df(Id.X*PARAM_CHUNK_SIZE, Id.Y*PARAM_CHUNK_HEIGHT, Id.Z*PARAM_CHUNK_SIZE) + vector3df(x, y, z);
                //vector3df pos = vector3df(x, y+Id.Y*PARAM_CHUNK_HEIGHT ,z);
                m.makeIdentity();
                m.setTranslation(pos);
                m.setRotationDegrees(r);
                dev->smgr->getMeshManipulator()->transform(buf, m);

                appendBuffer( buf );

                flags->FMeshBoundingBoxReload = true;

                if (DEBUG_INFO)
                {
                    ostringstream ss; ss <<"Mesh clonned: " << Id.X << "/" << Id.Y << "/" << Id.Z << " | XYZ=" << x << "," << y << "," << z << " clones_count: " << v_shifts.size()  << " shifts_v/i=" << mb_storage->getVertexCount() << "/" << mb_storage->getIndexCount() << endl;
                    dev->logger->write(ss.str().c_str());
                    cout << ss.str();
                }

                delete buf;
            }

            /*
                удаляет из мешбуфера дубликат
            */
            void dublicateDel(int x, int z)
            {
                //по координатам ищем индекс буфера
                for(int i=0, max=v_shifts.size(); i<max; i++)
                if ( int(mb_storage->Vertices[ v_shifts[i] ].Pos.X) == x && int(mb_storage->Vertices[ v_shifts[i] ].Pos.Z) == z )
                {
                    removeBuffer(i);
                    break;
                }
            }

            void blockDel(vector3di blockPos, bool generatePlanes=true, bool N=true, bool S=true, bool W=true, bool E=true, bool U=true, bool D=true)
            {
                if (DEBUG_INFO)
                {
                    ostringstream ss; ss << "Trying to delete block in chunk(" << Id.X << "/" << Id.Y << "/" << Id.Z << ") on XYZ = " << blockPos.X << " / " << blockPos.Y << " / " << blockPos.Z << " :";
                    dev->logger->write(ss.str().c_str());
                    cout << ss.str();
                }

                if (blockPos.X < 0 || blockPos.Z < 0 || blockPos.Y < 0
                    || blockPos.X > (PARAM_CHUNK_SIZE-1) || blockPos.Z > (PARAM_CHUNK_SIZE-1) || blockPos.Y > (PARAM_CHUNK_HEIGHT-1))
                {
                    ostringstream ss; ss << "ERROR: wrong block coordinates" << endl;
                    dev->logger->write(ss.str().c_str());
                    cout << ss.str();
                    return;
                }

                if (matrix[blockPos.X][blockPos.Z][blockPos.Y] == 0 && DEBUG_INFO)
                {
                    ostringstream ss; ss << "ERROR: block does not exist" << endl;
                    dev->logger->write(ss.str().c_str());
                    cout << ss.str();
                    return;
                }

                //удаляем вершины и индексы начиная с указанного смещения
                if (N && NShifts[blockPos.X][blockPos.Z][blockPos.Y] != -1)
                {
                    if (DEBUG_INFO)
                    {
                        ostringstream ss; ss << "N";
                        dev->logger->write(ss.str().c_str());
                        cout << ss.str();
                    }
                    mb_storage->Vertices.erase(NShifts[blockPos.X][blockPos.Z][blockPos.Y],6);
                    mb_storage->Indices.erase(NShifts[blockPos.X][blockPos.Z][blockPos.Y],6);
                    recalculateIndices(NShifts[blockPos.X][blockPos.Z][blockPos.Y],6);
                    NShifts[blockPos.X][blockPos.Z][blockPos.Y] = -1;
                }

                if (S && SShifts[blockPos.X][blockPos.Z][blockPos.Y] != -1)
                {
                    if (DEBUG_INFO)
                    {
                        ostringstream ss; ss << "S";
                        dev->logger->write(ss.str().c_str());
                        cout << ss.str();
                    }
                    mb_storage->Vertices.erase(SShifts[blockPos.X][blockPos.Z][blockPos.Y],6);
                    mb_storage->Indices.erase(SShifts[blockPos.X][blockPos.Z][blockPos.Y],6);
                    recalculateIndices(SShifts[blockPos.X][blockPos.Z][blockPos.Y],6);
                    SShifts[blockPos.X][blockPos.Z][blockPos.Y] = -1;
                }

                if (W && WShifts[blockPos.X][blockPos.Z][blockPos.Y] != -1)
                {
                    if (DEBUG_INFO)
                    {
                        ostringstream ss; ss << "W";
                        dev->logger->write(ss.str().c_str());
                        cout << ss.str();
                    }
                    mb_storage->Vertices.erase(WShifts[blockPos.X][blockPos.Z][blockPos.Y],6);
                    mb_storage->Indices.erase(WShifts[blockPos.X][blockPos.Z][blockPos.Y],6);
                    recalculateIndices(WShifts[blockPos.X][blockPos.Z][blockPos.Y],6);
                    WShifts[blockPos.X][blockPos.Z][blockPos.Y] = -1;
                }

                if (E && EShifts[blockPos.X][blockPos.Z][blockPos.Y] != -1)
                {
                    if (DEBUG_INFO)
                    {
                        ostringstream ss; ss << "E";
                        dev->logger->write(ss.str().c_str());
                        cout << ss.str();
                    }
                    mb_storage->Vertices.erase(EShifts[blockPos.X][blockPos.Z][blockPos.Y],6);
                    mb_storage->Indices.erase(EShifts[blockPos.X][blockPos.Z][blockPos.Y],6);
                    recalculateIndices(EShifts[blockPos.X][blockPos.Z][blockPos.Y],6);
                    EShifts[blockPos.X][blockPos.Z][blockPos.Y] = -1;
                }

                if (U && UShifts[blockPos.X][blockPos.Z][blockPos.Y] != -1)
                {
                    if (DEBUG_INFO)
                    {
                        ostringstream ss; ss << "U";
                        dev->logger->write(ss.str().c_str());
                        cout << ss.str();
                    }
                    mb_storage->Vertices.erase(UShifts[blockPos.X][blockPos.Z][blockPos.Y],6);
                    mb_storage->Indices.erase(UShifts[blockPos.X][blockPos.Z][blockPos.Y],6);
                    recalculateIndices(UShifts[blockPos.X][blockPos.Z][blockPos.Y],6);
                    UShifts[blockPos.X][blockPos.Z][blockPos.Y] = -1;
                }

                if (D && DShifts[blockPos.X][blockPos.Z][blockPos.Y] != -1)
                {
                    if (DEBUG_INFO)
                    {
                        ostringstream ss; ss << "D";
                        dev->logger->write(ss.str().c_str());
                        cout << ss.str();
                    }
                    mb_storage->Vertices.erase(DShifts[blockPos.X][blockPos.Z][blockPos.Y],6);
                    mb_storage->Indices.erase(DShifts[blockPos.X][blockPos.Z][blockPos.Y],6);
                    recalculateIndices(DShifts[blockPos.X][blockPos.Z][blockPos.Y],6);
                    DShifts[blockPos.X][blockPos.Z][blockPos.Y] = -1;
                }

                if (DEBUG_INFO)
                {
                    ostringstream ss; ss << endl;
                    dev->logger->write(ss.str().c_str());
                    cout << ss.str();
                }

                if (generatePlanes) generatePlaneMesh(blockPos);

                //если сторон куба не осталось, то куб спрятан
                if (NShifts[blockPos.X][blockPos.Z][blockPos.Y] == -1 &&
                    SShifts[blockPos.X][blockPos.Z][blockPos.Y] == -1 &&
                    WShifts[blockPos.X][blockPos.Z][blockPos.Y] == -1 &&
                    EShifts[blockPos.X][blockPos.Z][blockPos.Y] == -1 &&
                    UShifts[blockPos.X][blockPos.Z][blockPos.Y] == -1 &&
                    DShifts[blockPos.X][blockPos.Z][blockPos.Y] == -1)
                    {
                        int n,s,w,e,u,d;
                        if (blockPos.X == 0)
                        {
                            s = 0;
                            n = matrix[blockPos.X+1][blockPos.Z][blockPos.Y];
                        } else if (blockPos.X >= PARAM_CHUNK_SIZE-1) {
                            n = 0;
                            s = matrix[blockPos.X-1][blockPos.Z][blockPos.Y];
                        } else {
                            s = matrix[blockPos.X-1][blockPos.Z][blockPos.Y];
                            n = matrix[blockPos.X+1][blockPos.Z][blockPos.Y];
                        }
                        if (blockPos.Z == 0)
                        {
                            w = 0;
                            e = matrix[blockPos.X][blockPos.Z+1][blockPos.Y];
                        } else if (blockPos.Z >= PARAM_CHUNK_SIZE-1) {
                            e = 0;
                            w = matrix[blockPos.X][blockPos.Z-1][blockPos.Y];
                        } else {
                            w = matrix[blockPos.X][blockPos.Z-1][blockPos.Y];
                            e = matrix[blockPos.X][blockPos.Z+1][blockPos.Y];
                        }
                        if (blockPos.Y == 0)
                        {
                            d = 0;
                            u = matrix[blockPos.X][blockPos.Z][blockPos.Y+1];
                        } else if (blockPos.Y >= PARAM_CHUNK_HEIGHT-1) {
                            u = 0;
                            d = matrix[blockPos.X][blockPos.Z][blockPos.Y-1];
                        } else {
                            d = matrix[blockPos.X][blockPos.Z][blockPos.Y-1];
                            u = matrix[blockPos.X][blockPos.Z][blockPos.Y+1];
                        }

                        //если хоть с одной стороны нет блока, то можем удалять блок
                        if (n==0 || s==0 || w==0 || e==0 || u==0 || d==0)
                        {
                            matrix[blockPos.X][blockPos.Z][blockPos.Y] = 0;
                            if (DEBUG_INFO)
                            {
                                ostringstream ss; ss << "Block (" << blockPos.X << " / " << blockPos.Y << " / " << blockPos.Z << ") deleted" << endl;
                                dev->logger->write(ss.str().c_str());
                                cout << ss.str();
                            }
                        //но если со всех сторон выделенный блок окружен другими блоками, то прячем его
                        } else if (n==1 && s==1 && w==1 && e==1 && u==1 && d==1) {
                            if (DEBUG_INFO)
                            {
                                ostringstream ss; ss << "Block (" << blockPos.X << " / " << blockPos.Y << " / " << blockPos.Z << ") hided" << endl;
                                dev->logger->write(ss.str().c_str());
                                cout << ss.str();
                            }
                            matrix[blockPos.X][blockPos.Z][blockPos.Y] = 1;
                        }
                    }

                flags->FMeshBoundingBoxReload = true;

                mb_storage->recalculateBoundingBox();
                mb_storage->setDirty();
                return;
            }

            //! Создание нового блока
            bool blockAdd(vector3di selectedBlockPos, vector3df intersection)
            {
                int N,S,W,E,U,D;
                N = S = W = E = U = D = -1;
                vector3df blockPos = vector3df((f32)selectedBlockPos.X,(f32)selectedBlockPos.Y,(f32)selectedBlockPos.Z);

                if (DEBUG_INFO)
                {
                    ostringstream ss; ss << "Selected block XYZ = " << selectedBlockPos.X << " / " << selectedBlockPos.Y << " / " << selectedBlockPos.Z << endl;
                    dev->logger->write(ss.str().c_str());
                    cout << ss.str();
                }

                if (intersection.Y == selectedBlockPos.Y) blockPos += vector3df(0,PARAM_BLOCK_SIZE,0);
                if (intersection.Y == selectedBlockPos.Y-PARAM_BLOCK_SIZE) blockPos -= vector3df(0,PARAM_BLOCK_SIZE,0);
                if (intersection.X == selectedBlockPos.X+PARAM_BLOCK_SIZE/2) blockPos += vector3df(PARAM_BLOCK_SIZE/2+PARAM_BLOCK_SIZE/10,0,0);
                if (intersection.X == selectedBlockPos.X-PARAM_BLOCK_SIZE/2) blockPos -= vector3df(PARAM_BLOCK_SIZE/2+PARAM_BLOCK_SIZE/10,0,0);
                if (intersection.Z == selectedBlockPos.Z+PARAM_BLOCK_SIZE/2) blockPos += vector3df(0,0,PARAM_BLOCK_SIZE/2+PARAM_BLOCK_SIZE/10);
                if (intersection.Z == selectedBlockPos.Z-PARAM_BLOCK_SIZE/2) blockPos -= vector3df(0,0,PARAM_BLOCK_SIZE/2+PARAM_BLOCK_SIZE/10);

                vector3di iblockPos = vector3di((int)rint(blockPos.X),(int)rint(blockPos.Y),(int)rint(blockPos.Z));

                //проверка доступности места для блока
                if (iblockPos.X < 0 || iblockPos.X > PARAM_CHUNK_SIZE-1 ||
                    iblockPos.Z < 0 || iblockPos.Z > PARAM_CHUNK_SIZE-1 ||
                    iblockPos.Y < 0 || iblockPos.Y > PARAM_CHUNK_HEIGHT-1)
                {
                    if (DEBUG_INFO)
                    {
                        ostringstream ss; ss << "Cant`t add block here" << endl;
                        dev->logger->write(ss.str().c_str());
                        cout << ss.str();
                    }
                     return false;
                }

                //Y
                if (iblockPos.Y == 0)
                {
                    D = 0;
                } else {
                    if (matrix[iblockPos.X][iblockPos.Z][iblockPos.Y-1] == 0) D = 0;
                }
                if (iblockPos.Y == PARAM_CHUNK_HEIGHT-1) {
                    U = 0;
                } else {
                    if (matrix[iblockPos.X][iblockPos.Z][iblockPos.Y+1] == 0) U = 0;
                }
                //X
                if (iblockPos.X == 0)
                {
                    S = 0;
                } else {
                    if (matrix[iblockPos.X-1][iblockPos.Z][iblockPos.Y] == 0) S = 0;
                }
                if (iblockPos.X == PARAM_CHUNK_SIZE-1)
                {
                    N = 0;
                } else {
                    if (matrix[iblockPos.X+1][iblockPos.Z][iblockPos.Y] == 0) N = 0;
                }
                //Z
                if (iblockPos.Z == 0)
                {
                    W = 0;
                } else {
                    if (matrix[iblockPos.X][iblockPos.Z-1][iblockPos.Y] == 0) W = 0;
                }
                if (iblockPos.Z == PARAM_CHUNK_SIZE-1)
                {
                    E = 0;
                } else {
                    if (matrix[iblockPos.X][iblockPos.Z+1][iblockPos.Y] == 0) E = 0;
                }


                if (DEBUG_INFO)
                {
                    ostringstream ss; ss << "Trying to add block on iXYZ = " << iblockPos.X << " / " << iblockPos.Y << " / " << iblockPos.Z << endl;
                    dev->logger->write(ss.str().c_str());
                    cout << ss.str();
                }

                if (matrix[iblockPos.X][iblockPos.Z][iblockPos.Y] == 1 && DEBUG_INFO)
                {
                    ostringstream ss; ss << "ERROR: block already exist";
                    dev->logger->write(ss.str().c_str());
                    cout << ss.str();
                    return false;
                } else {

                    dublicateTo(createPlaneMesh(iblockPos,N,S,W,E,U,D),iblockPos.X, iblockPos.Y, iblockPos.Z);
                    matrix[iblockPos.X][iblockPos.Z][iblockPos.Y] = 1;

                    //Удалим ненужные плоскости

                                                                //        N     S      W      E      U      D
                    if (N==0 || (N==-1 && S==-1))
                    {
                        if (iblockPos.X>0)
                            blockDel(iblockPos-vector3di(1,0,0), false, true, false, false, false, false, false);
                    }
                    if (S==0 || (N==-1 && S==-1))
                    {
                        if (iblockPos.X<PARAM_CHUNK_SIZE-1)
                            blockDel(iblockPos+vector3di(1,0,0), false, false, true, false, false, false, false);
                    }
                    if (W==0 || (W==-1 && E==-1))
                    {
                        if (iblockPos.Z<PARAM_CHUNK_SIZE-1)
                            blockDel(iblockPos+vector3di(0,0,1), false, false, false, true, false, false, false);
                    }
                    if (E==0 || (W==-1 && E==-1))
                    {
                        if (iblockPos.Z>0)
                            blockDel(iblockPos-vector3di(0,0,1), false, false, false, false, true, false, false);
                    }
                    if (U==0 || (U==-1 && D==-1))
                    {
                        if (iblockPos.Y>0)
                            blockDel(iblockPos-vector3di(0,1,0), false, false, false, false, false, true, false);
                    }
                    if (D==0 || (U==-1 && D==-1))
                    {
                        if (iblockPos.Y<PARAM_CHUNK_HEIGHT-1)
                            blockDel(iblockPos+vector3di(0,1,0), false, false, false, false, false, false, true);
                    }

                }

                return true;
            }

            IMeshBuffer* createPlaneMesh(vector3di pos, int N, int S, int W, int E, int U, int D)
            {
                CMeshBuffer<S3DVertex>* mb = new CMeshBuffer<S3DVertex>();
                u32 shift = mb_storage->getVertexCount();

                if (DEBUG_INFO)
                {
                    ostringstream ss; ss << "Creating planes: ";
                    dev->logger->write(ss.str().c_str());
                    cout << ss.str();
                }

                if (U == 0) //верхняя плоскость
                {
                    S3DVertex v0,v1,v2,v3,v4,v5;
                    UShifts[pos.X][pos.Z][pos.Y] = shift;

                    u32 startIndex = mb->getVertexCount();

                    v0.Color.set(255, 255, 255, 255);
                    v0.Pos.set(0 + PARAM_BLOCK_SIZE/2, 0, 0 - PARAM_BLOCK_SIZE/2);
                    v0.TCoords.set((float)(textureId.X-1)/PARAM_MULTITEX_COUNT,(float)textureId.Y/PARAM_MULTITEX_COUNT);

                    v1.Color.set(255, 255, 255, 255);
                    v1.Pos.set(0 + PARAM_BLOCK_SIZE/2, 0, 0 + PARAM_BLOCK_SIZE/2);
                    v1.TCoords.set((float)textureId.X/PARAM_MULTITEX_COUNT,(float)textureId.Y/PARAM_MULTITEX_COUNT);

                    v2.Color.set(255, 255, 255, 255);
                    v2.Pos.set(0 - PARAM_BLOCK_SIZE/2, 0, 0 + PARAM_BLOCK_SIZE/2);
                    v2.TCoords.set((float)textureId.X/PARAM_MULTITEX_COUNT,(float)(textureId.Y-1)/PARAM_MULTITEX_COUNT);

                    v3.Color.set(255, 255, 255, 255);
                    v3.Pos.set(0 + PARAM_BLOCK_SIZE/2, 0, 0 - PARAM_BLOCK_SIZE/2);
                    v3.TCoords.set((float)(textureId.X-1)/PARAM_MULTITEX_COUNT,(float)textureId.Y/PARAM_MULTITEX_COUNT);

                    v4.Color.set(255, 255, 255, 255);
                    v4.Pos.set(0 - PARAM_BLOCK_SIZE/2, 0, 0 + PARAM_BLOCK_SIZE/2);
                    v4.TCoords.set((float)textureId.X/PARAM_MULTITEX_COUNT,(float)(textureId.Y-1)/PARAM_MULTITEX_COUNT);

                    v5.Color.set(255, 255, 255, 255);
                    v5.Pos.set(0 - PARAM_BLOCK_SIZE/2, 0, 0 - PARAM_BLOCK_SIZE/2);
                    v5.TCoords.set((float)(textureId.X-1)/PARAM_MULTITEX_COUNT,(float)(textureId.Y-1)/PARAM_MULTITEX_COUNT);

                    mb->Vertices.push_back(v0);
                    mb->Vertices.push_back(v1);
                    mb->Vertices.push_back(v2);
                    mb->Vertices.push_back(v3);
                    mb->Vertices.push_back(v4);
                    mb->Vertices.push_back(v5);

                    mb->Indices.push_back(startIndex + 0);
                    mb->Indices.push_back(startIndex + 2);
                    mb->Indices.push_back(startIndex + 1);
                    mb->Indices.push_back(startIndex + 3);
                    mb->Indices.push_back(startIndex + 5);
                    mb->Indices.push_back(startIndex + 4);

                    shift += 6;
                    if (DEBUG_INFO)
                    {
                        ostringstream ss; ss << "U";
                        dev->logger->write(ss.str().c_str());
                        cout << ss.str();
                    }
                }

                if (D == 0) //нижняя плоскость
                {
                    S3DVertex v0,v1,v2,v3,v4,v5;
                    DShifts[pos.X][pos.Z][pos.Y] = shift;

                    u32 startIndex = mb->getVertexCount();

                    v0.Color.set(255, 255, 255, 255);
                    v0.Pos.set(0 + PARAM_BLOCK_SIZE/2, 0 - PARAM_BLOCK_SIZE, 0 - PARAM_BLOCK_SIZE/2);
                    v0.TCoords.set((float)(textureId.X-1)/PARAM_MULTITEX_COUNT,(float)textureId.Y/PARAM_MULTITEX_COUNT);

                    v1.Color.set(255, 255, 255, 255);
                    v1.Pos.set(0 + PARAM_BLOCK_SIZE/2, 0 - PARAM_BLOCK_SIZE, 0 + PARAM_BLOCK_SIZE/2);
                    v1.TCoords.set((float)textureId.X/PARAM_MULTITEX_COUNT,(float)textureId.Y/PARAM_MULTITEX_COUNT);

                    v2.Color.set(255, 255, 255, 255);
                    v2.Pos.set(0 - PARAM_BLOCK_SIZE/2, 0 - PARAM_BLOCK_SIZE, 0 + PARAM_BLOCK_SIZE/2);
                    v2.TCoords.set((float)textureId.X/PARAM_MULTITEX_COUNT,(float)(textureId.Y-1)/PARAM_MULTITEX_COUNT);

                    v3.Color.set(255, 255, 255, 255);
                    v3.Pos.set(0 + PARAM_BLOCK_SIZE/2, 0 - PARAM_BLOCK_SIZE, 0 - PARAM_BLOCK_SIZE/2);
                    v3.TCoords.set((float)(textureId.X-1)/PARAM_MULTITEX_COUNT,(float)textureId.Y/PARAM_MULTITEX_COUNT);

                    v4.Color.set(255, 255, 255, 255);
                    v4.Pos.set(0 - PARAM_BLOCK_SIZE/2, 0 - PARAM_BLOCK_SIZE, 0 + PARAM_BLOCK_SIZE/2);
                    v4.TCoords.set((float)textureId.X/PARAM_MULTITEX_COUNT,(float)(textureId.Y-1)/PARAM_MULTITEX_COUNT);

                    v5.Color.set(255, 255, 255, 255);
                    v5.Pos.set(0 - PARAM_BLOCK_SIZE/2, 0 - PARAM_BLOCK_SIZE, 0 - PARAM_BLOCK_SIZE/2);
                    v5.TCoords.set((float)(textureId.X-1)/PARAM_MULTITEX_COUNT,(float)(textureId.Y-1)/PARAM_MULTITEX_COUNT);

                    mb->Vertices.push_back(v0);
                    mb->Vertices.push_back(v1);
                    mb->Vertices.push_back(v2);
                    mb->Vertices.push_back(v3);
                    mb->Vertices.push_back(v4);
                    mb->Vertices.push_back(v5);

                    mb->Indices.push_back(startIndex + 0);
                    mb->Indices.push_back(startIndex + 1);
                    mb->Indices.push_back(startIndex + 2);
                    mb->Indices.push_back(startIndex + 5);
                    mb->Indices.push_back(startIndex + 3);
                    mb->Indices.push_back(startIndex + 4);

                    shift += 6;
                    if (DEBUG_INFO)
                    {
                        ostringstream ss; ss << "D";
                        dev->logger->write(ss.str().c_str());
                        cout << ss.str();
                    }
                }

                if (N == 0) //передняя плоскость (север)
                {
                    S3DVertex v0,v1,v2,v3,v4,v5;
                    NShifts[pos.X][pos.Z][pos.Y] = shift;

                    u32 startIndex = mb->getVertexCount();

                    v0.Color.set(255, 255, 255, 255);
                    v0.Pos.set(0 + PARAM_BLOCK_SIZE/2, 0, 0 - PARAM_BLOCK_SIZE/2);
                    v0.TCoords.set((float)(textureId.X-1)/PARAM_MULTITEX_COUNT,(float)(textureId.Y-1)/PARAM_MULTITEX_COUNT);

                    v1.Color.set(255, 255, 255, 255);
                    v1.Pos.set(0 + PARAM_BLOCK_SIZE/2, 0, 0 + PARAM_BLOCK_SIZE/2);
                    v1.TCoords.set((float)textureId.X/PARAM_MULTITEX_COUNT,(float)(textureId.Y-1)/PARAM_MULTITEX_COUNT);

                    v2.Color.set(255, 255, 255, 255);
                    v2.Pos.set(0 + PARAM_BLOCK_SIZE/2, 0 - PARAM_BLOCK_SIZE, 0 + PARAM_BLOCK_SIZE/2);
                    v2.TCoords.set((float)textureId.X/PARAM_MULTITEX_COUNT,(float)textureId.Y/PARAM_MULTITEX_COUNT);

                    v3.Color.set(255, 255, 255, 255);
                    v3.Pos.set(0 + PARAM_BLOCK_SIZE/2, 0, 0 - PARAM_BLOCK_SIZE/2);
                    v3.TCoords.set((float)(textureId.X-1)/PARAM_MULTITEX_COUNT,(float)(textureId.Y-1)/PARAM_MULTITEX_COUNT);

                    v4.Color.set(255, 255, 255, 255);
                    v4.Pos.set(0 + PARAM_BLOCK_SIZE/2, 0 - PARAM_BLOCK_SIZE, 0 + PARAM_BLOCK_SIZE/2);
                    v4.TCoords.set((float)textureId.X/PARAM_MULTITEX_COUNT,(float)textureId.Y/PARAM_MULTITEX_COUNT);

                    v5.Color.set(255, 255, 255, 255);
                    v5.Pos.set(0 + PARAM_BLOCK_SIZE/2, 0 - PARAM_BLOCK_SIZE, 0 - PARAM_BLOCK_SIZE/2);
                    v5.TCoords.set((float)(textureId.X-1)/PARAM_MULTITEX_COUNT,(float)textureId.Y/PARAM_MULTITEX_COUNT);

                    mb->Vertices.push_back(v0);
                    mb->Vertices.push_back(v1);
                    mb->Vertices.push_back(v2);
                    mb->Vertices.push_back(v3);
                    mb->Vertices.push_back(v4);
                    mb->Vertices.push_back(v5);

                    mb->Indices.push_back(startIndex + 0);
                    mb->Indices.push_back(startIndex + 1);
                    mb->Indices.push_back(startIndex + 2);
                    mb->Indices.push_back(startIndex + 5);
                    mb->Indices.push_back(startIndex + 3);
                    mb->Indices.push_back(startIndex + 4);

                    shift += 6;
                    if (DEBUG_INFO)
                    {
                        ostringstream ss; ss << "N";
                        dev->logger->write(ss.str().c_str());
                        cout << ss.str();
                    }
                }

                if (S == 0) //задняя плоскость (юг)
                {
                    S3DVertex v0,v1,v2,v3,v4,v5;
                    SShifts[pos.X][pos.Z][pos.Y] = shift;

                    u32 startIndex = mb->getVertexCount();

                    v0.Color.set(255, 255, 255, 255);
                    v0.Pos.set(0 - PARAM_BLOCK_SIZE/2, 0, 0 - PARAM_BLOCK_SIZE/2);
                    v0.TCoords.set((float)textureId.X/PARAM_MULTITEX_COUNT,(float)(textureId.Y-1)/PARAM_MULTITEX_COUNT);

                    v1.Color.set(255, 255, 255, 255);
                    v1.Pos.set(0 - PARAM_BLOCK_SIZE/2, 0, 0 + PARAM_BLOCK_SIZE/2);
                    v1.TCoords.set((float)(textureId.X-1)/PARAM_MULTITEX_COUNT,(float)(textureId.Y-1)/PARAM_MULTITEX_COUNT);

                    v2.Color.set(255, 255, 255, 255);
                    v2.Pos.set(0 - PARAM_BLOCK_SIZE/2, 0 - PARAM_BLOCK_SIZE, 0 + PARAM_BLOCK_SIZE/2);
                    v2.TCoords.set((float)(textureId.X-1)/PARAM_MULTITEX_COUNT,(float)textureId.Y/PARAM_MULTITEX_COUNT);

                    v3.Color.set(255, 255, 255, 255);
                    v3.Pos.set(0 - PARAM_BLOCK_SIZE/2, 0, 0 - PARAM_BLOCK_SIZE/2);
                    v3.TCoords.set((float)textureId.X/PARAM_MULTITEX_COUNT,(float)(textureId.Y-1)/PARAM_MULTITEX_COUNT);

                    v4.Color.set(255, 255, 255, 255);
                    v4.Pos.set(0 - PARAM_BLOCK_SIZE/2, 0 - PARAM_BLOCK_SIZE, 0 + PARAM_BLOCK_SIZE/2);
                    v4.TCoords.set((float)(textureId.X-1)/PARAM_MULTITEX_COUNT,(float)textureId.Y/PARAM_MULTITEX_COUNT);

                    v5.Color.set(255, 255, 255, 255);
                    v5.Pos.set(0 - PARAM_BLOCK_SIZE/2, 0 - PARAM_BLOCK_SIZE, 0 - PARAM_BLOCK_SIZE/2);
                    v5.TCoords.set((float)textureId.X/PARAM_MULTITEX_COUNT,(float)textureId.Y/PARAM_MULTITEX_COUNT);

                    mb->Vertices.push_back(v0);
                    mb->Vertices.push_back(v1);
                    mb->Vertices.push_back(v2);
                    mb->Vertices.push_back(v3);
                    mb->Vertices.push_back(v4);
                    mb->Vertices.push_back(v5);

                    mb->Indices.push_back(startIndex + 0);
                    mb->Indices.push_back(startIndex + 2);
                    mb->Indices.push_back(startIndex + 1);
                    mb->Indices.push_back(startIndex + 3);
                    mb->Indices.push_back(startIndex + 5);
                    mb->Indices.push_back(startIndex + 4);

                    shift += 6;
                    if (DEBUG_INFO)
                    {
                        ostringstream ss; ss << "S";
                        dev->logger->write(ss.str().c_str());
                        cout << ss.str();
                    }
                }

                if (E == 0) //правая плоскость (восток)
                {
                    S3DVertex v0,v1,v2,v3,v4,v5;
                    EShifts[pos.X][pos.Z][pos.Y] = shift;

                    u32 startIndex = mb->getVertexCount();

                    v0.Color.set(255, 255, 255, 255);
                    v0.Pos.set(0 + PARAM_BLOCK_SIZE/2, 0 - PARAM_BLOCK_SIZE, 0 + PARAM_BLOCK_SIZE/2);
                    v0.TCoords.set((float)(textureId.X-1)/PARAM_MULTITEX_COUNT,(float)textureId.Y/PARAM_MULTITEX_COUNT);

                    v1.Color.set(255, 255, 255, 255);
                    v1.Pos.set(0 + PARAM_BLOCK_SIZE/2, 0, 0 + PARAM_BLOCK_SIZE/2);
                    v1.TCoords.set((float)(textureId.X-1)/PARAM_MULTITEX_COUNT,(float)(textureId.Y-1)/PARAM_MULTITEX_COUNT);

                    v2.Color.set(255, 255, 255, 255);
                    v2.Pos.set(0 - PARAM_BLOCK_SIZE/2, 0, 0 + PARAM_BLOCK_SIZE/2);
                    v2.TCoords.set((float)textureId.X/PARAM_MULTITEX_COUNT,(float)(textureId.Y-1)/PARAM_MULTITEX_COUNT);

                    v3.Color.set(255, 255, 255, 255);
                    v3.Pos.set(0 + PARAM_BLOCK_SIZE/2, 0 - PARAM_BLOCK_SIZE, 0 + PARAM_BLOCK_SIZE/2);
                    v3.TCoords.set((float)(textureId.X-1)/PARAM_MULTITEX_COUNT,(float)textureId.Y/PARAM_MULTITEX_COUNT);

                    v4.Color.set(255, 255, 255, 255);
                    v4.Pos.set(0 - PARAM_BLOCK_SIZE/2, 0, 0 + PARAM_BLOCK_SIZE/2);
                    v4.TCoords.set((float)textureId.X/PARAM_MULTITEX_COUNT,(float)(textureId.Y-1)/PARAM_MULTITEX_COUNT);

                    v5.Color.set(255, 255, 255, 255);
                    v5.Pos.set(0 - PARAM_BLOCK_SIZE/2, 0 - PARAM_BLOCK_SIZE, 0 + PARAM_BLOCK_SIZE/2);
                    v5.TCoords.set((float)textureId.X/PARAM_MULTITEX_COUNT,(float)textureId.Y/PARAM_MULTITEX_COUNT);

                    mb->Vertices.push_back(v0);
                    mb->Vertices.push_back(v1);
                    mb->Vertices.push_back(v2);
                    mb->Vertices.push_back(v3);
                    mb->Vertices.push_back(v4);
                    mb->Vertices.push_back(v5);

                    mb->Indices.push_back(startIndex + 0);
                    mb->Indices.push_back(startIndex + 1);
                    mb->Indices.push_back(startIndex + 2);
                    mb->Indices.push_back(startIndex + 5);
                    mb->Indices.push_back(startIndex + 3);
                    mb->Indices.push_back(startIndex + 4);

                    shift += 6;
                    if (DEBUG_INFO)
                    {
                        ostringstream ss; ss << "E";
                        dev->logger->write(ss.str().c_str());
                        cout << ss.str();
                    }
                }

                if (W == 0) //левая плоскость (запад)
                {
                    S3DVertex v0,v1,v2,v3,v4,v5;
                    WShifts[pos.X][pos.Z][pos.Y] = shift;

                    u32 startIndex = mb->getVertexCount();

                    v0.Color.set(255, 255, 255, 255);
                    v0.Pos.set(0 + PARAM_BLOCK_SIZE/2, 0 - PARAM_BLOCK_SIZE, 0 - PARAM_BLOCK_SIZE/2);
                    v0.TCoords.set((float)textureId.X/PARAM_MULTITEX_COUNT,(float)textureId.Y/PARAM_MULTITEX_COUNT);

                    v1.Color.set(255, 255, 255, 255);
                    v1.Pos.set(0 + PARAM_BLOCK_SIZE/2, 0, 0 - PARAM_BLOCK_SIZE/2);
                    v1.TCoords.set((float)textureId.X/PARAM_MULTITEX_COUNT,(float)(textureId.Y-1)/PARAM_MULTITEX_COUNT);

                    v2.Color.set(255, 255, 255, 255);
                    v2.Pos.set(0 - PARAM_BLOCK_SIZE/2, 0, 0 - PARAM_BLOCK_SIZE/2);
                    v2.TCoords.set((float)(textureId.X-1)/PARAM_MULTITEX_COUNT,(float)(textureId.Y-1)/PARAM_MULTITEX_COUNT);

                    v3.Color.set(255, 255, 255, 255);
                    v3.Pos.set(0 + PARAM_BLOCK_SIZE/2, 0 - PARAM_BLOCK_SIZE, 0 - PARAM_BLOCK_SIZE/2);
                    v3.TCoords.set((float)textureId.X/PARAM_MULTITEX_COUNT,(float)textureId.Y/PARAM_MULTITEX_COUNT);

                    v4.Color.set(255, 255, 255, 255);
                    v4.Pos.set(0 - PARAM_BLOCK_SIZE/2, 0, 0 - PARAM_BLOCK_SIZE/2);
                    v4.TCoords.set((float)(textureId.X-1)/PARAM_MULTITEX_COUNT,(float)(textureId.Y-1)/PARAM_MULTITEX_COUNT);

                    v5.Color.set(255, 255, 255, 255);
                    v5.Pos.set(0 - PARAM_BLOCK_SIZE/2, 0 - PARAM_BLOCK_SIZE, 0 - PARAM_BLOCK_SIZE/2);
                    v5.TCoords.set((float)(textureId.X-1)/PARAM_MULTITEX_COUNT,(float)textureId.Y/PARAM_MULTITEX_COUNT);

                    mb->Vertices.push_back(v0);
                    mb->Vertices.push_back(v1);
                    mb->Vertices.push_back(v2);
                    mb->Vertices.push_back(v3);
                    mb->Vertices.push_back(v4);
                    mb->Vertices.push_back(v5);

                    mb->Indices.push_back(startIndex + 0);
                    mb->Indices.push_back(startIndex + 2);
                    mb->Indices.push_back(startIndex + 1);
                    mb->Indices.push_back(startIndex + 3);
                    mb->Indices.push_back(startIndex + 5);
                    mb->Indices.push_back(startIndex + 4);

                    if (DEBUG_INFO)
                    {
                        ostringstream ss; ss << "W";
                        dev->logger->write(ss.str().c_str());
                        cout << ss.str();
                    }
                }

                if (DEBUG_INFO)
                {
                    ostringstream ss; ss << endl;
                    dev->logger->write(ss.str().c_str());
                    cout << ss.str();
                }

                mb->Vertices.set_used(mb->getVertexCount());
                mb->Indices.set_used(mb->getVertexCount());

                mb->setDirty();
                return mb;
            }

            bool generatePlaneMesh(vector3di pos)
            {
                if (DEBUG_INFO)
                {
                    ostringstream ss; ss << "Generating block: " << endl;
                    dev->logger->write(ss.str().c_str());
                    cout << ss.str();
                }
                //разбор по оси X
                if (pos.X != 0 && matrix[pos.X-1][pos.Z][pos.Y] == 1)
                    dublicateTo(createPlaneMesh(vector3di(pos.X-1,pos.Y,pos.Z),0,-1,-1,-1,-1,-1),pos.X-1,pos.Y,pos.Z);
                if (pos.X != (PARAM_CHUNK_SIZE-1) && matrix[pos.X+1][pos.Z][pos.Y] == 1)
                    dublicateTo(createPlaneMesh(vector3di(pos.X+1,pos.Y,pos.Z),-1,0,-1,-1,-1,-1),pos.X+1,pos.Y,pos.Z);
                if (pos.X == 0 && Id.X>0 && glMatrix->getMValueFromCoord(vector3di(Id.X-1,Id.Y,Id.Z),vector3di(PARAM_CHUNK_SIZE-1,pos.Y,pos.Z)))
                    flags->pushToPlaneMaker(vector3di(Id.X-1,Id.Y,Id.Z), vector3di(PARAM_CHUNK_SIZE-1,pos.Y,pos.Z),0,-1,-1,-1,-1,-1);
                if (pos.X == (PARAM_CHUNK_SIZE-1) && Id.X<(PARAM_WORLD_SIZE_X-1) && glMatrix->getMValueFromCoord(vector3di(Id.X+1,Id.Y,Id.Z),vector3di(0,pos.Y,pos.Z)))
                    flags->pushToPlaneMaker(vector3di(Id.X+1,Id.Y,Id.Z), vector3di(0,pos.Y,pos.Z),-1,0,-1,-1,-1,-1);
                //разбор по оси z
                if (pos.Z != 0 && matrix[pos.X][pos.Z-1][pos.Y] == 1)
                    dublicateTo(createPlaneMesh(vector3di(pos.X,pos.Y,pos.Z-1),-1,-1,-1,0,-1,-1),pos.X,pos.Y,pos.Z-1);
                if (pos.Z != (PARAM_CHUNK_SIZE-1) && matrix[pos.X][pos.Z+1][pos.Y] == 1)
                    dublicateTo(createPlaneMesh(vector3di(pos.X,pos.Y,pos.Z+1),-1,-1,0,-1,-1,-1),pos.X,pos.Y,pos.Z+1);
                if (pos.Z == 0 && Id.Z>0 && glMatrix->getMValueFromCoord(vector3di(Id.X,Id.Y,Id.Z-1),vector3di(pos.X,pos.Y,PARAM_CHUNK_SIZE-1)))
                    flags->pushToPlaneMaker(vector3di(Id.X,Id.Y,Id.Z-1), vector3di(pos.X,pos.Y,PARAM_CHUNK_SIZE-1),-1,-1,-1,0,-1,-1);
                if (pos.Z == (PARAM_CHUNK_SIZE-1) && Id.Z<(PARAM_WORLD_SIZE_Z-1) && glMatrix->getMValueFromCoord(vector3di(Id.X,Id.Y,Id.Z+1),vector3di(pos.X,pos.Y,0)))
                    flags->pushToPlaneMaker(vector3di(Id.X,Id.Y,Id.Z+1), vector3di(pos.X,pos.Y,0),-1,-1,0,-1,-1,-1);
                //разбор по оси y
                if (pos.Y != 0 && matrix[pos.X][pos.Z][pos.Y-1] == 1)
                    dublicateTo(createPlaneMesh(vector3di(pos.X,pos.Y-1,pos.Z),-1,-1,-1,-1,0,-1),pos.X,pos.Y-1,pos.Z);
                if (pos.Y != (PARAM_CHUNK_HEIGHT-1) && matrix[pos.X][pos.Z][pos.Y+1] == 1)
                    dublicateTo(createPlaneMesh(vector3di(pos.X,pos.Y+1,pos.Z),-1,-1,-1,-1,-1,0),pos.X,pos.Y+1,pos.Z);
                if (pos.Y == 0 && Id.Y>0 && glMatrix->getMValueFromCoord(vector3di(Id.X,Id.Y-1,Id.Z),vector3di(pos.X,PARAM_CHUNK_HEIGHT-1,pos.Z)))
                    flags->pushToPlaneMaker(vector3di(Id.X,Id.Y-1,Id.Z), vector3di(pos.X,PARAM_CHUNK_HEIGHT-1,pos.Z),-1,-1,-1,-1,0,-1);
                if (pos.Y == (PARAM_CHUNK_HEIGHT-1) && Id.Y<(PARAM_WORLD_SIZE_Y-1) && glMatrix->getMValueFromCoord(vector3di(Id.X,Id.Y+1,Id.Z),vector3di(pos.X,0,pos.Z)))
                    flags->pushToPlaneMaker(vector3di(Id.X,Id.Y+1,Id.Z), vector3di(pos.X,0,pos.Z),-1,-1,-1,-1,-1,0);
                return true;
            }

            void render()
            {
                matrix4 oldWorldMat = dev->driver->getTransform(ETS_WORLD);

                dev->driver->setTransform(ETS_WORLD, matrix4() );

                dev->driver->setMaterial(mb_material);
                dev->driver->drawMeshBuffer(mb_storage);

                dev->driver->setTransform(ETS_WORLD, oldWorldMat);
            }

            //! рассчет позиции выделенного блока
            vector3di getBlockPos(vector3df cursorPos, vector3df cameraPos)
            {
                vector3di blockPos;
                if (cameraPos.X > cursorPos.X)
                    blockPos.X = (int)rint(cursorPos.X - PARAM_BLOCK_SIZE/10);
                else
                    blockPos.X = (int)rint(cursorPos.X + PARAM_BLOCK_SIZE/10);

                if (cameraPos.Z > cursorPos.Z)
                    blockPos.Z = (int)rint(cursorPos.Z - PARAM_BLOCK_SIZE/10);
                else
                    blockPos.Z = (int)rint(cursorPos.Z + PARAM_BLOCK_SIZE/10);

                if (cameraPos.Y > cursorPos.Y)
                    blockPos.Y = (int)rint(cursorPos.Y + PARAM_BLOCK_SIZE/2 - PARAM_BLOCK_SIZE/10);
                else
                    blockPos.Y = (int)rint(cursorPos.Y + PARAM_BLOCK_SIZE/2 + PARAM_BLOCK_SIZE/10);

                while (blockPos.X >= PARAM_CHUNK_SIZE-PARAM_BLOCK_SIZE/2)
                {
                    blockPos.X -= PARAM_CHUNK_SIZE;
                }
                while (blockPos.Z >= PARAM_CHUNK_SIZE-PARAM_BLOCK_SIZE/2)
                {
                    blockPos.Z -= PARAM_CHUNK_SIZE;
                }
                while (blockPos.Y >= PARAM_CHUNK_HEIGHT-PARAM_BLOCK_SIZE/2)
                {
                    blockPos.Y -= PARAM_CHUNK_HEIGHT;
                }
                return blockPos;
            }

            void checkBuffer()
            {
                if (DEBUG_INFO)
                {
                    ostringstream ss;
                    ss << "### Checking chunk: " << endl;

                    for (u32 y=0; y<PARAM_CHUNK_HEIGHT; y++)
                    {
                        ss << "## Floor " << y << endl;
                        for (u32 x=0;x<PARAM_CHUNK_SIZE; x++)
                        {
                            for (u32 z=0;z<PARAM_CHUNK_SIZE; z++)
                            {
                                ss << matrix[x][z][y] << "[";
                                if (NShifts[x][z][y] > 0 ) ss << "N"; else ss << "_";
                                if (SShifts[x][z][y] > 0 ) ss << "S"; else ss << "_";
                                if (WShifts[x][z][y] > 0 ) ss << "W"; else ss << "_";
                                if (EShifts[x][z][y] > 0 ) ss << "E"; else ss << "_";
                                if (UShifts[x][z][y] > 0 ) ss << "U"; else ss << "_";
                                if (DShifts[x][z][y] > 0 ) ss << "D"; else ss << "_";
                                ss << "]" << "\t";
                            }
                            ss << endl;
                        }
                    }
                    ss << "Chunk: " << Id.X << "/" << Id.Y << "/" << Id.Z << " checked.";
                    dev->logger->write(ss.str().c_str());
                    cout << ss.str();
                    flags->FMeshCheckMatrix = false;
                }
            }

        protected:

        //!------------------------------------------------------------------------------------------------------
        //!                     П Р И В А Т Н А Я       С Е К Ц И Я
        //!------------------------------------------------------------------------------------------------------
        private:
            Di::cDevice* dev;
            Di::cFlags* flags;
            Di::cGlobalMatrix* glMatrix;
            Di::cChunkUtilities cUtils;

            //добавить содержимое буфера buf в буффер mb_storage
            void appendBuffer(SMeshBuffer* buf)
            {
                int v_cnt = mb_storage->getVertexCount();

                //запоминаем положение первой вершины кубика и перебрасываем вершины
                v_shifts.push_back(buf->getVertexCount());
                for (int i=0, max=buf->getVertexCount(); i<max; ++i)
                {
                    mb_storage->Vertices.push_back( buf->Vertices[i] );
                    mb_storage->BoundingBox.addInternalPoint(buf->Vertices[i].Pos);
                }

                //запоминаем положение первой поверхности и перебрасываем поверхности
                i_shifts.push_back(buf->getIndexCount());
                for (int i=0, max=buf->getIndexCount(); i<max; ++i)
                {
                    mb_storage->Indices.push_back(buf->Indices[i] + v_cnt);
                }

                mb_storage->setHardwareMappingHint(EHM_STREAM);
                mb_storage->recalculateBoundingBox();
                mb_storage->setDirty();
            }

            //удалить содержимое буфера по смещениям
            void removeBuffer(int idx)
            {
                //удаляем вершины и индексы начиная с указанного смещения
                mb_storage->Vertices.erase( v_shifts[idx], v_quant);
                mb_storage->Indices.erase( i_shifts[idx], i_quant);

                //теперь переиндексируем faces
                for(int i=i_shifts[idx], max=mb_storage->Indices.size(); i<max; i++)
                    mb_storage->Indices[i] -= i_quant;

                //удаляем смещения уже несуществуюго кубика
                v_shifts.erase(idx);
                i_shifts.erase(idx);

                //всем кубикам после удаленного надо обновить смещения
                //их положения вершин и индексов откатились в m_cubes на v_quant, i_quant
                for(int i=idx, max=v_shifts.size(); i<max; i++) v_shifts[i] -= v_quant;
                for(int i=idx, max=i_shifts.size(); i<max; i++) i_shifts[i] -= i_quant;

                mb_storage->recalculateBoundingBox();
                mb_storage->setDirty();
            }

            void recalculateIndices(int index,int sizeToDel)
            {
                //!Пересчет индексов в мешбуфере
                for (u32 i=index; i<mb_storage->getVertexCount(); i++)
                {
                    mb_storage->Indices[i] -= sizeToDel;
                }
                for (int x=0; x<PARAM_CHUNK_SIZE; x++)
                {
                    for (int z=0; z<PARAM_CHUNK_SIZE; z++)
                    {
                        for (int y=0; y<PARAM_CHUNK_HEIGHT; y++)
                        {
                            if (NShifts[x][z][y] > index) NShifts[x][z][y] -= sizeToDel;
                            if (SShifts[x][z][y] > index) SShifts[x][z][y] -= sizeToDel;
                            if (WShifts[x][z][y] > index) WShifts[x][z][y] -= sizeToDel;
                            if (EShifts[x][z][y] > index) EShifts[x][z][y] -= sizeToDel;
                            if (UShifts[x][z][y] > index) UShifts[x][z][y] -= sizeToDel;
                            if (DShifts[x][z][y] > index) DShifts[x][z][y] -= sizeToDel;
                        }
                    }
                }
            }

            IMeshBuffer * cloneMeshBuffer(IMeshBuffer * orig)
            {
                SMeshBuffer * buffer = new SMeshBuffer;

                //клонируем вершины
                S3DVertex* vbuf = (S3DVertex*)orig->getVertices();
                buffer->Vertices.set_used(0);
                for(int i=0, max=orig->getVertexCount(); i<max; i++)
                {
                    buffer->Vertices.push_back(vbuf[i]);
                }

                //клонируем индексы
                u16 * ibuf = (u16*)orig->getIndices();
                buffer->Indices.set_used(0);
                for (int i=0, max=orig->getIndexCount(); i<max; i++)
                {
                    buffer->Indices.push_back(ibuf[i]);
                }
                buffer->setHardwareMappingHint(orig->getHardwareMappingHint_Index(), EBT_INDEX);
                buffer->setHardwareMappingHint(orig->getHardwareMappingHint_Vertex(), EBT_VERTEX);
                return buffer;
            }
    };
}
#endif // CDIMESHBUFFER_H
