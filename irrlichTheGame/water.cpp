#include "water.h"

CustomWaterSceneNode::CustomWaterSceneNode(
	scene::ISceneManager* sceneManager, f32 width, 	f32 height, const irr::core::stringc& resourcePath, core::dimension2du renderTargetSize, scene::ISceneNode* parent, s32 id):
	scene::ISceneNode(parent, sceneManager, id), _time(0),
	_size(width, height), _sceneManager(sceneManager), _refractionMap(NULL), _reflectionMap(NULL),
	_windForce(2.0f),_windDirection(0, 1),_waveHeight(0.4f), _waterColor(0.1f, 0.1f, 0.6f, 1.f), _colorBlendFactor(0.2f), _camera(NULL)
{
	_videoDriver = sceneManager->getVideoDriver();

	_camera = sceneManager->addCameraSceneNode(0, core::vector3df(0, 0, 0), core::vector3df(0, 0, 0), -1, false);
	_waterMesh = sceneManager->addHillPlaneMesh("CustomWater", _size, core::dimension2d<u32>(1, 1));
	_waterSceneNode = sceneManager->addMeshSceneNode(_waterMesh->getMesh(0), this);
	video::IGPUProgrammingServices* GPUProgrammingServices = _videoDriver->getGPUProgrammingServices();

	core::stringc waterPixelShader;
	core::stringc waterVertexShader;

	if (_videoDriver->getDriverType() == video::EDT_DIRECT3D9)
	{
		waterPixelShader = resourcePath + "/shaders/Water_ps.hlsl";
		waterVertexShader = resourcePath + "/shaders/Water_vs.hlsl";
	}
	
	_shaderMaterial = GPUProgrammingServices->addHighLevelShaderMaterialFromFiles(
		waterVertexShader.c_str(), "main", video::EVST_VS_1_1,
		waterPixelShader.c_str(), "main", video::EPST_PS_1_1,
		this);

	_waterSceneNode->setMaterialType((video::E_MATERIAL_TYPE)_shaderMaterial);

	irr::video::ITexture* bumpTexture = _videoDriver->getTexture(resourcePath + "data/normalMap.png");
	_waterSceneNode->setMaterialTexture(0, bumpTexture);

	_refractionMap = _videoDriver->addRenderTargetTexture(renderTargetSize);
	_reflectionMap = _videoDriver->addRenderTargetTexture(renderTargetSize);

	_waterSceneNode->setMaterialTexture(1, _refractionMap);
	_waterSceneNode->setMaterialTexture(2, _reflectionMap);
}

CustomWaterSceneNode::~CustomWaterSceneNode()
{

}

void CustomWaterSceneNode::OnRegisterSceneNode()
{
	ISceneNode::OnRegisterSceneNode();

	if (IsVisible)
	{
		_sceneManager->registerNodeForRendering(this);
	}
}

void CustomWaterSceneNode::OnAnimate(u32 timeMs)
{
	ISceneNode::OnAnimate(timeMs);
	_time = timeMs;
	const f32 CLIP_PLANE_OFFSET_Y = 5.0f;

	if (IsVisible)
	{
		setVisible(false); 	
		//reflection
		_videoDriver->setRenderTarget(_refractionMap, true, true); 		
		core::plane3d<f32> refractionClipPlane(0, RelativeTranslation.Y + CLIP_PLANE_OFFSET_Y, 0, 0, -1, 0); 
		_videoDriver->setClipPlane(0, refractionClipPlane, true);
		_sceneManager->drawAll(); 
		//reflection
		_videoDriver->setRenderTarget(_reflectionMap, true, true); 
		scene::ICameraSceneNode* currentCamera = _sceneManager->getActiveCamera();
		_camera->setFarValue(currentCamera->getFarValue());
		_camera->setFOV(currentCamera->getFOV());
		core::vector3df position = currentCamera->getAbsolutePosition();
		position.Y = -position.Y + 2 * RelativeTranslation.Y; 
		_camera->setPosition(position);
		core::vector3df target = currentCamera->getTarget();		
		target.Y = -target.Y + 2 * RelativeTranslation.Y;
		_camera->setTarget(target);		
		_sceneManager->setActiveCamera(_camera);
		core::plane3d<f32> reflectionClipPlane(0, RelativeTranslation.Y - CLIP_PLANE_OFFSET_Y, 0, 0, 1, 0);
		_videoDriver->setClipPlane(0, reflectionClipPlane, true);
		_sceneManager->drawAll(); 
		_videoDriver->enableClipPlane(0, false);
		_videoDriver->setRenderTarget(0, false, true);
		_sceneManager->setActiveCamera(currentCamera);
		setVisible(true); 
	}
}

void CustomWaterSceneNode::render()
{

}

const core::aabbox3d<f32>& CustomWaterSceneNode::getBoundingBox() const
{
	return _waterSceneNode->getBoundingBox();
}

void CustomWaterSceneNode::OnSetConstants(video::IMaterialRendererServices* services, s32 userData)
{
	video::IVideoDriver* driver = services->getVideoDriver();

	core::matrix4 projection = driver->getTransform(video::ETS_PROJECTION);
	core::matrix4 view = driver->getTransform(video::ETS_VIEW);
	core::matrix4 world = driver->getTransform(video::ETS_WORLD);

	core::matrix4 cameraView = _camera->getViewMatrix();
	
	core::matrix4 worldViewProj = projection;			
	worldViewProj *= view;
	worldViewProj *= world;
	
	core::matrix4 worldReflectionViewProj = projection;
	worldReflectionViewProj *= cameraView;
	worldReflectionViewProj *= world;
	
	f32 waveLength = 0.1f;
	f32 time = _time / 100000.0f;
	core::vector3df cameraPosition = _sceneManager->getActiveCamera()->getPosition();

	services->setVertexShaderConstant("WorldViewProj", worldViewProj.pointer(), 16);
	services->setVertexShaderConstant("WorldReflectionViewProj", worldReflectionViewProj.pointer(), 16);
	services->setVertexShaderConstant("WaveLength", &waveLength, 1);
	services->setVertexShaderConstant("Time", &time, 1);
	services->setVertexShaderConstant("WindForce", &_windForce, 1);
	services->setVertexShaderConstant("WindDirection", &_windDirection.X, 2);
	services->setPixelShaderConstant("CameraPosition", &cameraPosition.X, 3);
	services->setPixelShaderConstant("WaveHeight", &_waveHeight, 1);
	services->setPixelShaderConstant("WaterColor", &_waterColor.r, 4);
	services->setPixelShaderConstant("ColorBlendFactor", &_colorBlendFactor, 1);

}