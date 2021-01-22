#ifndef COLLIDER_H
#define COLLIDER_H

#include <string>
#include <vector>
#include "../Utility/Position.h"

enum class Type
{
	Box,
	Sphere
};

struct Parameter
{
	Utility::Position pos;
};

struct Collider
{
	Type type; // 当たり判定を取る形状
	std::string id; // 自身の名前
	std::vector<std::string> targets; // 当たり判定を行う相手のid(名前)
	std::vector<std::string> target_kinds; // 当たり判定を行う種類
	Parameter parameter; // 当たり判定を取る為の情報(Positionなど)
};

#endif
