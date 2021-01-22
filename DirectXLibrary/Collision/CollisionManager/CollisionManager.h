#ifndef COLLISION_MANAGER_H
#define COLLISION_MANAGER_H

#include "../Collider.h"
#include "../../3D/Fbx/FBX.h"

#include <map>

class CollisionManager
{
public:

	void CollitionCheck()
	{
		int loop_count = 0;
		for (auto collider:colliders_)// メインのcollider配列
		{
			++loop_count;
			for (auto kind_name : collider->target_kinds)// 分類
			{
				for (auto target_collider : names[kind_name])// 個
				{
					IsHit(collider,target_collider);
				}
			}

			for (auto object_name : collider->targets)// 個
			{
				for (int target_num = loop_count;target_num < colliders_.size();++target_num)
				{
					if (object_name == colliders_[target_num]->id)
					{
						IsHit(collider,colliders_[target_num]);
					}
				}
			}
		}
	};

	void Entry(std::string kind, Collider* collider)
	{
		names[kind].push_back(collider);
		colliders_.push_back(collider);
	}

	void Erase(std::string id, Collider* collider)
	{
	}

private:

	bool IsHit(Collider* left_col, Collider* right_col)
	{
		return true;
	};

	std::map<std::string, std::vector<Collider*>> names;// 分類、個人名を保存する配列
	std::vector<Collider*> colliders_;
};

#endif
