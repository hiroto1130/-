#include "FBX.h"

Result Fbx::Load(ModelData& model, const char* texture_name)
{
	Result result;
	result = LoadFile(model,texture_name);
	if (result != Result::Success)
	{
		return result;
	}

	// VertexBuffer作成
	result = CreateVertexBuffer(model);
	if (result != Result::Success)
	{
		return result;
	}

	// IndexBuffer作成
	result = CreateIndexBuffer(model);
	if (result != Result::Success)
	{
		return result;
	}

	return Result::Success;
}

Result Fbx::CreateVertexBuffer(ModelData& model)
{
	LPDIRECT3DDEVICE9 device;
	Library::Device::GetDevice(&device);

	for (auto vertex_buffer : model.vertices_)
	{
		if (FAILED(device->CreateVertexBuffer(
			sizeof(CustomVertex) * vertex_buffer.second.size(), 0, D3DFVF_XYZ | D3DFVF_NORMAL, D3DPOOL_MANAGED,
			&model.vertex_buffers_[vertex_buffer.first], nullptr)))
		{
			return Result::FailsCreateVertexBuffer;
		}

		// 頂点バッファにデータをコピーする
		CustomVertex* list = nullptr;
		// Lock関数はGPUのメモリへのアクセス権をもらう関数
		if (SUCCEEDED(model.vertex_buffers_[vertex_buffer.first]->Lock(0, 0, (void**)&list, 0)))
		{
			// 頂点データをコピー
			for (int i = 0; i < vertex_buffer.second.size(); i++)
			{
				list[i] = vertex_buffer.second[i];
			}

			model.vertex_buffers_[vertex_buffer.first]->Unlock();
		}
	}

	return Result::Success;
}

Result Fbx::CreateIndexBuffer(ModelData& model)
{
	LPDIRECT3DDEVICE9 device;
	Library::Device::GetDevice(&device);

	for (auto index : model.indices_)
	{
		_D3DFORMAT format = _D3DFORMAT::D3DFMT_INDEX32;

		if (FAILED(device->CreateIndexBuffer(
			sizeof(UINT) * index.second.size(), D3DUSAGE_WRITEONLY,
			format, D3DPOOL_MANAGED, &model.index_buffers_[index.first], nullptr)))
		{
			return Result::FailsCreateIndexBuffer;
		}

		// インデックスバッファにコピーする
		UINT* index_buffer;
		if (SUCCEEDED(model.index_buffers_[index.first]->Lock(0, 0, (void**)&index_buffer, 0)))
		{
			// インデックスデータをコピー
			for (int i = 0; i < index.second.size() / 3; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					index_buffer[i * 3 + j] = index.second[i * 3 + j];
				}
			}
			model.index_buffers_[index.first]->Unlock();
		}
	}

	return Result::Success;
}

void Fbx::SetMotion(ModelData& model, FbxScene& fbx_scene)
{
	//モーション情報取得
	FbxArray<FbxString*> names;

	// モーションのリストを名前が付いた状態で取得する
	fbx_scene.FillAnimStackNameArray(names);

	int motion_start_frame = 0;
	// ひとまずは一個のモーションをハッキリさせる
	if (names != NULL) {

		// これがFbxにおけるモーションデータの名前
		FbxTakeInfo* take = fbx_scene.GetTakeInfo(names[0]->Buffer());

		// モーションの開始時間と終了時間取得
		// start 開始　stop 終了
		// 返り値は秒数で帰ってきている
		FbxLongLong start = take->mLocalTimeSpan.GetStart().Get();
		FbxLongLong stop = take->mLocalTimeSpan.GetStop().Get();

		//　一フレームにかかる時間を取得　　　　　　　// 何フレームか
		FbxLongLong fps60 = FbxTime::GetOneFrameValue(FbxTime::eFrames60);

		// 開始
		motion_start_frame = (int)(start / fps60);
		
		// モーションが始まってから終わるまでの時間を計算
		model.motions_["default"].numFrame = (int)((stop - start) / fps60);
	}
	model.start_frame_ = motion_start_frame;
}

Result Fbx::LoadFile(ModelData& model,const char* texture_name)
{
	FbxManager* fbx_manager = FbxManager::Create();

	FbxImporter* fbx_importer = nullptr;
	fbx_importer = FbxImporter::Create(fbx_manager, "SceneName");
	// 作成失敗
	if (fbx_importer == nullptr)
	{
		fbx_manager->Destroy();
		return Result::FailsCreatInporter;
	}

	FbxScene* fbx_scene = nullptr;
	fbx_scene = FbxScene::Create(fbx_manager, "SceneName");
	// 作成失敗
	if (fbx_scene == nullptr)
	{
		fbx_importer->Destroy();
		fbx_manager->Destroy();
		return Result::FailsCreatScene;
	}

	// 初期化失敗
	if (fbx_importer->Initialize(texture_name) == false)
	{
		fbx_importer->Destroy();
		fbx_manager->Destroy();
		fbx_scene->Destroy();
		return Result::FailsInporterInitialize;
	}
	
	// インポート失敗
	if (fbx_importer->Import(fbx_scene) == false)
	{
		fbx_importer->Destroy();
		fbx_manager->Destroy();
		fbx_scene->Destroy();
		return Result::FailsInporter;
	}

	FbxNode* root_node = fbx_scene->GetRootNode();
	// 取得失敗
	if (root_node == nullptr)
	{
		fbx_importer->Destroy();
		fbx_manager->Destroy();
		fbx_scene->Destroy();
		return Result::FailsGetNode;
	}

	CreateMesh(model, *fbx_manager, *fbx_scene);

	Result result = MaterialLink(fbx_scene,model);
	if (result != Result::Success)
	{
		fbx_importer->Destroy();
		fbx_manager->Destroy();
		fbx_scene->Destroy();
		return result;
	}

	//---- delete ----/

	if (fbx_importer != nullptr)
	{
		fbx_importer->Destroy();
	}

	if (fbx_scene != nullptr)
	{
		fbx_scene->Destroy();
	}

	if (fbx_manager != nullptr)
	{
		fbx_manager->Destroy();
	}

	return Result::Success;
}

void Fbx::CreateMesh(ModelData& model,FbxManager& fbx_manager, FbxScene& fbx_scene)
{
	FbxGeometryConverter converter(&fbx_manager);
	// ポリゴンを三角形にする
	converter.Triangulate(&fbx_scene, true);

	std::map<std::string, FbxNode*> mesh_node_list;

	// meshのリスト作成
	SearchMeshNode(fbx_scene.GetRootNode(), mesh_node_list);

	//SetMotion();

	// 全Mesh分割
	converter.SplitMeshesPerMaterial(&fbx_scene, true);

	for (auto data : mesh_node_list)
	{
		// mesh作成
		CreateMesh(data.first.c_str(), data.second->GetMesh(), model);
		LoadUv(data.second->GetMesh(), model, data.first.c_str());
		SetVertexColor(&fbx_scene, model, data.first.c_str());
	}
}

void Fbx::SearchMeshNode(FbxNode* node, std::map<std::string, FbxNode*>& list)
{
	for (int i = 0; i < node->GetNodeAttributeCount(); i++)
	{
		FbxNodeAttribute* attribute = node->GetNodeAttributeByIndex(i);

		// Attributeがメッシュなら追加
		if (attribute->GetAttributeType() == FbxNodeAttribute::EType::eMesh)
		{
			list[node->GetName()] = node;
			break;
		}
	}

	for (int i = 0; i < node->GetChildCount(); i++)
	{
		SearchMeshNode(node->GetChild(i), list);
	}
}

Result Fbx::CreateMesh(const char* node_name, FbxMesh* mesh, ModelData& model)
{
	// 頂点バッファの取得
	FbxVector4* vertices = mesh->GetControlPoints();
	// インデックスバッファの取得
	int* indices = mesh->GetPolygonVertices();
	// 頂点座標の数の取得
	int polygon_vertex_count = mesh->GetPolygonVertexCount();
	// GetPolygonVertexCount => 頂点数
	for (int i = 0; i < polygon_vertex_count; i++)
	{
		CustomVertex vertex;
		// インデックスバッファから頂点番号を取得
		int index = indices[i];

		// 頂点座標リストから座標を取得する
		vertex.position.x = (float)-vertices[index][0];
		vertex.position.y = (float)vertices[index][1];
		vertex.position.z = (float)vertices[index][2];

		// 追加
		model.vertices_[node_name].push_back(vertex);
	}

	FbxArray<FbxVector4> normals;
	// 法線リストの取得
	mesh->GetPolygonVertexNormals(normals);

	// 法線設定
	for (int i = 0; i < normals.Size(); i++)
	{
		model.vertices_[node_name][i].normal.x = (float)-normals[i][0];
		model.vertices_[node_name][i].normal.y = (float)normals[i][1];
		model.vertices_[node_name][i].normal.z = (float)normals[i][2];
	}

	// ポリゴン数の取得
	int polygon_count = mesh->GetPolygonCount();

	// ポリゴンの数だけ連番として保存する
	for (int i = 0; i < polygon_count; i++)
	{
		model.indices_[node_name].push_back(i * 3 + 2);
		model.indices_[node_name].push_back(i * 3 + 1);
		model.indices_[node_name].push_back(i * 3);
	}

	return Result::Success;
}

Result Fbx::LoadUv(FbxMesh* mesh, ModelData& model, std::string node_name)
{
	// uvsetの名前保存用
	FbxStringList uvset_names;
	// UVSetの名前リストを取得
	mesh->GetUVSetNames(uvset_names);

	FbxArray<FbxVector2> uv_buffer;

	// UVSetの名前からUVSetを取得する
	// 今回はシングルなので最初の名前を使う
	mesh->GetPolygonVertexUVs(uvset_names.GetStringAt(0), uv_buffer);

	for (int i = 0; i < uv_buffer.Size(); i++)
	{
		FbxVector2& uv = uv_buffer[i];

		// そのまま使用して問題ない
		model.vertices_[node_name][i].uv.x = (float)uv[0];
		model.vertices_[node_name][i].uv.y = (float)(1.0 - uv[1]);
	}

	return Result::Success;
}

void Fbx::SetVertexColor(FbxScene* scene, ModelData& model, std::string node_name)
{
	// FbxMeshの数を取得
	int mesh_num = scene->GetSrcObjectCount<FbxMesh>();

	for (int i = 0; i < mesh_num; i++)
	{
		FbxMesh* mesh = scene->GetSrcObject<FbxMesh>(i);

		// 頂点カラーのデータ数を確認
		int color_count = mesh->GetElementVertexColorCount();
		if (color_count == 0)
		{
			return;
		}

		// 頂点カラーデータの取得
		FbxGeometryElementVertexColor* vertex_colors = mesh->GetElementVertexColor(0);

		if (vertex_colors == nullptr) { return; }

		// モードチェック
		if (vertex_colors->GetMappingMode() == FbxLayerElement::eByPolygonVertex)
		{
			// 参照方法チェック
			if (vertex_colors->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
			{
				// 頂点カラーバッファ取得
				FbxLayerElementArrayTemplate<FbxColor>& colors = vertex_colors->GetDirectArray();
				// 頂点カラーインデックスバッファ
				FbxLayerElementArrayTemplate<int>& indeces = vertex_colors->GetIndexArray();

				for (int i = 0; i < indeces.GetCount(); i++)
				{
					int id = indeces.GetAt(i);
					FbxColor color = colors.GetAt(id);
					int a = static_cast<int>(color.mAlpha * 255.f);
					int r = static_cast<int>(color.mRed * 255.f);
					int g = static_cast<int>(color.mGreen * 255.f);
					int b = static_cast<int>(color.mBlue * 255.f);
					model.vertices_[node_name][i].color = D3DCOLOR_XRGB(a, r, g, b);
				}
			}
		}
	}
}

Result Fbx::MaterialLink(FbxScene* scene, ModelData& model)
{
	int mesh_num = scene->GetSrcObjectCount<FbxMesh>();
	
	for (int now_mesh = 0; now_mesh < mesh_num; ++now_mesh)
	{
		FbxMesh* mesh = scene->GetSrcObject<FbxMesh>(now_mesh);

		// マテリアルが無ければ終わり
		if (mesh->GetElementMaterialCount() == 0)
		{
			return Result::NotFoundMaterial;
		}

		// Mesh側のマテリアル情報を取得
		FbxLayerElementMaterial* material = mesh->GetElementMaterial(0);
		int index = material->GetIndexArray().GetAt(0);
		FbxSurfaceMaterial* surface_material = mesh->GetNode()->GetSrcObject<FbxSurfaceMaterial>(index);

		if (surface_material != nullptr)
		{
			model.material_name_ = surface_material->GetName();
		}
		else
		{
			model.material_name_ = "";
		}

		Result result = LoadTexture(mesh, surface_material, &model);
		if (result != Result::Success)
		{
			return result;
		}
	}
	return Result::Success;
}

Result Fbx::LoadTexture(FbxMesh* mesh, FbxSurfaceMaterial* material, ModelData* model)
{
	FbxFileTexture* texture = GetTextrue(mesh, material, model);

	if (texture == nullptr){
		return Result::NotFoundFileTexture;
	}

	std::string file_name = ConvertToUTF(texture);

	LPDIRECT3DDEVICE9 device;
	Library::Device::GetDevice(&device);

	if (D3DXCreateTextureFromFileEx(device, file_name.c_str(),
		0, 0, 1, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT,
		0x0000ff00, nullptr, nullptr, &model->texture_) != S_OK)
	{
		return Result::FailsLoadTextrue;
	}

	return Result::Success;
}

FbxFileTexture* Fbx::GetTextrue(FbxMesh* mesh, FbxSurfaceMaterial* material, ModelData* model)
{
	// Diffuseプロパティを取得
	FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);

	FbxFileTexture* texture = nullptr;

	// FbxFileTextureを取得する
	int texture_num = prop.GetSrcObjectCount<FbxFileTexture>();
	if (texture_num > 0)
	{
		// propからFbxFileTextureを取得	
		texture = prop.GetSrcObject<FbxFileTexture>(0);
	}
	else
	{
		// 失敗したらマルチテクスチャの可能性を考えてFbxLayeredTextureを指定する
		// FbxLayeredTextureからFbxFileTextureを取得
		int layer_num = prop.GetSrcObjectCount<FbxLayeredTexture>();
		if (layer_num > 0)
		{
			texture = prop.GetSrcObject<FbxFileTexture>(0);
		}
	}

	return texture;
}

std::string Fbx::ConvertToUTF(FbxFileTexture* texture)
{
	// ファイル名を取得
	std::string file_name_org = texture->GetRelativeFileName();

	char* buffer;
	size_t size = 0;
	FbxUTF8ToAnsi(file_name_org.c_str(), buffer, &size);

	std::string texture_name, tmp;
	int max = strlen(buffer);
	for (int i = max; i > 0; --i) {
		if (buffer[i] == '\\') {
			texture_name = &buffer[i + 1];
			break;
		}
	}

	const char* root_path = "";
	std::string file_name = root_path + texture_name;

	return file_name;
}