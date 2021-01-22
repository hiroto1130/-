#ifndef MATRIX_H
#define MATRIX_H

namespace Utility
{
	struct Matrix
	{
		Matrix(float Yaw = 0.0f,float Pitch = 0.0f,float Roll = 0.0f)
			:yaw(Yaw),pitch(Pitch),roll(Roll) {}

		float yaw,pitch,roll;
	};

}

#endif
