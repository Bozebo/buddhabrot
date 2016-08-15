#ifndef HEADER_MAT3
#define HEADER_MAT3

#include "maths.h"

namespace maths{

	struct mat3{
		union{
			float elements[9];
			vec3 cols[3];
		};

		mat3();
		mat3(float &values);

		static mat3 identity();

		static mat3 translation(const vec2 &translation);
		static mat3 rotation(float angle, const vec2 &axis);
		static mat3 scale(const vec2 scale);

		mat3& multiply(const mat3 &other);
		friend mat3 operator*(mat3 left, const mat3 &right);
		mat3& operator*=(const mat3 &other);

	};

}

#endif