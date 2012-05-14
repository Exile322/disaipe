#ifndef CSURFACEGENERATOR_H
#define CSURFACEGENERATOR_H

#include <stdlib.h>
#include <math.h>

#include <irrlicht.h>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class cSurfaceGenerator
{
    public:
        cSurfaceGenerator() {}
        virtual ~cSurfaceGenerator() {}

        //!Генерация куска поверхности размером PARAM_CHUNK_SIZE
        array<u32> generateChunk()
        {
            array<u32> chunk;
            for (int i=0;i<PARAM_CHUNK_SIZE;i++)
            {
                float K = 1;    //начальный коэффициент равен 100%
                if (i>0 && chunk[i-1]==1)
                {
                    K = K - (rand()%100+rand()%30+30)/100;
                }

                chunk.push_back(rand()%1 + int(K));
            }
            return chunk;
        }

        array<u32> smoothChunk(array<u32> chunk)
        {
            for (int i=0;i<PARAM_CHUNK_SIZE;i++)
            {
                if (chunk[i]>0 && i > 0 && i < sqrt(PARAM_CHUNK_SIZE))
                    chunk[i]++;
            }
            return chunk;
        }
    protected:
    private:
};

#endif // CSURFACEGENERATOR_H
