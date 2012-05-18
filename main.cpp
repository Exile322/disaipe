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

    //! Пробная генерация сферы во весь мир
    /*for (u32 x=0; x<PARAM_WORLD_SIZE_X; x++)
        for (u32 z=0; z<PARAM_WORLD_SIZE_Z; z++)
            for (u32 y=0; y<PARAM_WORLD_SIZE_Y; y++)
            {
                mesh[x][z][y] = new Di::cDiMeshBuffer(vector3di(x,y,z), &dev, &glMatrix, &flags);
                u32 R = (PARAM_WORLD_SIZE_X*PARAM_CHUNK_SIZE)/2;
                mesh[x][z][y]->generateSphere(R,1,vector3di(R-x*PARAM_CHUNK_SIZE,R-y*PARAM_CHUNK_HEIGHT,R-z*PARAM_CHUNK_SIZE));
                mesh[x][z][y]->generateSphere(R/2,0,vector3di(R-x*PARAM_CHUNK_SIZE,R*2,R-z*PARAM_CHUNK_SIZE));
            }*/

    //! Пробная генерация ландшафта
    IImage* img = dev.driver->createImageFromFile("textures/terrain-heightmap.bmp");
    for (u32 x=0; x<PARAM_WORLD_SIZE_X; x++)
        for (u32 z=0; z<PARAM_WORLD_SIZE_Z; z++)
            for (u32 y=0; y<PARAM_WORLD_SIZE_Y; y++)
            {
                mesh[x][z][y] = new Di::cDiMeshBuffer(vector3di(x,y,z), &dev, &glMatrix, &flags);
            }
    for (u32 x=0; x<PARAM_WORLD_SIZE_X; x++)
        for (u32 z=0; z<PARAM_WORLD_SIZE_Z; z++)
            mesh[x][z][0]->generateFromHeightMap(img,1.0f,vector2di(x*PARAM_CHUNK_SIZE,z*PARAM_CHUNK_SIZE));

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

    //Подготовка мешей
    SMesh *surfaceMesh[PARAM_SECTIONS][PARAM_SECTIONS];
    for (u32 x=0; x < PARAM_SECTIONS; x++)
        for (u32 z=0; z < PARAM_SECTIONS; z++)
            surfaceMesh[x][z] = new SMesh();

    //Заполнение мешей
    for (u32 x=0; x<PARAM_WORLD_SIZE_X; x++)
        for (u32 z=0; z<PARAM_WORLD_SIZE_Z; z++)
            for (u32 y=0; y<PARAM_WORLD_SIZE_Y; y++)
            {
                mesh[x][z][y]->build();
                surfaceMesh[x/(PARAM_WORLD_SIZE_X/PARAM_SECTIONS)][z/(PARAM_WORLD_SIZE_X/PARAM_SECTIONS)]->addMeshBuffer(mesh[x][z][y]->mb_storage);
            }

    //Подготовка узлов сцены
    IMeshSceneNode* node[PARAM_SECTIONS][PARAM_SECTIONS];
    for (u32 x=0; x < PARAM_SECTIONS; x++)
        for (u32 z=0; z < PARAM_SECTIONS; z++)
        {
            node[x][z]= dev.smgr->addMeshSceneNode(surfaceMesh[x][z]);

            //Высчитываем габаритный бокс ноды
            vector3df minEdge = vector3df(PARAM_WORLD_SIZE_X/PARAM_SECTIONS*x*PARAM_CHUNK_SIZE-PARAM_BLOCK_SIZE,0,PARAM_WORLD_SIZE_Z/PARAM_SECTIONS*z*PARAM_CHUNK_SIZE-PARAM_BLOCK_SIZE);
            core::aabbox3df bbox = core::aabbox3df(minEdge,minEdge + vector3df(PARAM_CHUNK_SIZE*(PARAM_WORLD_SIZE_X/PARAM_SECTIONS)+PARAM_BLOCK_SIZE,PARAM_CHUNK_SIZE*PARAM_WORLD_SIZE_Y,PARAM_CHUNK_SIZE*(PARAM_WORLD_SIZE_X/PARAM_SECTIONS)+PARAM_BLOCK_SIZE));
            //Присваиваем полученный габиритный бокс ноде
            node[x][z]->getMesh()->setBoundingBox(bbox);

            //Добавляем ноду в список проверяемых для occlusion culling
            //http://irrlicht.sourceforge.net/docutemp/example026.html
            dev.driver->addOcclusionQuery(node[x][z],surfaceMesh[x][z]);

            ITriangleSelector* selector = dev.smgr->createTriangleSelector(surfaceMesh[x][z], node[x][z]);
            dev.metaSelector->addTriangleSelector(selector);
            node[x][z]->setTriangleSelector(selector);
            selector->drop();
        }

    // create skybox
	dev.driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);
	dev.smgr->addSkyBoxSceneNode(
		dev.driver->getTexture("textures/irrlicht2_up.jpg"),
		dev.driver->getTexture("textures/irrlicht2_dn.jpg"),
		dev.driver->getTexture("textures/irrlicht2_lf.jpg"),
		dev.driver->getTexture("textures/irrlicht2_rt.jpg"),
		dev.driver->getTexture("textures/irrlicht2_ft.jpg"),
		dev.driver->getTexture("textures/irrlicht2_bk.jpg"));
	dev.driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);

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
    camera->setFarValue(200.0f);
    ISceneNodeAnimator* camAnim = dev.smgr->createCollisionResponseAnimator(dev.metaSelector,camera,vector3df(0.5f,0.5f,0.5f),vector3df(0,0,0));
    camera->addAnimator(camAnim);
    camAnim->drop();

    while(dev.device->run())
    {

        //Обновление боундинг бокса мешбуффера
        if (flags.FMeshBoundingBoxReload)
        {
            bool skip = false; //флаг пропуска операции. TRUE, если координаты не верны
            int x = cUtils.getSection((int)camera->getAbsolutePosition().X);
            int z = cUtils.getSection((int)camera->getAbsolutePosition().Z);

            if (x < 0 || x > PARAM_SECTIONS-1 || z <0 || z > PARAM_SECTIONS-1) skip = true;

            if (!skip)
            {
                //Удаляем предыдущий селектор
                dev.metaSelector->removeTriangleSelector(node[x][z]->getTriangleSelector());
                //Обновление селектора. Потерял около часа, искав причину "необновления" боундинг бокса. Я нуб :(
                ITriangleSelector* selector = dev.smgr->createTriangleSelector(node[x][z]->getMesh(), node[x][z]);
                node[x][z]->setTriangleSelector(selector);
                dev.metaSelector->addTriangleSelector(selector);
                selector->drop();

                //обновление аниматора камеры
                //для этого ищем устаревший аниматор, удаляем его и заменяем новым
                camAnim = dev.smgr->createCollisionResponseAnimator(dev.metaSelector,camera,vector3df(0.4f,0.4f,0.4f),vector3df(0,0,0));
                list<ISceneNodeAnimator*>::ConstIterator it = camera->getAnimators().begin();
                ISceneNodeAnimator* anim;
                for (; it != camera->getAnimators().end(); ++it)
                {

                    if ((*it)->getType() == ESNAT_COLLISION_RESPONSE)
                        anim = *it;
                }
                camera->removeAnimator(anim);
                camera->addAnimator(camAnim);
                camAnim->drop();

                flags.FMeshBoundingBoxReload = !flags.FMeshBoundingBoxReload;
            }
        }

        //Проверяем не нужно ли где сгенерировать недостоющие плоскости
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

        //! ###################################################################################################################
        //! Главный цикл
        dev.driver->beginScene(true, true, SColor(0,100,100,100));

            dev.step();

            //Обработка клавиш мыши. Задержка между нажатиями устанавливается параметром DEVICE_MOUSE_PRESS_DELAY
            if (dev.device->getTimer()->getTime() - receiver.mKeysTimer > DEVICE_MOUSE_PRESS_DELAY)
            {
                if (flags.FRecMLBPressed) mesh[cUtils.getChunkX(dev.selected.intersection)][cUtils.getChunkZ(dev.selected.intersection)][cUtils.getChunkY(dev.selected.intersection,camera->getAbsolutePosition())]->blockDel(mesh[cUtils.getChunkX(dev.selected.intersection)][cUtils.getChunkZ(dev.selected.intersection)][cUtils.getChunkY(dev.selected.intersection,camera->getAbsolutePosition())]->getBlockPos(cUtils.transformBlockfromGlobal(dev.selected.intersection),camera->getAbsolutePosition()));
                if (flags.FRecMRBPressed) mesh[cUtils.getChunkX(dev.selected.intersection)][cUtils.getChunkZ(dev.selected.intersection)][cUtils.getChunkY(dev.selected.intersection,camera->getAbsolutePosition())]->blockAdd(mesh[cUtils.getChunkX(dev.selected.intersection)][cUtils.getChunkZ(dev.selected.intersection)][cUtils.getChunkY(dev.selected.intersection,camera->getAbsolutePosition())]->getBlockPos(cUtils.transformBlockfromGlobal(dev.selected.intersection),camera->getAbsolutePosition()),dev.selected.intersection);

                receiver.dropFlags();
            }

            //Вывод данныъ матрицы выделеного чанка
            if (flags.FMeshCheckMatrix) mesh[cUtils.getChunkX(dev.selected.intersection)][cUtils.getChunkZ(dev.selected.intersection)][cUtils.getChunkY(dev.selected.intersection,camera->getAbsolutePosition())]->checkBuffer();
            //Меняем параметры материала по нажатию кнопочек
            if (dev.selected.isSelected)
            {
                bool skip = false; //флаг пропуска операции. TRUE, если координаты не верны
                int x = cUtils.getSection((int)camera->getAbsolutePosition().X);
                int z = cUtils.getSection((int)camera->getAbsolutePosition().Z);
                if (x < 0 || x > PARAM_SECTIONS-1 || z <0 || z > PARAM_SECTIONS-1) skip = true;
                if (!skip)
                {
                    node[x][z]->setMaterialFlag(EMF_WIREFRAME,flags.FWireframe);
                    if (flags.FHalfTransparent)
                    {
                        node[x][z]->setDebugDataVisible(EDS_HALF_TRANSPARENCY);
                    } else {
                        node[x][z]->setDebugDataVisible(EDS_OFF);
                    }
                    if (flags.FBoundingBox)
                    {
                        node[x][z]->setDebugDataVisible(EDS_BBOX_ALL);
                    } else {
                        node[x][z]->setDebugDataVisible(EDS_OFF);
                    }
                }
            }

            if (dev.device->getTimer()->getTime()-dev.timeNow>100)
            {
                dev.driver->runAllOcclusionQueries(false);
                dev.driver->updateAllOcclusionQueries();
                for (u32 x=0; x < PARAM_SECTIONS; x++)
                    for (u32 z=0; z < PARAM_SECTIONS; z++)
                        node[x][z]->setVisible(dev.driver->getOcclusionQueryResult(node[x][z])>0);
                dev.timeNow=dev.device->getTimer()->getTime();
            }

            stringw str;
            str += "ver."; str += AutoVersion::MAJOR; str += "."; str += AutoVersion::MINOR; str += "."; str += AutoVersion::BUILDS_COUNT; str += " | FPS: ";
            str += dev.driver->getFPS();
            str += " | Prim-es: "; str += dev.driver->getPrimitiveCountDrawn();
            str += " | Section: "; str += cUtils.getSection(camera->getAbsolutePosition().X); str += "/"; str += cUtils.getSection(camera->getAbsolutePosition().Z);
            str += " | Chunk: "; str += cUtils.getChunkX(dev.selected.intersection); str += "/"; str += cUtils.getChunkZ(dev.selected.intersection); str+= "/"; str+= cUtils.getChunkY(dev.selected.intersection,camera->getAbsolutePosition());

            //Перемещение прицела на место курсора
            if(dev.selected.isSelected)
            {
                bill->setPosition(dev.selected.intersection);
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

