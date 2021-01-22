#ifndef CAMERA_H
#define CAMERA_H

#include "../Utility/Vector.h"
#include "../Utility/Matrix.h"

class Camera
{
public:

	//!カメラ変数
	Utility::Vector3 pos_{ 0.0f,2.0f,-10.0f };

	Utility::Vector3 another_pos_{ 0.0f,0.0f,0.0f };

	Utility::Matrix rotation_matrix_{ 0,0,0 };

	float perspective_ = 4;

	// 角度
	Utility::Vector3 angle_pos_{ 0,0,0 };

};

#endif
