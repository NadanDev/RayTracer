#pragma once

#include "vec3.h"

using namespace std;

using colour = vec3;

void writeColour(ostream& out, const colour& pixelColour)
{
	auto r = pixelColour.x();
	auto g = pixelColour.y();
	auto b = pixelColour.z();

	int rByte = int(255.999 * r);
	int gByte = int(255.999 * g);
	int bByte = int(255.999 * b);

	out << rByte << ' ' << gByte << ' ' << bByte << '\n';
}