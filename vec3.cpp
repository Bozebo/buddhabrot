#include "vec3.h"

namespace maths{

	vec3::vec3(const float &x, const float &y, const float &z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}


	vec3& vec3::add(const vec3 &other){
		x += other.x;
		y += other.y;
		z += other.z;

		return *this;
	}
	vec3& vec3::subtract(const vec3 &other){
		x -= other.x;
		y -= other.y;
		z -= other.z;

		return *this;
	}
	vec3& vec3::multiply(const vec3 &other){
		x *= other.x;
		y *= other.y;
		z *= other.z;

		return *this;
	}
	vec3& vec3::divide(const vec3 &other){
		x /= other.x;
		y /= other.y;
		z /= other.z;

		return *this;
	}


	bool vec3::operator==(const vec3 &other){
		return (x == other.x && y == other.y && z == other.z);
	}
	bool vec3::operator!=(const vec3 &other){
		return (x != other.x && y != other.y && z != other.z);
	}

	vec3 operator+(vec3 left, const vec3 &right){
		return left.add(right);
	}
	vec3 operator-(vec3 left, const vec3 &right){
		return left.subtract(right);
	}
	vec3 operator*(vec3 left, const vec3 &right){
		return left.multiply(right);
	}
	vec3 operator/(vec3 left, const vec3 &right){
		return left.divide(right);
	}

	vec3& vec3::operator+=(const vec3 &other){
		*this = *this + other;

		return *this;
	}
	vec3& vec3::operator-=(const vec3 &other){
		*this = *this - other;

		return *this;
	}
	vec3& vec3::operator*=(const vec3 &other){
		*this = *this * other;

		return *this;
	}
	vec3& vec3::operator/=(const vec3 &other){
		*this = *this / other;

		return *this;
	}

}
