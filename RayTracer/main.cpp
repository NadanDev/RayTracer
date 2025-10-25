#include "rtweekend.h"

#include <vector>
#include <chrono>
#include <thread>
#include <SDL.h>

#include "hittable.h"
#include "hittableList.h"
#include "sphere.h"
#include "camera.hpp"
#include "input.hpp"
#include "material.h"


int main(int argc, char* argv[])
{
	// Image Settings
	auto aspectRatio = 16.0 / 9.0;
	int imageWidth = 500;
	// Image Height : Must be at least 1
	int imageHeight = int(imageWidth / aspectRatio);
	imageHeight = (imageHeight < 1) ? 1 : imageHeight;

	// World
	hittableList world;

	auto materialGround = make_shared<lambertian>(colour(0.8, 0.8, 0.0));
	auto materialCenter = make_shared<lambertian>(colour(0.1, 0.2, 0.5));
	auto materialLeft = make_shared<dielectric>(1.50);
	auto materialBubble = make_shared<dielectric>(1.00 / 1.50);
	auto materialRight = make_shared<metal>(colour(0.8, 0.6, 0.2), 0.3);

	world.add(make_shared<sphere>(point3(0.0, -100.5, -1.0), 100.0, materialGround));
	world.add(make_shared<sphere>(point3(0.0, 0.0, -1.2), 0.5, materialCenter));
	world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.5, materialLeft));
	world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.4, materialBubble));
	world.add(make_shared<sphere>(point3(1.0, 0.0, -1.0), 0.5, materialRight));

	Camera cam;
	// Initial values (Changeable)
	cam.live = false;
	cam.cameraCenter = point3(0, 0, 2);
	cam.FOV = 45;
	cam.sensitivity = 1;
	cam.moveSpeed = 3;
	cam.enableCameraLook = false;
	cam.enableAntialiasing = true;
	cam.samplesPerPixel = 50;
	cam.pixelSamplesScale = 1.0 / cam.samplesPerPixel;
	cam.maxDepth = 50;
	// Calculations for intial values
	cam.viewportHeight = tan(degreesToRadians(cam.FOV) / 2) * 2 * cam.focalLength;
	cam.viewportWidth = cam.viewportHeight * (double(imageWidth) / imageHeight);


	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		cerr << "Could not initialize SDL: " << SDL_GetError() << "\n";
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("Ray Tracer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_SHOWN); // 720p window (render resolution lower)
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, imageWidth, imageHeight);
	SDL_SetRelativeMouseMode(SDL_TRUE); // Enable mouse control

	// Render
	vector<unsigned char> pixels(imageWidth * imageHeight * 3);
	SDL_Event event;
	bool quit = false;

	do
	{
		auto start = chrono::high_resolution_clock::now();

		// RENDER FRAME
		cam.updateViewport(imageWidth, imageHeight); // Must be run first to initialize viewport

		renderFrame(imageWidth, imageHeight, pixels, cam, world);
		SDL_UpdateTexture(texture, nullptr, pixels.data(), imageWidth * 3);

		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, nullptr, nullptr);
		SDL_RenderPresent(renderer);

		// FPS
		auto end = chrono::high_resolution_clock::now();
		double deltaTime = (chrono::duration<double>(end - start)).count();
		double fps = 1 / deltaTime;
		cout << fps << "\n";

		// Player Input
		inputHandler(cam, deltaTime, cam.sensitivity, cam.moveSpeed);


		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				quit = true;
			}
		}
	} while (!quit && cam.live);

	cout << "Done!\n";

	// Required to fix issue of no image on long renders
	SDL_UpdateTexture(texture, nullptr, pixels.data(), imageWidth * 3);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, nullptr, nullptr);
	SDL_RenderPresent(renderer);

	while (!quit) // Keep image on screen
	{
		inputHandler(cam, 0, cam.sensitivity, cam.moveSpeed); // To enable escape

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