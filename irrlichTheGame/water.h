#ifndef _WATER_SCENE_NODE_H
#define _WATER_SCENE_NODE_H

#include <irrlicht.h>

using namespace irr;

class CustomWaterSceneNode: public scene::ISceneNode, video::IShaderConstantSetCallBack
{
public:
	CustomWaterSceneNode(scene::ISceneManager* sceneManager, f32 width, f32 height,
							const irr::core::stringc& resourcePath = irr::core::stringc(),
							core::dimension2du renderTargetSize=core::dimension2du(512,512),scene::ISceneNode* parent = NULL, s32 id = -1);
	virtual ~CustomWaterSceneNode();
	virtual void OnRegisterSceneNode();
	virtual void OnAnimate(u32 timeMs);
	virtual void render();
	virtual const core::aabbox3d<f32>& getBoundingBox() const;
	virtual void OnSetConstants(video::IMaterialRendererServices* services, s32 userData);

private:

	scene::ICameraSceneNode*		_camera;
	scene::ISceneNode*				_waterSceneNode;
	video::IVideoDriver*			_videoDriver;
	scene::ISceneManager*			_sceneManager;	
	core::dimension2d<f32>			_size;
	s32								_shaderMaterial;
	scene::IAnimatedMesh*			_waterMesh;
	video::ITexture*				_refractionMap;
	video::ITexture*				_reflectionMap;
	f32								_windForce;
	core::vector2df					_windDirection;
	f32								_waveHeight;
	video::SColorf					_waterColor;
	f32								_colorBlendFactor;
	u32								_time;
};

#endif