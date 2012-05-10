#include "src/defines.h"
#include "src/cDevice.h"
#include "src/cDiMeshBuffer.h"
#include "src/cSurfaceGenerator.h"
#include "src/cReceiver.h"
#include "src/cFlags.h"
#include "version.h"

#include <irrlicht.h>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

int main(int argc, char** argv)
{
    Di::cDevice dev;
    dev.init();

    Di::cFlags flags;

    Di::cReceiver receiver;
    receiver.setup(&dev, &flags);

    dev.device->setEventReceiver(&receiver);

    cDiMeshBuffer mesh(&dev, &flags);
    //mesh.addOriginalMesh(0,"models/plane.3ds");
    //mesh.addOriginalMesh(1,"models/edge1.3ds");
    //mesh.addOriginalMesh(2,"models/edge2.3ds");

    //cSurfaceGenerator generator;

    /*array<u32> block = generator.smoothChunk(generator.generateChunk());
    int b = 0;
    for (int x=0; x<16;x++)
        for (int z=0; z<16; z++)
        {
            mesh.dublicateTo(block[b],x,0,z);
            b++;
        }
    */

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
                        S = 0;
                        break;
                    case PARAM_CHUNK_SIZE-1:
                        N = 0;
                        break;
                    default:
                        break;
                }
                switch (y)
                {
                    case 0:
                        W = 0;
                        break;
                    case PARAM_CHUNK_SIZE-1:
                        E = 0;
                        break;
                    default:
                        break;
                }
                switch (z)
                {
                    case 0:
                        D = 0;
                        break;
                    case PARAM_CHUNK_HEIGHT-1:
                        U = 0;
                        break;
                    default:
                        break;
                }
                if (x!=PARAM_CHUNK_SIZE-1) N = mesh.matrix[x+1][y][z];
                if (x!=0) S = mesh.matrix[x-1][y][z];
                if (y!=PARAM_CHUNK_SIZE-1) E = mesh.matrix[x][y+1][z];
                if (y!=0) W = mesh.matrix[x][y-1][z];
                if (z!=PARAM_CHUNK_HEIGHT-1) U = mesh.matrix[x][y][z+1];
                if (z!=0) D = mesh.matrix[x][y][z-1];
                if (mesh.matrix[x][y][z]==1) mesh.dublicateTo(mesh.createPlaneMesh(vector3di(x,z,y),N,S,W,E,U,D),x,z,y);
            }
        }
    }

    ITriangleSelector* selector = 0;

    SMesh *mesh2 = new SMesh();
    mesh2->addMeshBuffer(mesh.mb_storage);
    IMeshSceneNode* node = dev.smgr->addMeshSceneNode(mesh2);
    node->getMaterial(0).setTexture(0,dev.driver->getTexture("textures/plane.jpg"));
    node->setMaterialFlag(EMF_LIGHTING,false);
    node->setMaterialFlag(EMF_WIREFRAME,true);

    selector = dev.smgr->createTriangleSelector(mesh2, node);
    node->setTriangleSelector(selector);
    selector->drop();

    //Billboard
    scene::IBillboardSceneNode * bill = dev.smgr->addBillboardSceneNode();
    bill->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR );
    bill->setMaterialTexture(0, dev.driver->getTexture("textures/particle.bmp"));
    bill->setMaterialFlag(video::EMF_LIGHTING, false);
    bill->setMaterialFlag(video::EMF_ZBUFFER, false);
    bill->setSize(core::dimension2d< f32 >(1.0f, 1.0f));

    ICameraSceneNode* camera = dev.smgr->addCameraSceneNodeFPS(0,20,0.02f);
    camera->setPosition(vector3df(1,18,0));
    camera->setTarget(vector3df(0,0,0));
    camera->setNearValue(0.1f);
    ISceneNodeAnimator* camAnim = dev.smgr->createCollisionResponseAnimator(selector,camera,vector3df(0.5f,0.5f,0.5f),vector3df(0,0,0));
    camera->addAnimator(camAnim);
    camAnim->drop();

    while(dev.device->run())
    {

        //Обновление боундинг бокса мешбуффера
        if (flags.FMeshBoundingBoxReload)
        {
            node->getMesh()->getMeshBuffer(0)->recalculateBoundingBox();
            node->getMesh()->setBoundingBox(node->getMesh()->getMeshBuffer(0)->getBoundingBox());

            //Обновление селектора. Потерял около часа, искав причину "необновления" боундинг бокса. Я нуб :(
            selector = dev.smgr->createTriangleSelector(node->getMesh(), node);
            node->setTriangleSelector(selector);
            selector->drop();

            //обновление аниматора камеры
            //для этого ищем устаревший аниматор, удаляем его и заменяем новым
            camAnim = dev.smgr->createCollisionResponseAnimator(selector,camera,vector3df(0.4f,0.4f,0.4f),vector3df(0,0,0));
            list<ISceneNodeAnimator*>::ConstIterator it = camera->getAnimators().begin();
            for (; it != camera->getAnimators().end(); ++it)
            {
                if ((*it)->getType() == ESNAT_COLLISION_RESPONSE)
                    camera->removeAnimator(*it);
            }
            camera->addAnimator(camAnim);
            camAnim->drop();

            flags.FMeshBoundingBoxReload = !flags.FMeshBoundingBoxReload;
        }

        //Меняем параметры материала по нажатию кнопочек
        node->setMaterialFlag(EMF_WIREFRAME,flags.FWireframe);
        if (flags.FHalfTransparent)
        {
            node->setDebugDataVisible(EDS_HALF_TRANSPARENCY);
        } else {
            node->setDebugDataVisible(EDS_OFF);
        }

        dev.driver->beginScene(true, true, SColor(0,100,100,100));

            dev.smgr->drawAll();

            core::line3d<f32> ray;
            ray.start = camera->getAbsolutePosition();
            ray.end = ray.start + (camera->getTarget() - ray.start).normalize() * 1000.0f;

            // переменная под хранение точки пересещения
            core::vector3df intersection;
            // переменная под хранение треугольника с которым пересекся луч
            core::triangle3df hitTriangle;

            scene::ISceneNode * selectedSceneNode =
                    dev.collMan->getSceneNodeAndCollisionPointFromRay(
                                    ray,
                                    intersection, // точка столкновения
                                    hitTriangle, // полигон(треугольник) в котором точка столкновения
                                    0, // определять столкновения только для нод с идентификатором IDFlag_IsPickable
                                    0); // проверять относительно всей сцены (оставляем значение по умолчанию)

            //Обработка клавиш мыши. Задержка между нажатиями устанавливается параметром DEVICE_MOUSE_PRESS_DELAY
            if (dev.device->getTimer()->getTime() - receiver.mKeysTimer > DEVICE_MOUSE_PRESS_DELAY)
            {
                if (flags.FRecMLBPressed) mesh.blockDel(mesh.getBlockPos(intersection,camera->getAbsolutePosition()));
                if (flags.FRecMRBPressed) mesh.blockAdd(mesh.getBlockPos(intersection,camera->getAbsolutePosition()),intersection);

                receiver.dropFlags();
            }

            if (flags.FMeshCheckMatrix) mesh.checkBuffer();

            stringw str;
            str += "ver."; str += AutoVersion::MAJOR; str += "."; str += AutoVersion::MINOR; str += "."; str += AutoVersion::BUILDS_COUNT; str += " | FPS: ";
            str += dev.driver->getFPS();

            if(selectedSceneNode)
            {
                bill->setPosition(intersection);
                bill->setVisible(true);
            } else {
                bill->setVisible(false);
            }

            dev.device->setWindowCaption(str.c_str());

        dev.driver->endScene();
    }

    dev.drop();

    return 0;
}

