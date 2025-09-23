#include <iostream>
#include <vector>
#include <chrono>
#include <SDL.h>

#include "vec3.h"
#include "ray.h"
#include "colour.h"

using namespace std;

double hit_sphere(const point3& center, double radius, const ray& r)
{
	vec3 oc = center - r.origin();
	auto a = dot(r.direction(), r.direction());
	auto b = -2.0 * dot(r.direction(), oc);
	auto c = dot(oc, oc) - radius * radius;
	auto discriminant = b * b - 4 * a * c;

	if (discriminant < 0)
	{
		return -1.0;
	}
	else
	{
		return (-b - sqrt(discriminant)) / (2.0 * a);
	}
}

colour rayColour(const ray& r)
{
	auto t = hit_sphere(point3(0, 0, -1), 0.5, r);
	if (t > 0.0)
	{
		vec3 N = unitVector(r.at(t) - vec3(0, 0, -1));
		return 0.5 * colour(N.x() + 1, N.y() + 1, N.z() + 1);
	}

	vec3 unitDirection = unitVector(r.direction());
	auto a = 0.5 * (unitDirection.y() + 1.0); // a must be between 0 and 1 instead of -1 to 1
	return (1.0 - a) * colour(1.0, 1.0, 1.0) + a * colour(0.5, 0.7, 1.0); // Lerp
}

int main(int argc, char* argv[])
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
	double viewportXPos = 0;
	double viewportYPos = 0;
	double viewportZPos = focalLength;
	auto viewportUpperLeft = cameraCenter - vec3(viewportXPos, viewportYPos, viewportZPos) - viewportU / 2 - viewportV / 2;
	auto pixel00Loc = viewportUpperLeft + 0.5 * (pixelDeltaU + pixelDeltaV); // Middle

	double cameraHorizontalRotation = 0;
	double cameraVerticalRotation = 0;


	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		cerr << "Could not initialize SDL: " << SDL_GetError() << "\n";
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("Ray Tracer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, imageWidth, imageHeight, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, imageWidth, imageHeight);

	// Render
	vector<unsigned char> pixels(imageWidth * imageHeight * 3);

	SDL_Event event;
	bool quit = false;
	while (!quit)
	{
		auto start = chrono::high_resolution_clock::now();

		for (int j = 0; j < imageHeight; j++)
		{
			for (int i = 0; i < imageWidth; i++)
			{
				int offset = (j * imageWidth + i) * 3;

				auto pixelCenter = pixel00Loc + (i * pixelDeltaU) + (j * pixelDeltaV);
				auto rayDirection = pixelCenter - cameraCenter;
				ray r(cameraCenter, rayDirection);

				colour pixelColour = rayColour(r);

				pixels[offset] = int(255.999 * pixelColour.x());
				pixels[offset + 1] = int(255.999 * pixelColour.y());
				pixels[offset + 2] = int(255.999 * pixelColour.z());
			}
		}

		SDL_UpdateTexture(texture, nullptr, pixels.data(), imageWidth * 3);

		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, nullptr, nullptr);
		SDL_RenderPresent(renderer);

		auto end = chrono::high_resolution_clock::now();
		chrono::duration<double> timeElapsed = end - start;
		double fps = 1 / timeElapsed.count();
		//cout << fps << "\n";


		const Uint8* state = SDL_GetKeyboardState(nullptr);
		if (state[SDL_SCANCODE_W])
		{
			cameraCenter += point3(0, 0, -0.1);
		}
		if (state[SDL_SCANCODE_A])
		{
			cameraCenter += point3(-0.1, 0, 0);
		}
		if (state[SDL_SCANCODE_S])
		{
			cameraCenter += point3(0, 0, 0.1);
		}
		if (state[SDL_SCANCODE_D])
		{
			cameraCenter += point3(0.1, 0, 0);
		}

		if (state[SDL_SCANCODE_UP])
		{
			cameraVerticalRotation -= 0.1;
		}
		if (state[SDL_SCANCODE_LEFT])
		{
			cameraHorizontalRotation += 0.1;
		}
		if (state[SDL_SCANCODE_DOWN])
		{
			cameraVerticalRotation += 0.1;
		}
		if (state[SDL_SCANCODE_RIGHT])
		{
			cameraHorizontalRotation -= 0.1;
		}
		viewportXPos = sin(cameraHorizontalRotation) * focalLength;
		viewportYPos = sin(cameraVerticalRotation) * focalLength;
		//viewportZPos = ((cos(cameraVerticalRotation) * focalLength) * (cos(cameraHorizontalRotation) * focalLength));
		viewportZPos = ((cos(cameraHorizontalRotation) * focalLength));

		cout << "X: " << viewportXPos << "\n";
		cout << "Y: " << viewportYPos << "\n";
		cout << "Z: " << viewportZPos << "\n";

		viewportUpperLeft = cameraCenter - vec3(viewportXPos, viewportYPos, viewportZPos) - viewportU / 2 - viewportV / 2;
		pixel00Loc = viewportUpperLeft + 0.5 * (pixelDeltaU + pixelDeltaV); // Middle


		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				quit = true;
			}
		}
	}

	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}