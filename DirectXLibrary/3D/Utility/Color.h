#ifndef COLOR_H
#define COLOR_H

namespace Utility
{
	struct Color
	{
		Color(float Red = 1.0f, float Green = 1.0f, float Blue = 1.0f, float Alpha = 1.0f)
			: red(Red), m_green(Green), m_blue(Blue), m_alpha(Alpha)
		{}

		float red,m_green,m_blue,m_alpha;
	};
}
#endif
