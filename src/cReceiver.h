#ifndef CRECEIVER_H
#define CRECEIVER_H

#include "cDevice.h"
#include "cFlags.h"

#include <irrlicht.h>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

namespace Di
{
    class cReceiver : public IEventReceiver
    {
        public:
            cReceiver() {}
            ~cReceiver() {}

            bool isKeyDown[KEY_KEY_CODES_COUNT];
            bool isKeyWasDown[KEY_KEY_CODES_COUNT];

            u32 mKeysTimer;

            void setup(cDevice* d, cFlags* f)
            {
                dev = d;
                flags = f;

                mKeysTimer = 0;

                memset(isKeyWasDown,false,sizeof(isKeyWasDown));
                memset(isKeyDown,false,sizeof(isKeyDown));
            }

            //! Обработка событий
            bool OnEvent(const SEvent &event)
            {
                switch (event.EventType)
                {
                    //! Клавишные события ###########################################################
                    case EET_KEY_INPUT_EVENT:
                        switch (event.KeyInput.Key)
                        {
                            case KEY_ESCAPE:
                                dev->closeDevice();
                                break;
                            default:
                                if (event.KeyInput.PressedDown)
                                {
                                    isKeyDown[event.KeyInput.Key] = true;
                                    isKeyWasDown[event.KeyInput.Key] = false;
                                } else if (isKeyDown[event.KeyInput.Key] == true && !event.KeyInput.PressedDown) {
                                    isKeyWasDown[event.KeyInput.Key] = true;
                                    isKeyDown[event.KeyInput.Key] = false;
                                }
                                break;
                        }
                        chechKeys();
                        break;
                    //! Мышиные события #############################################################
                    case EET_MOUSE_INPUT_EVENT:
                        if (event.MouseInput.isLeftPressed())
                        {
                            flags->FRecMLBPressedDown = true;
                            flags->FRecMLBPressed = false;
                        } else if (flags->FRecMLBPressedDown == true && !event.MouseInput.isLeftPressed()) {
                            flags->FRecMLBPressed = true;
                            flags->FRecMLBPressedDown = false;
                        }
                        if (event.MouseInput.isRightPressed())
                        {
                            flags->FRecMRBPressedDown = true;
                            flags->FRecMRBPressed = false;
                        } else if (flags->FRecMRBPressedDown == true && !event.MouseInput.isRightPressed()) {
                            flags->FRecMRBPressed = true;
                            flags->FRecMRBPressedDown = false;
                        }
                        break;
                    case EET_LOG_TEXT_EVENT:
                        dev->logger->write(event.LogEvent.Text);
                        break;
                    default:
                        break;
                }

                return false;
            }

            void dropFlags()
            {
                flags->FRecMLBPressed = false;
                flags->FRecMRBPressed = false;

                mKeysTimer = dev->device->getTimer()->getTime();
            }

            void chechKeys()
            {
                if (isKeyWasDown[KEYS_WIREWRAME]) flags->FWireframe = !flags->FWireframe;
                if (isKeyWasDown[KEYS_HALF_TRANSPARENT]) flags->FHalfTransparent = !flags->FHalfTransparent;
                if (isKeyWasDown[KEYS_BOUNDING_BOX]) flags->FBoundingBox = !flags->FBoundingBox;
                if (isKeyWasDown[KEY_KEY_C]) flags->FMeshCheckMatrix = true;

                memset(isKeyWasDown,false,sizeof(isKeyWasDown));
            }

        protected:
        private:
            cDevice* dev;
            cFlags* flags;
    };
};

#endif // CRECEIVER_H
