#ifndef CUTILITIES_H
#define CUTILITIES_H

namespace Di
{
    class cChunkUtilities
    {
        public:
            cChunkUtilities() {}
            virtual ~cChunkUtilities() {}

            //!----------------------------------------------------------------------------------------------
            //! Поиск нужного чанка по заданной координате -- X
            u32 getChunkX(vector3df pos)
            {
                u32 out = pos.X/PARAM_CHUNK_SIZE;
                return out;
            }
            //! Поиск нужного чанка по заданной координате -- Z
            u32 getChunkZ(vector3df pos)
            {
                u32 out = pos.Z/PARAM_CHUNK_SIZE;
                return out;
            }
            //! Поиск нужного чанка по заданной координате -- Y
            u32 getChunkY(vector3df pos, vector3df camPos=vector3df(0))
            {
                u32 out = pos.Y/(PARAM_CHUNK_HEIGHT-PARAM_BLOCK_SIZE);
                if ( 0 != fmod(pos.Y,PARAM_CHUNK_HEIGHT-PARAM_BLOCK_SIZE))
                    return out;
                else
                {
                    if (pos.Y < camPos.Y && out!=0) out--;
                    return out;
                }
            }

            //!-----------------------------------------------------------------------------------------------
            //!Поиск блока в нужном чанке по заданной глобальной координате -- x
            u32 getBlockX(vector3di pos)
            {
                u32 out = pos.X%PARAM_CHUNK_SIZE;
                return out;
            }
            //!Поиск блока в нужном чанке по заданной глобальной координате -- z
            u32 getBlockZ(vector3di pos)
            {
                u32 out = pos.Z%PARAM_CHUNK_SIZE;
                return out;
            }
            //!Поиск блока в нужном чанке по заданной глобальной координате -- y
            u32 getBlockY(vector3di pos)
            {
                u32 out = pos.Y%PARAM_CHUNK_HEIGHT;
                return out;
            }

            //!Трансформация позиции блока из глобальной в матричную
            vector3df transformBlockfromGlobal(vector3df pos)
            {
                while (pos.X > PARAM_CHUNK_SIZE-PARAM_BLOCK_SIZE/2)
                {
                    pos.X -= PARAM_CHUNK_SIZE;
                }
                while (pos.Z > PARAM_CHUNK_SIZE-PARAM_BLOCK_SIZE/2)
                {
                    pos.Z -= PARAM_CHUNK_SIZE;
                }
                while (pos.Y >= PARAM_CHUNK_HEIGHT-PARAM_BLOCK_SIZE/2)
                {
                    pos.Y -= PARAM_CHUNK_HEIGHT;
                }

                if (pos.X < 0) pos.X = 0;
                if (pos.Z < 0) pos.Z = 0;
                if (pos.Y < 0) pos.Y = 0;

                return pos;
            }

        private:
    };
}
#endif // CUTILITIES_H
