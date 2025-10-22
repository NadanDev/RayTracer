#include "rtweekend.h"

#include <vector>
#include <chrono>
#include <thread>
#include <SDL.h>

#include "hittable.h"
#include "hittableList.h"
#include "sphere.h"
#include "camera.h"
#include "input.h"


int main(int argc, char* argv[])
{
	// Image Settings
	auto aspectRatio = 16.0 / 9.0;
	int imageWidth = 400;
	// Image Height : Must be at least 1
	int imageHeight = int(imageWidth / aspectRatio);
	imageHeight = (imageHeight < 1) ? 1 : imageHeight;

	// World
	hittableList world;

	world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
	world.add(make_shared<sphere>(point3(0, -100.5, -1), 100));

	Camera cam;
	// Initial values (Changeable)
	cam.FOV = 45;
	cam.sensitivity = 1;
	cam.moveSpeed = 3;
	cam.enableCameraLook = false;
	cam.enableAntialiasing = true;
	cam.samplesPerPixel = 100;
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

	while (!quit)
	{
		auto start = chrono::high_resolution_clock::now();

		// RENDER FRAME
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
		cam.updateViewport(imageWidth, imageHeight);


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