#include <irrlicht.h> 
#include <cmath> 
#include "water.h"
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#ifdef _IRR_WINDOWS_
#pragma comment(lib, "Irrlicht.lib")
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

class MyEventReceiver : public IEventReceiver
{
public:
	virtual bool OnEvent(const SEvent& event)
	{
		if (event.EventType == irr::EET_KEY_INPUT_EVENT) { 
			KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown; 
		} 
		return false;
	}

	virtual bool IsKeyDown(EKEY_CODE keyCode) const {
		return KeyIsDown[keyCode];
	}

	MyEventReceiver()
	{
		for (u32 i = 0; i < KEY_KEY_CODES_COUNT; ++i) {
			KeyIsDown[i] = false;
		}
	}

private:
	bool KeyIsDown[KEY_KEY_CODES_COUNT]; 
};

MyEventReceiver receiver;
ISceneManager* smgr;
IVideoDriver* driver;
list<scene::ISceneNode*> bullets;

vector3df shipWaving(vector3df vector, bool &x, bool &z, int xDeg = 2, int zDeg = 3, float xVal = 0.02, float zVal = 0.03){

	if (vector.X >= xDeg) {
		x = false; 
	} 

	if (vector.X <= -xDeg) {
		x = true; 
	}

	if (x) {
		vector.X += xVal;
	}

	else { 
		vector.X -= xVal;
	} 

	if (vector.Z >= zDeg) {
		z = false; 
	} 

	if (vector.Z <= -zDeg) {
		z = true; 
	} 

	if (z) { 
		vector.Z += zVal;
	}
	else { 
		vector.Z -= zVal;
	} 
	return vector;
}

vector3df shipSink(vector3df vector){
	if (vector.X >= -85){
		vector.X -= 0.03;
	}

	if (vector.Z >= -180){
		vector.Z -= 0.02;
	}
	
	return vector;
}

vector3df normalizeBall(scene::ISceneNode* ball)
{
	matrix4 mat = ball->getRelativeTransformation();
	vector3df ballRT(mat[8], mat[9], mat[10]);
	ballRT.normalize();
	return ballRT;
}

void shoot(IAnimatedMeshSceneNode *node, int side)
{
	scene::ISceneNode* bullet = smgr->addSphereSceneNode(10.f, 16, 0, -1, node->getPosition() + core::vector3df(0, 10, 0), node->getRotation());
	
	bullet->setRotation(core::vector3df(bullet->getRotation().X, bullet->getRotation().Y - 90, bullet->getRotation().Z));
	bullet->setRotation(core::vector3df(bullet->getRotation().X, bullet->getRotation().Y + 90, bullet->getRotation().Z));
	
	bullet->setMaterialFlag(video::EMF_LIGHTING, false);
	bullet->setMaterialTexture(0, driver->getTexture("metal.jpg"));
	bullets.push_back(bullet);

}

int main()
{
	

	IrrlichtDevice *device = createDevice(video::EDT_DIRECT3D9, dimension2d<u32>(1366, 768), 16, false, false, true, &receiver); if (!device) return 1;

	driver = device->getVideoDriver(); 
	smgr = device->getSceneManager();
	IGUIEnvironment* guienv = device->getGUIEnvironment();

	scene::ITriangleSelector* selector = 0;
	

	IAnimatedMesh* ship_mesh = smgr->getMesh("Pirate Ship.x"); 
	IAnimatedMeshSceneNode* ship_node = smgr->addAnimatedMeshSceneNode(ship_mesh);
	ship_node->setMaterialFlag(EMF_LIGHTING, false); 
	ship_node->setScale(vector3df(50, 30, 30)); 
	float speed = 0;
	vector3df playerVector;
	bool ship_x_wave = true;
	bool ship_y_wave = true;
	bool ship_z_wave = true;

	IAnimatedMesh* npc_ship_mesh = smgr->getMesh("ship_finished.x"); 
	IAnimatedMeshSceneNode* npc_ship_node = smgr->addAnimatedMeshSceneNode(npc_ship_mesh);
	npc_ship_node->setMaterialFlag(EMF_LIGHTING, false); 
	npc_ship_node->setScale(vector3df(50, 30, 30)); 
	bool npc_ship_x_wave = true;
	bool npc_ship_y_wave = true;
	bool npc_ship_z_wave = true;

	
	scene::ITerrainSceneNode* terrain = smgr->addTerrainSceneNode(
		"terrain-heightmap.bmp",
		0,					// parent node
		-1,					// node id
		core::vector3df(0, -2000, 0),		// position
		core::vector3df(0.f, 0.f, 0.f),		// rotation
		core::vector3df(122, 15, 122),	// scale
		video::SColor(255, 255, 255, 255),	// vertexColor
		5,					// maxLOD
		scene::ETPS_17,				// patchSize
		3					// smoothFactor
		);

	terrain->setMaterialFlag(video::EMF_LIGHTING, false);

	terrain->setMaterialTexture(0,
		driver->getTexture("terrain-texture.jpg"));
	terrain->setMaterialTexture(1,
		driver->getTexture("detailmap3.jpg"));

	terrain->setMaterialType(video::EMT_DETAIL_MAP);

	terrain->scaleTexture(1.0f, 20.0f);	

	selector = smgr->createTerrainTriangleSelector(terrain);
	terrain->setTriangleSelector(selector);
	if (selector)
	{
		ISceneNodeAnimator* animBoat = smgr->createCollisionResponseAnimator(
			selector, ship_node, 
			core::vector3df(120, 10, 120),
			core::vector3df(0, -40, 0),
			core::vector3df(0, 0, 0));
		selector->drop();
		ship_node->addAnimator(animBoat);
		animBoat->drop();
	}

	scene::ITerrainSceneNode* terrain2 = smgr->addTerrainSceneNode(
		"terrain-heightmap2.bmp",
		0,					// parent node
		10,					// node id
		core::vector3df(0, -10, 0),		// position
		core::vector3df(0.f, 0.f, 0.f),		// rotation
		core::vector3df(122, 15, 122),	// scale
		video::SColor(255, 255, 255, 255),	// vertexColor
		5,					// maxLOD
		scene::ETPS_17,				// patchSize
		3,					// smoothFactor
		true
		);


	f32 waterSize = 30000.0f;
	CustomWaterSceneNode*  water = new CustomWaterSceneNode(smgr, waterSize, waterSize, "../irrlichTheGame/");
	water->setPosition(core::vector3df(waterSize / 2, 0, waterSize/2));
	smgr->getRootSceneNode()->addChild(water);

	selector = smgr->createTerrainTriangleSelector(terrain2);
	terrain2->setTriangleSelector(selector);
	ship_node->setPosition(vector3df(waterSize / 2, 0, waterSize / 2));
	npc_ship_node->setPosition(vector3df((waterSize / 2)+200, 0, waterSize / 2));
	if (selector)
	{
		ISceneNodeAnimator* animBoat = smgr->createCollisionResponseAnimator(
			selector, ship_node, core::vector3df(60, 10, 60),
			core::vector3df(0, -100, 0),
			core::vector3df(0, 0, 0));
		ship_node->addAnimator(animBoat);
		animBoat->drop();

		scene::ISceneNodeAnimator* anim = smgr->createCollisionResponseAnimator(
			selector, npc_ship_node,
			core::vector3df(60, 10, 60),
			core::vector3df(0, -100, 0),
			core::vector3df(0, 0, 0));
		selector->drop();
		npc_ship_node->addAnimator(anim);
		anim->drop(); 
	}	

	ISceneNode* SkyBox = 0; 
	SkyBox = smgr->addSkyBoxSceneNode(
		driver->getTexture("skyup.bmp"), 
		driver->getTexture("skydn.bmp"),
		driver->getTexture("sky0.bmp"), 
		driver->getTexture("sky1.bmp"), 
		driver->getTexture("sky2.bmp"), 
		driver->getTexture("sky3.bmp"));

	//scene::ISceneNode* skydome = smgr->addSkyDomeSceneNode(driver->getTexture("0611-800.jpg"), 32, 16,1,2);

	
	scene::ICameraSceneNode* camera =
		smgr->addCameraSceneNodeFPS(0, 100.0f, 5.f);
	camera->setFarValue(30000.0f);

	device->getCursorControl()->setVisible(false); 
	smgr->getActiveCamera()->setPosition(vector3df(waterSize / 2, 75, ((waterSize / 2)-300)));
	
	/*ILightSceneNode* sun = smgr->addLightSceneNode(0, vector3df(0, 0, 0), video::SColorf(0.2f, 0.2f, 0.2f, 0.0f), 1000000000.0f);

	scene::ISceneNodeAnimator* anim = smgr->createFlyCircleAnimator(vector3df(5120, -10, 5120), 5500, 0.004f, vector3df(0.0f, 0.0f, 1.0f), 0, 0.0f);
	sun->addAnimator(anim);
	anim->drop();

	scene::IBillboardSceneNode* bill = smgr->addBillboardSceneNode(sun, dimension2d<f32>(1500, 1500));
	bill->setMaterialFlag(EMF_LIGHTING, false);
	bill->setMaterialFlag(EMF_ZWRITE_ENABLE, false);
	bill->setMaterialType(EMT_TRANSPARENT_ADD_COLOR);
	bill->setMaterialTexture(0, driver->getTexture("particlewhite.bmp"));*/
	
	SkyBox = smgr->addLightSceneNode(0, core::vector3df(0, 0, 0),
		video::SColorf(1.0f, 0.6f, 0.7f, 1.0f), 800.0f);
	scene::ISceneNodeAnimator* anim = 0;
	anim = smgr->createFlyCircleAnimator(core::vector3df(0, 150, 0), 250.0f);
	SkyBox->addAnimator(anim);
	anim->drop();

	// attach billboard to light

	SkyBox = smgr->addBillboardSceneNode(SkyBox, core::dimension2d<f32>(50, 50));
	SkyBox->setMaterialFlag(video::EMF_LIGHTING, false);
	SkyBox->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
	SkyBox->setMaterialTexture(0, driver->getTexture("particlewhite.bmp"));

	int lastFPS = -1;
	while (device->run())
	{	

		if (receiver.IsKeyDown(KEY_ESCAPE)) { device->closeDevice(); } 
		if (receiver.IsKeyDown(KEY_KEY_M)) { device->minimizeWindow(); }
		if (receiver.IsKeyDown(KEY_KEY_W)) { 
			if (speed > 3) { 
				speed = 3; 
			} else { 
				speed += 0.05f; 
			} 
		} 

		if (speed > 0) 
		{
			playerVector = ship_node->getPosition();  
			playerVector.Z += speed*cos((ship_node->getRotation().Y)*(3.14 / 180)); 
			playerVector.X += speed*sin((ship_node->getRotation().Y)*(3.14/180)); 
			ship_node->setPosition(playerVector);
			playerVector = ship_node->getPosition(); playerVector.Y = 125; 
			playerVector.Z -= 300 * (cos(ship_node->getRotation().Y*0.017453)); 
			playerVector.X -= 300 * (sin(ship_node->getRotation().Y*0.017453));
			smgr->getActiveCamera()->setPosition(playerVector); 
			speed -= 0.02f;
		}

		if (receiver.IsKeyDown(KEY_KEY_A)) 
		{
			playerVector = ship_node->getRotation(); 
			playerVector.Y -= 1; 
			ship_node->setRotation(playerVector);

			playerVector = ship_node->getPosition(); playerVector.Y = 125; 
			playerVector.Z -= 300 * (cos(ship_node->getRotation().Y*0.017453)); 
			playerVector.X -= 300 * (sin(ship_node->getRotation().Y*0.017453));
			smgr->getActiveCamera()->setPosition(playerVector); 
	
		}

		if (receiver.IsKeyDown(KEY_KEY_D)) 
		{
			playerVector = ship_node->getRotation(); 
			playerVector.Y += 1; 
			ship_node->setRotation(playerVector);  
			playerVector = ship_node->getPosition(); playerVector.Y = 125; 
			playerVector.Z -= 300 * (cos(ship_node->getRotation().Y*0.017453)); 
			playerVector.X -= 300 * (sin(ship_node->getRotation().Y*0.017453));
			smgr->getActiveCamera()->setPosition(playerVector); 

		}
		if (receiver.IsKeyDown(irr::KEY_SPACE))
		{
			shoot(ship_node, 1);
		}
		
		ship_node->setRotation(shipWaving(ship_node->getRotation(), ship_x_wave, ship_z_wave));
		npc_ship_node->setRotation(shipWaving(npc_ship_node->getRotation(), npc_ship_x_wave, npc_ship_z_wave));

		for each (scene::ISceneNode* bullet in bullets)
		{
			core::vector3df ballPos = bullet->getPosition();
			core::vector3df newBallPos;
			newBallPos = ballPos + normalizeBall(bullet) *0.5f;
			bullet->setPosition(newBallPos);
		}

		driver->beginScene(true, true, SColor(255, 100, 101, 140));
		smgr->drawAll();
		driver->endScene(); 
		int fps = driver->getFPS();
		if (lastFPS != fps)
		{
			core::stringw str = L"Pirates - Irrlicht Engine [";
			str += driver->getName();
			str += "] FPS:";
			str += fps;
			device->setWindowCaption(str.c_str());
			lastFPS = fps;
		}
	}

	device->drop(); 
}