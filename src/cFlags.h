#ifndef CFLAGS_H
#define CFLAGS_H

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

        protected:
        private:
    };
};

#endif // CFLAGS_H
