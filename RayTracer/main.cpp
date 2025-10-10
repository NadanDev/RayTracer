#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <SDL.h>

#include "vec3.h"
#include "ray.h"
#include "colour.h"

using namespace std;


struct Camera 
{
	// Camera Settings
	point3 cameraCenter = point3(0, 0, 0);
	double FOV = 0;
	double focalLength = 1;
	double viewportHeight = 0;
	double viewportWidth = 0;

	// Live rotation of camera
	double cameraHorizontalRotation = 0;
	double cameraVerticalRotation = 0;
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

void inputHandler(Camera& cam, const double deltaTime, const double sensitivity, const double moveSpeed)
{
	const Uint8* state = SDL_GetKeyboardState(nullptr);

	// Movement
	if (state[SDL_SCANCODE_W])
	{
		cam.cameraCenter += cam.cameraDir * moveSpeed * deltaTime;
	}
	if (state[SDL_SCANCODE_A])
	{
		cam.cameraCenter += -cam.cameraRight * moveSpeed * deltaTime;
	}
	if (state[SDL_SCANCODE_S])
	{
		cam.cameraCenter += -cam.cameraDir * moveSpeed * deltaTime;
	}
	if (state[SDL_SCANCODE_D])
	{
		cam.cameraCenter += cam.cameraRight * moveSpeed * deltaTime;
	}
	if (state[SDL_SCANCODE_SPACE])
	{
		cam.cameraCenter += vec3(0, moveSpeed * deltaTime, 0);
	}
	if (state[SDL_SCANCODE_LSHIFT])
	{
		cam.cameraCenter += vec3(0, -moveSpeed * deltaTime, 0);
	}

	// Rotation
	if (state[SDL_SCANCODE_UP])
	{
		cam.cameraVerticalRotation -= sensitivity * deltaTime;
	}
	if (state[SDL_SCANCODE_LEFT])
	{
		cam.cameraHorizontalRotation += sensitivity * deltaTime;
	}
	if (state[SDL_SCANCODE_DOWN])
	{
		cam.cameraVerticalRotation += sensitivity * deltaTime;
	}
	if (state[SDL_SCANCODE_RIGHT])
	{
		cam.cameraHorizontalRotation -= sensitivity * deltaTime;
	}
}

void renderRows(const int startY, const int endY, const int imageWidth, const int imageHeight, vector<unsigned char>& pixels, const Camera& cam)
{
	for (int j = startY; j < endY; ++j)
	{
		for (int i = 0; i < imageWidth; ++i)
		{
			int offset = (j * imageWidth + i) * 3;

			auto pixelCenter = cam.pixel00Loc + (i * cam.pixelDeltaU) + (j * cam.pixelDeltaV);
			auto rayDirection = pixelCenter - cam.cameraCenter;
			ray r(cam.cameraCenter, rayDirection);

			colour pixelColour = rayColour(r);

			pixels[offset] = int(255.999 * pixelColour.x());
			pixels[offset + 1] = int(255.999 * pixelColour.y());
			pixels[offset + 2] = int(255.999 * pixelColour.z());
		}
	}
}

void renderFrame(const int imageWidth, const int imageHeight, vector<unsigned char>& pixels, const Camera& cam)
{
	int numThreads = thread::hardware_concurrency();
	int rowsPerThread = imageHeight / numThreads;
	
	vector<thread> threads;

	for (int i = 0; i < numThreads; ++i)
	{
		int startY = i * rowsPerThread;
		int endY = (i == numThreads - 1) ? imageHeight : startY + rowsPerThread;

		threads.emplace_back(renderRows, startY, endY, imageWidth, imageHeight, ref(pixels), cref(cam));
	}

	for (auto& thread : threads)
	{
		thread.join();
	}
}


int main(int argc, char* argv[])
{
	// Image Settings
	auto aspectRatio = 16.0 / 9.0;
	int imageWidth = 1000;
	// Image Height : Must be at least 1
	int imageHeight = int(imageWidth / aspectRatio);
	imageHeight = (imageHeight < 1) ? 1 : imageHeight;

	Camera cam;
	// Initial values (Changeable)
	cam.FOV = 45;
	cam.sensitivity = 3;
	cam.moveSpeed = 3;
	// Calculations for intial values
	cam.viewportHeight = tan((cam.FOV * M_PI / 180.0) / 2) * 2 * cam.focalLength;
	cam.viewportWidth = cam.viewportHeight * (double(imageWidth) / imageHeight);

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

		// RENDER FRAME
		renderFrame(imageWidth, imageHeight, pixels, cam);
		SDL_UpdateTexture(texture, nullptr, pixels.data(), imageWidth * 3);

		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, nullptr, nullptr);
		SDL_RenderPresent(renderer);

		auto end = chrono::high_resolution_clock::now();
		double deltaTime = (chrono::duration<double>(end - start)).count();
		double fps = 1 / deltaTime;
		//cout << fps << "\n";

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