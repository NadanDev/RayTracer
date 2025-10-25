#pragma once

#include "material.h"

struct Camera
{
	bool live = false;

	// Camera Settings
	point3 cameraCenter = point3(0, 0, 0);
	double FOV = 0;
	const double focalLength = 1;
	double viewportHeight = 0;
	double viewportWidth = 0;

	// Antialiasing
	bool enableAntialiasing = false;
	int samplesPerPixel = 0;
	double pixelSamplesScale = 0;

	// Bounces
	int maxDepth = 0;

	// Live rotation of camera
	double cameraHorizontalRotation = 0;
	double cameraVerticalRotation = 0;
	bool enableCameraLook = false;
	// Prefs
	double sensitivity = 0;
	double moveSpeed = 0;

	// Camera Look Direction and Vectors
	vec3 cameraDir;
	vec3 cameraRight;
	vec3 cameraUp;
	vec3 viewportCenter;
	// Viewport scale in world
	vec3 viewportU;
	vec3 viewportV;
	// Pixel positions
	vec3 viewportUpperLeft;
	vec3 pixelDeltaU;
	vec3 pixelDeltaV;
	vec3 pixel00Loc;

	void updateViewport(const int imageWidth, const int imageHeight)
	{
		cameraDir = -unitVector(vec3(sin(cameraHorizontalRotation) * cos(cameraVerticalRotation), sin(cameraVerticalRotation), cos(cameraHorizontalRotation) * cos(cameraVerticalRotation)));

		cameraRight = unitVector(cross(cameraDir, vec3(0, 1, 0)));
		cameraUp = cross(cameraRight, cameraDir);

		viewportCenter = cameraCenter + focalLength * cameraDir;

		viewportU = viewportWidth * cameraRight;
		viewportV = -viewportHeight * cameraUp;

		viewportUpperLeft = viewportCenter - viewportU / 2 - viewportV / 2;

		pixelDeltaU = viewportU / imageWidth;
		pixelDeltaV = viewportV / imageHeight;

		pixel00Loc = viewportUpperLeft + 0.5 * (pixelDeltaU + pixelDeltaV);
	}
};


colour rayColour(const ray& r, int depth, const hittable& world)
{
	if (depth <= 0)
	{
		return colour(0, 0, 0);
	}

	hitRecord rec;
	if (world.hit(r, interval(0.001, infinity), rec))
	{
		ray scattered;
		colour attenuation;
		if (rec.mat->scatter(r, rec, attenuation, scattered))
		{
			return attenuation * rayColour(scattered, depth - 1, world);
		}
		return colour(0, 0, 0);
	}

	vec3 unitDirection = unitVector(r.direction());
	auto a = 0.5 * (unitDirection.y() + 1.0);
	return (1.0 - a) * colour(1.0, 1.0, 1.0) + a * colour(0.5, 0.7, 1.0);
}

vec3 sampleSquare() 
{
	return vec3(randomDouble() - 0.5, randomDouble() - 0.5, 0);
}

ray getRay(int i, int j, const Camera& cam) 
{
	// Construct a camera ray originating from the origin and directed at randomly sampled
	// point around the pixel location i, j.

	auto offset = sampleSquare();
	auto pixelSample = cam.pixel00Loc + ((i + offset.x()) * cam.pixelDeltaU) + ((j + offset.y()) * cam.pixelDeltaV);

	auto rayOrigin = cam.cameraCenter;
	auto rayDirection = pixelSample - rayOrigin;

	return ray(rayOrigin, rayDirection);
}

inline double linearToGamma(double linearComponent)
{
	if (linearComponent > 0)
	{
		return sqrt(linearComponent);
	}

	return 0;
}

void renderRows(const int startX, const int endX, const int imageWidth, const int imageHeight, vector<unsigned char>& pixels, const Camera& cam, const hittable& world, int threadID)
{
	for (int j = 0; j < imageHeight; ++j)
	{
		for (int i = startX; i < endX; ++i)
		{
			int offset = (j * imageWidth + i) * 3;

			colour pixelColour(0, 0, 0);
			if (cam.enableAntialiasing)
			{
				for (int sample = 0; sample < cam.samplesPerPixel; sample++) 
				{
					ray r = getRay(i, j, cam);
					pixelColour += rayColour(r, cam.maxDepth, world);
				}

				pixelColour = cam.pixelSamplesScale * pixelColour;
			}
			else
			{
				auto pixelCenter = cam.pixel00Loc + (i * cam.pixelDeltaU) + (j * cam.pixelDeltaV);
				auto rayDirection = pixelCenter - cam.cameraCenter;
				ray r(cam.cameraCenter, rayDirection);

				pixelColour = rayColour(r, cam.maxDepth, world);
			}

			static const interval intensity(0.000, 0.999);
			pixels[offset]     = int(256 * intensity.clamp(linearToGamma(pixelColour.x())));
			pixels[offset + 1] = int(256 * intensity.clamp(linearToGamma(pixelColour.y())));
			pixels[offset + 2] = int(256 * intensity.clamp(linearToGamma(pixelColour.z())));
		}

		if (!cam.live)
		{
			cout << "Thread " << threadID << " " << j / (double)imageHeight * 100 << "% rows complete\n";
		}
	}

	if (!cam.live)
	{
		cout << startX / (double)imageWidth * 100 << "% to " << endX / (double)imageWidth * 100 << "% part of image complete! (Thread " << threadID << ")\n";
	}
}


void renderFrame(const int imageWidth, const int imageHeight, vector<unsigned char>& pixels, const Camera& cam, const hittable& world)
{
	int numThreads = thread::hardware_concurrency();
	int colsPerThread = imageWidth / numThreads;

	vector<thread> threads;

	for (int i = 0; i < numThreads; ++i)
	{
		int startX = i * colsPerThread;
		int endX = (i == numThreads - 1) ? imageWidth : startX + colsPerThread;

		threads.emplace_back(renderRows, startX, endX, imageWidth, imageHeight, ref(pixels), cref(cam), cref(world), i);
	}

	for (auto& thread : threads)
	{
		thread.join();
	}
}