#include "../../DirectXLibrary/Winodw/Window.h"
#include "../../DirectXLibrary/Device/Device.h"
#include "../../DirectXLibrary/Device/Input/Key.h"

#include"../../DirectXLibrary/3D/Render/Render.h"
#include "../../DirectXLibrary/3D/Fbx/FBX.h"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR szStr, int iCmdShow)
{

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	Library::Window* window = new Library::Window();
	Library::WindowParameter::SetRectSize(RectSize{ 700,700 });

	window->Cenerate();

	Library::Device* device = new Library::Device();
	Library::Key* key = new Library::Key;

	ModelData* model = new ModelData();
	ModelData* model_2 = new ModelData();
	model_2->scale_ = 1.5;
	Fbx* fbx = new Fbx();
	Render* render = new Render();

	fbx->Load(*model,"Box.fbx");
	model->scale_ = 0.05;
	fbx->Load(*model_2,"Box.fbx");
	model_2->scale_ = 0.05;

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			key->UpdateKeyStatus();
			device->DrawStart();

			if (key->IsPressed(DIK_UP) || key->IsHeld(DIK_UP))
			{
				model_2->position_.y += 1;
			}else
				if (key->IsPressed(DIK_DOWN) || key->IsHeld(DIK_DOWN))
				{
					model_2->position_.y -= 1;
				}else
					if (key->IsPressed(DIK_RIGHT) || key->IsHeld(DIK_RIGHT))
					{
						model_2->position_.x += 1;
					}else
						if (key->IsPressed(DIK_LEFT) || key->IsHeld(DIK_LEFT))
						{
							model_2->position_.x -= 1;
						}

			render->RenderModel(*model);
			render->RenderModel(*model_2);

			model->rotation_matrix_.yaw += 0.01;


			device->DrawEnd();

		}
	}

	delete window;
	delete key;

	return (INT)msg.wParam;


}