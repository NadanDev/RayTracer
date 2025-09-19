#include <iostream>

#include "vec3.h"
#include "ray.h"
#include "colour.h"

using namespace std;

colour rayColour(const ray& r)
{
	vec3 unitDirection = unitVector(r.direction());
	auto a = 0.5 * (unitDirection.y() + 1.0); // a must be between 0 and 1 instead of -1 to 1
	return (1.0 - a) * colour(1.0, 1.0, 1.0) + a * colour(0.5, 0.7, 1.0); // Lerp
}

int main()
{
	auto aspectRatio = 16.0 / 9.0;
	int imageWidth = 400;

	// Image Height : Must be at least 1
	int imageHeight = int(imageWidth / aspectRatio);
	imageHeight = (imageHeight < 1) ? 1 : imageHeight;

	// Camera
	auto focalLength = 1.0;
	auto viewportHeight = 2.0;
	auto viewportWidth = viewportHeight * (double(imageWidth) / imageHeight);
	auto cameraCenter = point3(0, 0, 0);

	// Vertical and Horizontal vectors
	auto viewportU = vec3(viewportWidth, 0, 0);
	auto viewportV = vec3(0, -viewportHeight, 0);

	// Horizontal and vertical delta vectors from pixel to pixel
	auto pixelDeltaU = viewportU / imageWidth;
	auto pixelDeltaV = viewportV / imageHeight;

	// Upper left pixel
	auto viewportUpperLeft = cameraCenter - vec3(0, 0, focalLength) - viewportU / 2 - viewportV / 2;
	auto pixel00Loc = viewportUpperLeft + 0.5 * (pixelDeltaU + pixelDeltaV); // Middle


	// Render
	cout << "P3\n" << imageWidth << ' ' << imageHeight << "\n255\n";

	for (int j = 0; j < imageHeight; j++)
	{
		clog << "\rScanlines remaining: " << (imageHeight - j) << '\n';
		for (int i = 0; i < imageWidth; i++)
		{
			auto pixelCenter = pixel00Loc + (i * pixelDeltaU) + (j * pixelDeltaV);
			auto rayDirection = pixelCenter - cameraCenter;
			ray r(cameraCenter, rayDirection);

			colour pixelColour = rayColour(r);
			writeColour(cout, pixelColour);
		}
	}

	clog << "Done.\n";
}