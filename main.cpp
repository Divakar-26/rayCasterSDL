#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <math.h>
#include <iostream>

#define PI 3.1415926
#define P2 PI / 2
#define P3 3 * PI / 2
#define DR 0.0174533

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 512
#define CELL_SIZE 64
#define PLAYER_SIZE 20
#define PLAYER_SPEED 3
#define LINE_LENGTH 200

float playerAngle = 0.0, playerDx = cos(playerAngle) * PLAYER_SPEED, playerDy = sin(playerAngle) * PLAYER_SPEED;

float playerX = WINDOW_WIDTH / 4;
float playerY = WINDOW_HEIGHT / 2;
SDL_FRect player = {
	playerX - PLAYER_SIZE / 2,
	playerY - PLAYER_SIZE / 2,
	PLAYER_SIZE,
	PLAYER_SIZE};

int mapX = 8, mapY = 8;
int map[] = {
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 1, 1, 1, 1, 1, 1, 1};

bool isCollidingWithWall(float newX, float newY)
{
	// Offsets to check all 4 corners
	float halfSize = PLAYER_SIZE / 2.0;

	float cornersX[4] = {newX - halfSize, newX + halfSize - 1, newX - halfSize, newX + halfSize - 1};
	float cornersY[4] = {newY - halfSize, newY - halfSize, newY + halfSize - 1, newY + halfSize - 1};

	for (int i = 0; i < 4; ++i)
	{
		int gridX = (int)(cornersX[i]) / CELL_SIZE;
		int gridY = (int)(cornersY[i]) / CELL_SIZE;

		if (gridX < 0 || gridY < 0 || gridX >= mapX || gridY >= mapY)
			return true;

		int index = gridY * mapX + gridX;
		if (map[index] == 1)
			return true;
	}
	return false;
}

void drawPlayer(SDL_Renderer *ren, SDL_FRect *player)
{
	SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
	SDL_RenderFillRect(ren, player);

	float lineEndX = playerX + LINE_LENGTH * cos(playerAngle);
	float lineEndY = playerY + LINE_LENGTH * sin(playerAngle);

	SDL_SetRenderDrawColor(ren, 0, 255, 0, 255);
	SDL_RenderLine(ren, playerX, playerY, lineEndX, lineEndY);
}

void movePlayer(SDL_Renderer *ren, SDL_FRect *rect, SDL_Event event)
{
	if (event.type == SDL_EVENT_KEY_DOWN)
	{
		float newX = playerX;
		float newY = playerY;

		switch (event.key.key)
		{
		case SDLK_W:
			newX += playerDx;
			newY += playerDy;
			break;
		case SDLK_S:
			newX -= playerDx;
			newY -= playerDy;
			break;
		case SDLK_A:
			playerAngle += 0.1;
			if (playerAngle > 2 * PI)
				playerAngle -= 2 * PI;
			playerDx = cos(playerAngle) * PLAYER_SPEED;
			playerDy = sin(playerAngle) * PLAYER_SPEED;
			break;
		case SDLK_D:
			playerAngle -= 0.1;
			if (playerAngle < 0)
				playerAngle += 2 * PI;
			playerDx = cos(playerAngle) * PLAYER_SPEED;
			playerDy = sin(playerAngle) * PLAYER_SPEED;
			break;
		}

		if (!isCollidingWithWall(newX, newY))
		{
			playerX = newX;
			playerY = newY;
			player.x = playerX - PLAYER_SIZE / 2;
			player.y = playerY - PLAYER_SIZE / 2;
		}
	}
}

void movePlayerCursor(SDL_Event event)
{
	if (event.type == SDL_EVENT_MOUSE_MOTION)
	{
		float mouseX, mouseY;
		SDL_GetMouseState(&mouseX, &mouseY);

		float deltaX = mouseX - playerX;
		float deltaY = mouseY - playerY;

		playerAngle = atan2(deltaY, deltaX);
		playerDx = cos(playerAngle) * PLAYER_SPEED;
		playerDy = sin(playerAngle) * PLAYER_SPEED;
	}
}

void editMap()
{
	float mouseX, mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);

	if (mouseX < 512 && mouseY < 512)
	{
		int x = (int)mouseX / CELL_SIZE, y = (int)mouseY / CELL_SIZE;
		int index = y * 8 + x;
		map[index] = !map[index];
	}
}

float dist(float ax, float ay, float bx, float by, float ang)
{
	return sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay));
}

void castRays(SDL_Renderer *ren, SDL_FRect *player)
{
	int dof, mx, my, mp;
	float ra, rx, ry, xOff, yOff, distT;

	ra = playerAngle - 30 * DR;
	if (ra < 0)
		ra += 2 * PI;
	if (ra > 2 * PI)
		ra -= 2 * PI;

	for (int i = 0; i < 60; i++)
	{
		dof = 0;
		float distH = 1e9, hx = playerX, hy = playerY;

		float aTan = -1 / tan(ra);
		if (ra > PI)
		{
			ry = (((int)playerY >> 6) << 6) - 0.0001;
			rx = (playerY - ry) * aTan + playerX;
			yOff = -64;
			xOff = -yOff * aTan;
		}
		else if (ra < PI)
		{
			ry = (((int)playerY >> 6) << 6) + 64;
			rx = (playerY - ry) * aTan + playerX;
			yOff = 64;
			xOff = -yOff * aTan;
		}
		else
		{
			rx = playerX;
			ry = playerY;
			dof = 8;
		}

		while (dof < 8)
		{
			mx = (int)(rx) >> 6;
			my = (int)(ry) >> 6;
			mp = my * mapX + mx;
			if (mp > 0 && mp < mapX * mapY && map[mp] == 1)
			{
				hx = rx;
				hy = ry;
				distH = dist(playerX, playerY, hx, hy, ra);
				dof = 8;
			}
			else
			{
				rx += xOff;
				ry += yOff;
				dof++;
			}
		}

		dof = 0;
		float distV = 1e9, vx = playerX, vy = playerY;
		float nTan = -tan(ra);
		if (ra > P2 && ra < P3)
		{
			rx = (((int)playerX >> 6) << 6) - 0.0001;
			ry = (playerX - rx) * nTan + playerY;
			xOff = -64;
			yOff = -xOff * nTan;
		}
		else if (ra < P2 || ra > P3)
		{
			rx = (((int)playerX >> 6) << 6) + 64;
			ry = (playerX - rx) * nTan + playerY;
			xOff = 64;
			yOff = -xOff * nTan;
		}
		else
		{
			rx = playerX;
			ry = playerY;
			dof = 8;
		}

		while (dof < 8)
		{
			mx = (int)(rx) >> 6;
			my = (int)(ry) >> 6;
			mp = my * mapX + mx;
			if (mp > 0 && mp < mapX * mapY && map[mp] == 1)
			{
				vx = rx;
				vy = ry;
				distV = dist(playerX, playerY, vx, vy, ra);
				dof = 8;
			}
			else
			{
				rx += xOff;
				ry += yOff;
				dof++;
			}
		}

		if (distV < distH)
		{
			rx = vx;
			ry = vy;
			distT = distV;
			SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
		}
		else
		{
			rx = hx;
			ry = hy;
			distT = distH;
			SDL_SetRenderDrawColor(ren, 200, 0, 0, 255);
		}

		SDL_RenderLine(ren, playerX, playerY, rx, ry);

		float ca = playerAngle - ra;
		if (ca < 0)
			ca += 2 * PI;
		if (ca > 2 * PI)
			ca -= 2 * PI;
		distT *= cos(ca);

		int lineH = (CELL_SIZE * 320) / (distT + 0.0001);
		if (lineH > 320)
			lineH = 320;
		int lineO = 160 - (lineH >> 1);

		SDL_FRect wallStrip = {(float)(i * 8 + 530), (float)(lineO + 100), 8.0f, (float)lineH};
		SDL_RenderFillRect(ren, &wallStrip);

		ra += DR;
		if (ra < 0)
			ra += 2 * PI;
		if (ra > 2 * PI)
			ra -= 2 * PI;
	}
}

void drawGrid(SDL_Renderer *ren, int w, int h, int cellsize, int *map)
{
	for (int y = 0; y < h; y += cellsize)
	{
		for (int x = 0; x < w; x += cellsize)
		{
			int index = (y / cellsize) * (w / cellsize) + (x / cellsize);
			if (map[index] == 1)
				SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
			else
				SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);

			SDL_FRect cell = {(float)x, (float)y, (float)(cellsize - 1), (float)(cellsize - 1)};
			SDL_RenderFillRect(ren, &cell);
		}
	}
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window *window = SDL_CreateWindow("RayCaster", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

	bool running = true;
	SDL_Event event;
	while (running)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
				running = false;
			else if (event.type == SDL_EVENT_KEY_DOWN)
				movePlayer(renderer, &player, event);
			else if (event.type == SDL_EVENT_MOUSE_MOTION)
				movePlayerCursor(event);
			else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
				editMap();
		}

		player.x = playerX - PLAYER_SIZE / 2;
		player.y = playerY - PLAYER_SIZE / 2;

		SDL_SetWindowSize(window, WINDOW_WIDTH, WINDOW_HEIGHT);
		SDL_SetRenderDrawColor(renderer, 92, 112, 127, 255);
		SDL_RenderClear(renderer);
		drawGrid(renderer, 512, 512, CELL_SIZE, map);
		drawPlayer(renderer, &player);
		castRays(renderer, &player);
		SDL_RenderPresent(renderer);
		SDL_Delay(16);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
