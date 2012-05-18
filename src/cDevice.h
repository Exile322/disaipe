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

            u32 timeNow;

            IMetaTriangleSelector* metaSelector;

            struct SSelected
            {
                bool isSelected;
                ISceneNode* node;
                // переменная под хранение точки пересещения
                core::vector3df intersection;
                // переменная под хранение треугольника с которым пересекся луч
                core::triangle3df hitTriangle;
            }selected;

            int init()
            {
                device = createDevice(EDT_OPENGL, dimension2du(1024, 768), 32, false, false, false, 0);
                if (!device)  return ERR_DEVICE;

                driver = device->getVideoDriver();
                genv = device->getGUIEnvironment();
                smgr = device->getSceneManager();
                logger = new Di::cLogger();
                collMan = smgr->getSceneCollisionManager();

                device->getCursorControl()->setVisible(false);

                logger->init(device);

                metaSelector = smgr->createMetaTriangleSelector();

                timeNow = device->getTimer()->getTime();

                if (DEBUG_INFO)
                {
                    ostringstream ss; ss << "SYSTEM: DEVICE START\n\n";
                    logger->write(ss.str().c_str());
                    cout << ss.str() << endl;
                }

                return 0;
            }

            void step()
            {
                smgr->drawAll();
                getSelected(smgr->getActiveCamera());
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

            void getSelected(ICameraSceneNode* camera)
            {
                core::line3d<f32> ray;
                ray.start = camera->getAbsolutePosition();
                ray.end = ray.start + (camera->getTarget() - ray.start).normalize() * 1000.0f;

                selected.node = collMan->getSceneNodeAndCollisionPointFromRay(
                                    ray,
                                    selected.intersection, // точка столкновения
                                    selected.hitTriangle, // полигон(треугольник) в котором точка столкновения
                                    0, // определять столкновения только для нод с идентификатором IDFlag_IsPickable
                                    0); // проверять относительно всей сцены (оставляем значение по умолчанию)
                if (selected.node)
                    selected.isSelected = true;
                else
                    selected.isSelected = false;
            }
        protected:
        private:
    };
};
#endif // CDEVICE_H
