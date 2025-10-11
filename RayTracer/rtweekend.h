#pragma once

#include <cmath>
#include <iostream>
#include <limits>
#include <memory>


// C++ Std Usings
using namespace std;
using std::make_shared;
using std::shared_ptr;

// Constants
const double infinity = numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility Functions
inline double degreesToRadians(double degrees)
{
	return degrees * pi / 180.0;
}

// Common Headers
#include "colour.h"
#include "ray.h"
#include "vec3.h"