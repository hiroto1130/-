#ifndef RENDER_H
#define RENDER_H

#include <d3dx9.h>

#include "../../Device/Device.h"
#include "../Fbx/FBX.h"
#include "Camera.h"


class Render
{
public:

	Render() { camera = new Camera();}
	~Render() { delete camera;}

	void RenderModel(ModelData& model);
	void Rendering(ModelData& model);

private:

	Camera* camera = nullptr;

	void WorldTransform(ModelData& model);

	void ViewTransform();

	void ProjectionTransform();

	void SetLight();

};

#endif
