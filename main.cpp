#include "src/defines.h"
#include "src/cDevice.h"
#include "src/cDiMeshBuffer.h"
#include "src/cSurfaceGenerator.h"
#include "src/cReceiver.h"
#include "src/cFlags.h"
#include "src/cUtilities.h"
#include "src/cGlobalMatrix.h"
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
    Di::cChunkUtilities cUtils;
    Di::cGlobalMatrix glMatrix;

    Di::cReceiver receiver;
    receiver.setup(&dev, &flags);

    dev.device->setEventReceiver(&receiver);

    Di::cDiMeshBuffer *mesh[PARAM_WORLD_SIZE_X][PARAM_WORLD_SIZE_Z][PARAM_WORLD_SIZE_Y];

    /*for (u32 x=0; x<PARAM_WORLD_SIZE_X; x++)
        for (u32 z=0; z<PARAM_WORLD_SIZE_Z; z++)
            for (u32 y=0; y<PARAM_WORLD_SIZE_Y; y++)
            {
                mesh[x][z][y] = new Di::cDiMeshBuffer(vector3di(x,y,z), &dev, &glMatrix, &flags);
                mesh[x][z][y]->generateSphere(8,1,vector3di(7,7,7));
            }*/
    //! Пробная генерация сферы во весь мир
    for (u32 x=0; x<PARAM_WORLD_SIZE_X; x++)
        for (u32 z=0; z<PARAM_WORLD_SIZE_Z; z++)
            for (u32 y=0; y<PARAM_WORLD_SIZE_Y; y++)
            {
                mesh[x][z][y] = new Di::cDiMeshBuffer(vector3di(x,y,z), &dev, &glMatrix, &flags);
                u32 R = PARAM_WORLD_SIZE_X*PARAM_CHUNK_SIZE/2;
                mesh[x][z][y]->generateSphere(R,1,vector3di(R-x*PARAM_CHUNK_SIZE,R-y*PARAM_CHUNK_HEIGHT,R-z*PARAM_CHUNK_SIZE));
            }


    //mesh[0][0][0]->generateSphere(8,1,vector3di(7,7,7));
    //mesh[0][0][0]->generateSphere(5,0,vector3di(7,7,7));
    //mesh[0][0][1]->generateEllipse(8,8,8,vector3di(8,8,8));

    /*for (u32 x=0; x<PARAM_WORLD_SIZE_X; x++)
        for (u32 z=0; z<PARAM_WORLD_SIZE_Z; z++)
            for (u32 y=0; y<PARAM_WORLD_SIZE_Y; y++)
            {
                mesh[x][z][y]->build();
            }*/

    //mesh[0][0][0]->addOriginalMesh(0,"models/plane.3ds");
    //mesh[0][0][0]->addOriginalMesh(1,"models/edge1.3ds");
    //mesh[0][0][0]->addOriginalMesh(2,"models/edge2.3ds");

    //cSurfaceGenerator generator;

    /*array<u32> block = generator.smoothChunk(generator.generateChunk());
    int b = 0;
    for (int x=0; x<16;x++)
        for (int z=0; z<16; z++)
        {
            mesh[0][0][0]->dublicateTo(block[b],x,0,z);
            b++;
        }
    */

    ITriangleSelector* selector = 0;

    SMesh *surfaceMesh = new SMesh();
    for (u32 x=0; x<PARAM_WORLD_SIZE_X; x++)
        for (u32 z=0; z<PARAM_WORLD_SIZE_Z; z++)
            for (u32 y=0; y<PARAM_WORLD_SIZE_Y; y++)
            {
                mesh[x][z][y]->build();
                surfaceMesh->addMeshBuffer(mesh[x][z][y]->mb_storage);
            }

    IMeshSceneNode* node = dev.smgr->addMeshSceneNode(surfaceMesh);

    selector = dev.smgr->createTriangleSelector(surfaceMesh, node);
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
            for (u32 i=0; i < node->getMesh()->getMeshBufferCount(); i++)
            {
                node->getMesh()->getMeshBuffer(i)->recalculateBoundingBox();
            }

            core::aabbox3df bbox;
            for (u32 i=0; i < node->getMesh()->getMeshBufferCount(); i++)
            {
                bbox.addInternalBox(node->getMesh()->getMeshBuffer(i)->getBoundingBox());
            }

            node->getMesh()->setBoundingBox(bbox);

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

        if (flags.getPlaneMakerSize() > 0)
        {
            for (u32 i=0; i<flags.getPlaneMakerSize();i++)
            {
                Di::cFlags::SPlaneMaker p;
                p = flags.planeMaker[i];
                mesh[p.chunk.X][p.chunk.Z][p.chunk.Y]->dublicateTo(mesh[p.chunk.X][p.chunk.Z][p.chunk.Y]->createPlaneMesh(p.block,p.N,p.S,p.W,p.E,p.U,p.D),p.block.X,p.block.Y,p.block.Z);
                flags.planeMaker.erase(i);
            }
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
                if (flags.FRecMLBPressed) mesh[cUtils.getChunkX(intersection)][cUtils.getChunkZ(intersection)][cUtils.getChunkY(intersection,camera->getAbsolutePosition())]->blockDel(mesh[cUtils.getChunkX(intersection)][cUtils.getChunkZ(intersection)][cUtils.getChunkY(intersection,camera->getAbsolutePosition())]->getBlockPos(cUtils.transformBlockfromGlobal(intersection),camera->getAbsolutePosition()));
                if (flags.FRecMRBPressed) mesh[cUtils.getChunkX(intersection)][cUtils.getChunkZ(intersection)][cUtils.getChunkY(intersection,camera->getAbsolutePosition())]->blockAdd(mesh[cUtils.getChunkX(intersection)][cUtils.getChunkZ(intersection)][cUtils.getChunkY(intersection,camera->getAbsolutePosition())]->getBlockPos(cUtils.transformBlockfromGlobal(intersection),camera->getAbsolutePosition()),intersection);

                receiver.dropFlags();
            }

            if (flags.FMeshCheckMatrix) mesh[cUtils.getChunkX(intersection)][cUtils.getChunkZ(intersection)][cUtils.getChunkY(intersection,camera->getAbsolutePosition())]->checkBuffer();

            stringw str;
            str += "ver."; str += AutoVersion::MAJOR; str += "."; str += AutoVersion::MINOR; str += "."; str += AutoVersion::BUILDS_COUNT; str += " | FPS: ";
            str += dev.driver->getFPS();
            str += " | Prim-es: "; str += dev.driver->getPrimitiveCountDrawn();
            str += " | Chunk: "; str += cUtils.getChunkX(intersection); str += "/"; str += cUtils.getChunkZ(intersection); str+= "/"; str+= cUtils.getChunkY(intersection,camera->getAbsolutePosition());

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

