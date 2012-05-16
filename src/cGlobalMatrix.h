#ifndef CGLOBALMATRIX_H
#define CGLOBALMATRIX_H

namespace Di
{
    class cGlobalMatrix
    {
        public:
            cGlobalMatrix() {}
            virtual ~cGlobalMatrix() {}

            //!Регистрация матрицы чанка в общем реестре
            void registerChunkMatrix(vector3di chunk , boost::array<boost::array<boost::array<u32,PARAM_CHUNK_HEIGHT>,PARAM_CHUNK_SIZE>,PARAM_CHUNK_SIZE> *m)
            {
                globalMatrix[chunk.X][chunk.Z][chunk.Y].matrix = m;
                globalMatrix[chunk.X][chunk.Z][chunk.Y].isUsed = true;
            }

            //!Получение значения матрицы нужного чанка по координатам блока
            int getMValueFromCoord(vector3di chunk, vector3di block)
            {
                if (!globalMatrix[chunk.X][chunk.Z][chunk.Y].isUsed) return 0;
                return (*globalMatrix[chunk.X][chunk.Z][chunk.Y].matrix)[block.X][block.Z][block.Y];
            }


        private:

            struct SGlobalMatrix
            {
                boost::array<boost::array<boost::array<u32,PARAM_CHUNK_HEIGHT>,PARAM_CHUNK_SIZE>,PARAM_CHUNK_SIZE> *matrix;
                bool isUsed;

                SGlobalMatrix(): isUsed(false){};

            }globalMatrix[PARAM_WORLD_SIZE_X][PARAM_WORLD_SIZE_Z][PARAM_WORLD_SIZE_Y];
    };
}
#endif // CGLOBALMATRIX_H
