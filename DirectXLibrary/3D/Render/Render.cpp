#include "Render.h"

void Render::WorldTransform(ModelData& model)
{
	D3DXMATRIXA16 position_matrix;
	D3DXMATRIXA16 world_matrix;
	D3DXMATRIXA16 rotation_matrix, rotation_matrix_two;

	D3DXMatrixIdentity(&world_matrix);

	// 拡縮
	D3DXMATRIXA16 scale_matrix;
	D3DXMatrixScaling(&scale_matrix, model.scale_, model.scale_, model.scale_);
	D3DXMatrixMultiply(&world_matrix, &world_matrix, &scale_matrix);
	
	// 回転処理
	D3DXMatrixIdentity(&rotation_matrix);
	D3DXMatrixRotationX(&rotation_matrix_two, model.rotation_matrix_.pitch);
	D3DXMatrixMultiply(&rotation_matrix, &rotation_matrix, &rotation_matrix_two);
	D3DXMatrixRotationY(&rotation_matrix_two, model.rotation_matrix_.yaw);
	D3DXMatrixMultiply(&rotation_matrix, &rotation_matrix, &rotation_matrix_two);
	D3DXMatrixRotationZ(&rotation_matrix_two, model.rotation_matrix_.roll);
	D3DXMatrixMultiply(&rotation_matrix, &rotation_matrix, &rotation_matrix_two);
	D3DXMatrixMultiply(&world_matrix, &world_matrix, &rotation_matrix);
	
	// 移動処理
	D3DXMatrixTranslation(&position_matrix, model.position_.x, model.position_.y, model.position_.z);
	D3DXMatrixMultiply(&world_matrix, &world_matrix, &position_matrix);
	
	LPDIRECT3DDEVICE9 device;
	Library::Device::GetDevice(&device);
	device->SetTransform(D3DTS_WORLD, &world_matrix);
}

void Render::ViewTransform()
{
	D3DXMATRIXA16 view_matrix;

	D3DXVECTOR3 vec_eye_pt(camera->pos_.x, camera->pos_.y, camera->pos_.z); //カメラ（視点）位置
	D3DXVECTOR3 vec_look_at_pt(camera->another_pos_.x, camera->another_pos_.y - 1.0f, camera->another_pos_.z + 3.0f);//注視位置
	D3DXVECTOR3 vec_up_vec(0.0f, 1.0f, 0.0f);//上方位置

	// 注視点を原点に移動
	D3DXVECTOR3 vec3 = vec_eye_pt - vec_look_at_pt;

	D3DXMATRIXA16 yaw_matrix;
	D3DXMATRIXA16 ptch_matrix;
	D3DXMATRIXA16 roll_matrix;

	D3DXMatrixRotationX(&ptch_matrix, camera->rotation_matrix_.pitch);
	D3DXMatrixRotationY(&yaw_matrix, camera->rotation_matrix_.yaw);
	D3DXMatrixRotationZ(&roll_matrix, camera->rotation_matrix_.roll);

	D3DXVec3TransformCoord(&vec3, &vec3, &ptch_matrix);
	D3DXVec3TransformCoord(&vec3, &vec3, &yaw_matrix);
	D3DXVec3TransformCoord(&vec3, &vec3, &roll_matrix);

	vec_eye_pt = vec3 + vec_look_at_pt;

	D3DXMatrixIdentity(&view_matrix);

	D3DXMATRIXA16 camera_position_matrix;

	D3DXMatrixLookAtLH(&camera_position_matrix, &vec_eye_pt, &vec_look_at_pt, &vec_up_vec);

	D3DXMatrixMultiply(&view_matrix, &view_matrix, &camera_position_matrix);

	LPDIRECT3DDEVICE9 device;
	Library::Device::GetDevice(&device);

	device->SetTransform(D3DTS_VIEW, &view_matrix);
}

void Render::ProjectionTransform()
{
	D3DXMATRIXA16 projection_matrix;

	D3DXMatrixPerspectiveFovLH(&projection_matrix,D3DX_PI / camera->perspective_, 1.0f, 1.0f, 100.0f);

	LPDIRECT3DDEVICE9 device;
	Library::Device::GetDevice(&device);

	device->SetTransform(D3DTS_PROJECTION, &projection_matrix);
}

void Render::RenderModel(ModelData& model)
{
	LPDIRECT3DDEVICE9 device;
	Library::Device::GetDevice(&device);

	for (DWORD i = 0; i < 1/*THING_AMOUNT*/; i++)
	{
		WorldTransform(model);
		ViewTransform();
		ProjectionTransform();
		SetLight();
		Rendering(model);
	}
}

void Render::SetLight()
{
	D3DXVECTOR3 vec_direction(1, 1, 1);
	D3DLIGHT9 light;
	ZeroMemory(&light, sizeof(D3DLIGHT9));
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Diffuse.r = 1.0f;
	light.Diffuse.g = 1.0f;
	light.Diffuse.b = 1.0f;
	light.Specular.r = 1.0f;
	light.Specular.g = 1.0f;
	light.Specular.b = 1.0f;
	D3DXVec3Normalize((D3DXVECTOR3*)&light.Direction, &vec_direction);
	light.Range = 200.0f;

	LPDIRECT3DDEVICE9 device;
	Library::Device::GetDevice(&device);

	device->SetLight(0, &light);
	device->LightEnable(0, TRUE);
}

void Render::Rendering(ModelData& model)
{
	LPDIRECT3DDEVICE9 device;
	Library::Device::GetDevice(&device);

	device->SetTexture(0, model.texture_);

	for (auto index : model.indices_)
	{
		// FVF設定(XYZ座標、法線)
		device->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1);

		// 頂点バッファの登録
		LPDIRECT3DVERTEXBUFFER9 vertex_buffer = model.vertex_buffers_[index.first];
		device->SetStreamSource(0, vertex_buffer, 0, sizeof(CustomVertex));

		// インデックスバッファの登録
		LPDIRECT3DINDEXBUFFER9 index_buffer = model.index_buffers_[index.first];
		device->SetIndices(index_buffer);

		// 描画
		int vertices_size = model.vertices_[index.first].size();
		int index_second_size = index.second.size();
		device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, vertices_size, 0, index_second_size);
	}
}
