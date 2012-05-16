#ifndef CDEVICE_H
#define CDEVICE_H

#include "defines.h"

#include <irrlicht.h>

#include "cLogger.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

namespace Di
{
    class cDevice
    {
        public:
            cDevice() {}
            virtual ~cDevice() {}

            IrrlichtDevice* device;
            IVideoDriver* driver;
            IGUIEnvironment* genv;
            ISceneManager* smgr;
            ISceneCollisionManager* collMan;
            cLogger* logger;

            int init()
            {
                device = createDevice(EDT_OPENGL, dimension2du(640, 480), 32, false, false, false, 0);
                if (!device)  return ERR_DEVICE;

                driver = device->getVideoDriver();
                genv = device->getGUIEnvironment();
                smgr = device->getSceneManager();
                logger = new Di::cLogger();
                collMan = smgr->getSceneCollisionManager();

                device->getCursorControl()->setVisible(false);

                logger->init(device);

                if (DEBUG_INFO)
                {
                    ostringstream ss; ss << "SYSTEM: DEVICE START\n\n";
                    logger->write(ss.str().c_str());
                    cout << ss.str() << endl;
                }

                return 0;
            }

            void closeDevice()
            {
                if (DEBUG_INFO)
                {
                    ostringstream ss; ss << "SYSTEM: DEVICE CLOSE\n";
                    logger->write(ss.str().c_str());
                    cout << ss.str() << endl;
                }
                logger->drop();
                device->closeDevice();
            }

            int drop()
            {
                if (DEBUG_INFO)
                {
                    ostringstream ss; ss << "SYSTEM: DEVICE DPOP\n";
                    logger->write(ss.str().c_str());
                    cout << ss.str() << endl;
                }
                //logger->drop();
                if (device->drop())
                {
                    if (DEBUG_INFO)
                    {
                        ostringstream ss; ss << " - Device: dropped\n";
                        logger->write(ss.str().c_str());
                        cout << ss.str() << endl;
                    }
                } else {
                    cout << ERR_DEVICE_DROP << endl;
                }
                return 0;
            }

        protected:
        private:
    };
};
#endif // CDEVICE_H
