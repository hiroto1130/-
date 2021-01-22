#ifndef VECTOR_H
#define VECTOR_H 

namespace Utility
{
	struct Vector3
	{
		Vector3(float X= 0.0f, float Y = 0.0f, float Z = 0.0f) : x(X),y(Y),z(Z) {}

		float x, y,z;
	};

	struct Vector2
	{
		Vector2(float X = 0.0f, float Y = 0.0f) : x(X),y(Y) {}

		float x,y;
	};
}

#endif
