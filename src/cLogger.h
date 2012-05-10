#ifndef CLOGGER_H
#define CLOGGER_H

#include <irrlicht.h>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

namespace Di
{
    class cLogger
    {
        public:
            cLogger() {}

            virtual ~cLogger()
            {
                drop();
            }

            void init (IrrlichtDevice* d)
            {
                device = d;
                logger = device->getLogger();

                fout= fopen("log.txt","w");
            }

            void drop()
            {
                fclose(fout);
            }

            void write(stringc str)
            {
                fputs(str.c_str(),fout);
            }

            void write(const c8* in)
            {
                fputs(in,fout);
            }

        protected:
        private:
            IrrlichtDevice* device;
            ILogger* logger;

            FILE *fout;
    };
};

#endif // CLOGGER_H
