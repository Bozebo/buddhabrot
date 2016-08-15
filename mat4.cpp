#include "mat4.h"

namespace maths{

	mat4::mat4(){
		elements[0] = 1.0f;
		elements[1] = 0.0f;
		elements[2] = 0.0f;
		elements[3] = 0.0f;

			elements[4] = 0.0f;
			elements[5] = 1.0f;
			elements[6] = 0.0f;
			elements[7] = 0.0f;

				elements[8] = 0.0f;
				elements[9] = 0.0f;
				elements[10] = 1.0f;
				elements[11] = 0.0f;

					elements[12] = 0.0f;
					elements[13] = 0.0f;
					elements[14] = 0.0f;
					elements[15] = 1.0f;
	}

	mat4::mat4(float &values){
		for (int i = 0; i < 16; i++){
			elements[i] = (values+=i);
		}
	}

	mat4 mat4::identity(){
		return mat4();
	}


	mat4 mat4::orthographic(float left, float right, float bottom, float top, float near, float far){
		mat4 result = mat4::identity();

		result.elements[0 + 0 * 4] = 2.0f / (right - left);

		result.elements[1 + 1 * 4] = 2.0f / (top - bottom);

		result.elements[2 + 2 * 4] = 2.0f / (near - far);

		result.elements[0 + 3 * 4] = (left + right) / (left - right);
		result.elements[1 + 3 * 4] = (bottom + top) / (bottom - top);
		result.elements[2 + 3 * 4] = (far + near) / (far - near);

		return result;
	}

	mat4 mat4::perspective(float fov, float aspect, float near, float far){
		mat4 result = mat4::identity();
		float q = 1.0f / tan(toRadians(0.5f * fov));
		float a = q / aspect;

		float b = (near + far) / (near - far);
		float c = (2.0f * near * far) / (near - far);

		result.elements[0 + 0 * 4] = a;
		result.elements[1 + 1 * 4] = q;
		result.elements[2 + 2 * 4] = b;
		result.elements[3 + 2 * 4] = -1.0f;
		result.elements[2 + 3 * 4] = c;

		return result;
	}

	mat4 mat4::translation(const vec3 &translation){
		mat4 ret = mat4::identity();
		ret.elements[12] = translation.x;
		ret.elements[13] = translation.y;
		ret.elements[14] = translation.z;
		return ret;
	}
	mat4 mat4::rotation(float angle, const vec3 &axis){
		mat4 result = mat4::identity();

		float r = toRadians(angle);
		float c = cos(r);
		float s = sin(r);
		float omc = 1.0f - c;

		float x = axis.x;
		float y = axis.y;
		float z = axis.z;

		result.elements[0] = x * omc + c;
		result.elements[1] = y * x * omc + z * s;
		result.elements[2] = x * z * omc - y * s;

		result.elements[4] = x * y * omc - z * s;
		result.elements[5] = y * omc + c;
		result.elements[6] = y * z * omc + x * s;

		result.elements[8] = x * z * omc + y * s;
		result.elements[9] = y * z * omc - x * s;
		result.elements[10] = z * omc + c;

		return result;
	}
	mat4 mat4::scale(const vec3  scale){
		mat4 ret = mat4::identity();
		ret.elements[0] = scale.x;
		ret.elements[5] = scale.y;
		ret.elements[10] = scale.z;
		return ret;
	}


	mat4& mat4::multiply(const mat4 &other){

		float data[16];
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				float sum = 0.0f;
				for (int e = 0; e < 4; e++)
				{
					sum += elements[x + e * 4] * other.elements[e + y * 4];
				}
				data[x + y * 4] = sum;
			}
		}
		memcpy(elements, data, 16 * sizeof(float));

		return *this;
	}

	mat4 operator*(mat4 left, const mat4 &right){
		return left.multiply(right);
	}

	mat4& mat4::operator*=(const mat4 &other){
		return multiply(other);
	}

}
