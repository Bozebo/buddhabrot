#ifndef HEADER_MATHS
#define HEADER_MATHS

#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>

#include "vec2.h"
#include "vec3.h"
#include "vec4.h"

#include "mat3.h"
#include "mat4.h"

namespace maths{


	inline float toRadians(float degrees){
		return degrees * float(M_PI / 180.0f);
	}

}

#endif
