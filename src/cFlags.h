#ifndef CFLAGS_H
#define CFLAGS_H

#include <irrlicht.h>

namespace Di
{
    class cFlags
    {
        public:
            cFlags()
            {
                FRecMLBPressed = false; FRecMLBPressedDown = false;
                FRecMRBPressed = false; FRecMRBPressedDown = false;
                FMeshBoundingBoxReload = false;
                FMeshCheckMatrix = false;

                FWireframe = false;
                FHalfTransparent = false;
            }
            virtual ~cFlags() {}

            //Флаги рецейвера
            bool FRecMLBPressed;
            bool FRecMRBPressed;
            bool FRecMLBPressedDown;
            bool FRecMRBPressedDown;

            //флаги материалов
            bool FWireframe;
            bool FHalfTransparent;

            //флаги мешбуферов
            bool FMeshBoundingBoxReload;
            bool FMeshCheckMatrix;

            void pushToPlaneMaker(vector3di chunk, vector3di block, int N,int S, int W, int E, int U, int D)
            {
              SPlaneMaker out;
              out.chunk = chunk;
              out.block = block;
              out.N = N;
              out.S = S;
              out.W = W;
              out.E = E;
              out.U = U;
              out.D = D;
              planeMaker.push_back(out);
            }

            u32 getPlaneMakerSize()
            {
               return planeMaker.size();
            }

            //Вспомогательная структура для связи чанков в построении плоскостей
            struct SPlaneMaker
            {
                vector3di chunk, block;
                int N,S,W,E,U,D;
            };

            array<SPlaneMaker> planeMaker;

        protected:
        private:


    };
};

#endif // CFLAGS_H
