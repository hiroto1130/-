#ifndef Fbx_H
#define Fbx_H

#include <fbxsdk.h>
#include <d3d11.h>
#include <d3dx9.h>
#include <map>
#include <vector>

#include "../Utility/Vector.h"
#include "../Utility/Matrix.h"
#include "../Utility/Color.h"
#include "../../Device/Device.h"
#include "Result.h"


struct CustomVertex
{
	Utility::Vector3 position;			// 座標(x, y, z)
	Utility::Vector3 normal;			// 法線
	DWORD            color = D3DCOLOR_XRGB(255,255,255);   // color
	Utility::Vector2 uv;
};

// ボーン情報
struct Bone
{
	char		name[64];
	D3DXMATRIX	offset;
	D3DXMATRIX	transform;
};

// アニメーション
struct Motion
{
	UINT					numFrame;	// フレーム数
	std::vector<D3DXMATRIX> Key[256];	// キーフレーム
};

struct ModelData
{
	std::map<std::string,std::vector<CustomVertex>>  vertices_;
	std::map<std::string,std::vector<UINT>>          indices_;
	std::map<std::string,LPDIRECT3DVERTEXBUFFER9>    vertex_buffers_;
	std::map<std::string,LPDIRECT3DINDEXBUFFER9>     index_buffers_;
	std::map<std::string,ID3D11ShaderResourceView*>  material_links_;
	std::map<std::string,Motion>                     motions_;
	std::string                                      material_name_;

	Utility::Matrix rotation_matrix_{ 0,0,0 };

	//! 大きさの変数
	float scale_ = 0.02f;

	//! モーション開始フレーム
	int	start_frame_ = 0;

	std::string id_; // modelの名前 

	//! 移動変数
	D3DXVECTOR3 position_;

	LPDIRECT3DTEXTURE9 texture_ = nullptr;
};

class Fbx
{
public:

	Fbx() {};
	~Fbx() {};

	Result Load(ModelData& model, const char* texture_name);

private:

	void GetStartFrame(FbxScene& scene,ModelData& model)
	{
		//	モーション情報取得
		FbxArray<FbxString*> names;

		scene.FillAnimStackNameArray(names);
		// モーションのリストを名前が付いた状態で取得する

		int StartFrame = 0;
		if (names != NULL) {// ひとまずは一個のモーションをハッキリさせるようにしよう！

			FbxTakeInfo* take = scene.GetTakeInfo(names[0]->Buffer());
			// これがFbxにおけるモーションデータの名前
			// take 取得関数
			// 1take = 1モーション 　// Buffer

			FbxLongLong start = take->mLocalTimeSpan.GetStart().Get();
			FbxLongLong stop = take->mLocalTimeSpan.GetStop().Get();
			// モーションの開始時間と終了時間取得
			// start 開始　stop 終了
			// 返り値は秒数で帰ってきている

			FbxLongLong fps60 = FbxTime::GetOneFrameValue(FbxTime::eFrames60);
			//　一フレームにかかる時間を取得　　　　　　　// 何フレームか

			StartFrame = (int)(start / fps60);
			// 開始

			model.motions_["default"].numFrame = (int)((stop - start) / fps60);
			// モーションが始まってから終わるまでの時間を計算
		}
		model.start_frame_ = StartFrame;
	}

	void SetMotion(ModelData& model, FbxScene& fbx_scene);

	Result LoadFile(ModelData& model, const char* texture_name);

	void SearchMeshNode(FbxNode* node, std::map<std::string, FbxNode*>& list);

	Result CreateMesh(const char* node_name, FbxMesh* mesh, ModelData& model);

	Result CreateVertexBuffer(ModelData& model);

	Result CreateIndexBuffer(ModelData& model);

	Result MaterialLink(FbxScene* scene, ModelData& model);

	void SetVertexColor(FbxScene* scene, ModelData& model, std::string node_name);

	std::string ConvertToUTF(FbxFileTexture* texture);

	FbxFileTexture* GetTextrue(FbxMesh* mesh, FbxSurfaceMaterial* material, ModelData* model);

	Result LoadTexture(FbxMesh* mesh, FbxSurfaceMaterial* material, ModelData* model);

	Result LoadUv(FbxMesh* mesh, ModelData& model, std::string node_name);

	void CreateMesh(ModelData& model, FbxManager& fbx_manager, FbxScene& fbx_scene);
};



#endif