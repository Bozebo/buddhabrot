#ifndef HEADER_MAT4
#define HEADER_MAT4

#include "maths.h"

namespace maths{

	struct mat4{
		union{
			float elements[16];
			vec4 cols[4];
		};

		mat4();
		mat4(float &values);

		static mat4 identity();

		static mat4 orthographic(float left, float right, float bottom, float top, float near, float far);
		static mat4 perspective(float fov, float aspect, float near, float far);

		static mat4 translation(const vec3 &translation);
		static mat4 rotation(float angle, const vec3 &axis);
		static mat4 scale(const vec3  scale);

		mat4& multiply(const mat4 &other);
		friend mat4 operator*(mat4 left, const mat4 &right);
		mat4& operator*=(const mat4 &other);

	};

}

#endif