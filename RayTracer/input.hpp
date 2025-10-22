#pragma once

void inputHandler(Camera& cam, const double deltaTime, const double sensitivity, const double moveSpeed)
{
	const Uint8* state = SDL_GetKeyboardState(nullptr);
	int x;
	int y;
	SDL_GetRelativeMouseState(&x, &y);

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
	if (cam.enableCameraLook)
	{
		cam.cameraHorizontalRotation -= x * sensitivity * deltaTime;
		cam.cameraVerticalRotation += y * sensitivity * deltaTime;
	}

	if (state[SDL_SCANCODE_ESCAPE])
	{
		SDL_SetRelativeMouseMode(SDL_FALSE);
	}
	if (state[SDL_SCANCODE_DELETE])
	{
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}
}