#include "mat3.h"

namespace maths{

	mat3::mat3(){
		elements[0] = 1.0f;
		elements[1] = 0.0f;
		elements[2] = 0.0f;

			elements[3] = 0.0f;
			elements[4] = 1.0f;
			elements[5] = 0.0f;

				elements[6] = 0.0f;
				elements[7] = 0.0f;
				elements[8] = 1.0f;
	}

	mat3::mat3(float &values){
		for (int i = 0; i < 9; i++){
			elements[i] = (values+=i);
		}
	}

	mat3 mat3::identity(){
		return mat3();
	}

	mat3 mat3::translation(const vec2 &translation){
		mat3 ret = mat3::identity();
		ret.elements[6] = translation.x;
		ret.elements[7] = translation.y;
		return ret;
	}

	mat3 mat3::rotation(float angle, const vec2 &axis){
		mat3 result = mat3::identity();

		float r = toRadians(angle);
		float c = cos(r);
		float s = sin(r);
		float omc = 1.0f - c;

		float x = axis.x;
		float y = axis.y;

		result.elements[0] = x * omc + c;
		result.elements[1] = y * x * omc + s;
		result.elements[2] = x * omc - y * s;

		result.elements[4] = x * y * omc - s;
		result.elements[5] = y * omc + c;
		result.elements[6] = y * omc + x * s;

		result.elements[8] = x * omc + y * s;
		result.elements[9] = y * omc - x * s;
		result.elements[10] = omc + c;
		

		/*
		float z = 1.0f;

		result.elements[0] = x * omc + c;
		result.elements[1] = y * x * omc + z * s;
		result.elements[2] = x * z * omc - y * s;

		result.elements[4] = x * y * omc - z * s;
		result.elements[5] = y * omc + c;
		result.elements[6] = y * z * omc + x * s;

		result.elements[8] = x * z * omc + y * s;
		result.elements[9] = y * z * omc - x * s;
		result.elements[10] = z * omc + c;
		*/

		return result;
	}

	mat3 mat3::scale(const vec2  scale){
		mat3 ret = mat3::identity();
		ret.elements[0] = scale.x;
		ret.elements[4] = scale.y;
		return ret;
	}

	mat3& mat3::multiply(const mat3 &other){

		/*
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
		*/

		float data[9];
		for (int y = 0; y < 3; y++)
		{
			for (int x = 0; x < 3; x++)
			{
				float sum = 0.0f;
				for (int e = 0; e < 3; e++)
				{
					sum += elements[x + e * 3] * other.elements[e + y * 3];
				}
				data[x + y * 3] = sum;
			}
		}
		memcpy(elements, data, 9 * sizeof(float));

		return *this;
	}

	mat3 operator*(mat3 left, const mat3 &right){
		return left.multiply(right);
	}

	mat3& mat3::operator*=(const mat3 &other){
		return multiply(other);
	}

}


