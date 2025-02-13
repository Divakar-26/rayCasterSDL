
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <math.h>
#include<iostream>

#define PI 3.1415926
#define P2 PI/2
#define P3 3*PI/2
#define DR 0.0174533

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 512
#define CELL_SIZE 64
#define PLAYER_SIZE 20
#define PLAYER_SPEED 10
#define LINE_LENGTH 200


float playerX = WINDOW_HEIGHT / 2, playerY = WINDOW_WIDTH / 4, playerAngle = 0.0, playerDx = cos(playerAngle) * PLAYER_SPEED, playerDy = sin(playerAngle) * PLAYER_SPEED;

SDL_FRect player = { playerX, playerY , PLAYER_SIZE, PLAYER_SIZE };

int mapX = 8, mapY = 8, mapSize = 64;
int map[] = {
	1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 ,
	1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 ,
	1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 ,
	1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 ,
	1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 ,
	1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 ,
	1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 ,
	1 , 1 , 1 , 1 , 1 , 1 , 1 , 1
};

void drawPlayer(SDL_Renderer* ren, SDL_FRect* player) {
	SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
	SDL_RenderFillRect(ren, player);

	float lineEndX = player->x + PLAYER_SIZE / 2 + LINE_LENGTH * cos(playerAngle);
	float lineEndY = player->y + PLAYER_SIZE / 2 + LINE_LENGTH * sin(playerAngle);

	SDL_SetRenderDrawColor(ren, 0, 255, 0, 255); 	SDL_RenderLine(ren, player->x + PLAYER_SIZE / 2, player->y + PLAYER_SIZE / 2, lineEndX, lineEndY);
}

void movePlayer(SDL_Renderer* ren, SDL_FRect* rect, SDL_Event event) {
	if (event.type == SDL_EVENT_KEY_DOWN) {
		switch (event.key.key) {
		case SDLK_W: { player.y += playerDy; player.x += playerDx; break; }
		case SDLK_S: { player.y -= playerDy; player.x -= playerDx; break; }
		case SDLK_A: { playerAngle += 0.1; if (playerAngle > 2 * PI) { playerAngle -= 2 * PI; } playerDx = cos(playerAngle) * PLAYER_SPEED; playerDy = sin(playerAngle) * PLAYER_SPEED; break; }
		case SDLK_D: { playerAngle -= 0.1; if (playerAngle < 0) { playerAngle += 2 * PI; } playerDx = cos(playerAngle) * PLAYER_SPEED; playerDy = sin(playerAngle) * PLAYER_SPEED; break; }
		}
	}
}

void movePlayerCursor(SDL_Event event) {
	if (event.type == SDL_EVENT_MOUSE_MOTION) {
		float mouseX, mouseY;
		SDL_GetMouseState(&mouseX, &mouseY);


		float deltaX = mouseX - (player.x + PLAYER_SIZE / 2);
		float deltaY = mouseY - (player.y + PLAYER_SIZE / 2);


		playerAngle = atan2(deltaY, deltaX);

		playerDx = cos(playerAngle) * PLAYER_SPEED;
		playerDy = sin(playerAngle) * PLAYER_SPEED;

		//std::cout << mouseX << " " << mouseY << " " << playerDx << " " << playerDy << std::endl;
	}
}

void editMap() {
	float mouseX, mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);

	if (mouseX < 512 && mouseY < 512) {
		int x = (int)mouseX / 64, y = (int)mouseY / 64;
		int index = y * 8 + x;
		if (map[index] == 0) {
			map[index] = 1;
		}
		else map[index] = 0;
	}
}

float dist(float ax, float ay, float bx, float by, float ang) {
	return (sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay)));
}

void castRays(SDL_Renderer* ren, SDL_FRect* player) {
	int dof, mx, my, mp;
	float ra, firstRy, firstRx, xOffSet, yOffSet, distT;
	ra = playerAngle - 30 * DR;


	if (ra < 0) ra += 2 * PI;
	if (ra > 2 * PI) ra -= 2 * PI;

	for (int i = 0; i < 60; i++) {
		dof = 0;
		float distH = 512, hx = player->x, hy = player->y;
		if (ra > PI) {
			firstRy = (((int)player->y / 64) * 64) - 0.0001;
			firstRx = player->x + (firstRy - player->y) / tan(ra);
			yOffSet = -64;
			xOffSet = yOffSet / tan(ra);
		}
		else if (ra < PI) {
			firstRy = (((int)player->y / 64) * 64) + 64;
			firstRx = player->x + (firstRy - player->y) / tan(ra);
			yOffSet = 64;
			xOffSet = yOffSet / tan(ra);
		}
		if (ra == 0 || ra == PI) {
			firstRx = player->x;
			firstRy = player->y;
			dof = 8;
		}
		while (dof < 8) {
			mx = (int)(firstRx) >> 6;
			my = (int)(firstRy) >> 6;
			mp = my * mapX + mx;
			if (mp > 0 && mp < mapX * mapY && map[mp] == 1) { hx = firstRx; hy = firstRy; distH = dist(player->x, player->y, hx, hy, ra); dof = 8; }
			else {
				firstRx += xOffSet;
				firstRy += yOffSet;
				dof += 1;
			}
		}

		dof = 0;
		float distV = 512, vx = player->x, vy = player->y;
		if (ra > P2 && ra < P3) {
			firstRx = (((int)player->x / 64) * 64) - 0.0001;
			firstRy = player->y + (player->x - firstRx) * (-tan(ra));
			xOffSet = -64;
			yOffSet = -xOffSet * -tan(ra);
		}
		else if (ra < P2 || ra > P3) {
			firstRx = (((int)player->x / 64) * 64) + 64;
			firstRy = player->y + (player->x - firstRx) * (-tan(ra));
			xOffSet = 64;
			yOffSet = -xOffSet * -tan(ra);
		}
		if (ra == 0 || ra == PI) {
			firstRx = player->x;
			firstRy = player->y;
			dof = 8;
		}
		while (dof < 8) {
			mx = (int)(firstRx) >> 6;
			my = (int)(firstRy) >> 6;
			mp = my * mapX + mx;
			if (mp > 0 && mp < mapX * mapY && map[mp] == 1) { vx = firstRx; vy = firstRy; distV = dist(player->x, player->y, vx, vy, ra); dof = 8; }
			else {
				firstRx += xOffSet;
				firstRy += yOffSet;
				dof += 1;
			}
		}

		if (distV < distH) {
			firstRx = vx; firstRy = vy; distT = distV; 
			SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
		}
		if (distV > distH) {
			firstRx = hx; firstRy = hy; distT = distH;	
			SDL_SetRenderDrawColor(ren, 200, 0, 0, 0);
		}

		if (firstRx < 0 || firstRy < 0 || firstRx >= 512 || firstRy >= 512) {
			continue;
		}

		SDL_RenderLine(ren, player->x + PLAYER_SIZE / 2, player->y + PLAYER_SIZE / 2, firstRx, firstRy);

		float ca = playerAngle - ra;
		if (ca < 0) ca += 2 * PI;
		if (ca > 2 * PI) ca -= 2 * PI;
		distT *= cos(ca);

		int lineH = (mapSize * 320) / distT;
		if (lineH > 320) lineH = 320;
		int lineO = 160 - lineH / 2;

		SDL_FRect rect = { (float)(i * 8 + 530), (float)(lineO + 100), 8.0f, (float)lineH };
		SDL_RenderFillRect(ren, &rect);

		ra += DR;
		if (ra < 0) ra += 2 * PI;
		if (ra > 2 * PI) ra -= 2 * PI;
	}
}

void drawGrid(SDL_Renderer* ren, int w, int h, int cellsize, int* map) {
	for (int x = 0; x < w; x += cellsize) {
		for (int y = 0; y < h; y += cellsize) {

			int index = (y / cellsize) * (w / cellsize) + (x / cellsize);

			if (map[index] == 1) {
				SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
				SDL_FRect cell = { (float)x, (float)y, (float)(cellsize - 2), (float)(cellsize - 2 )};
				SDL_RenderFillRect(ren, &cell);
			}
			else if (map[index] == 0) {
				SDL_SetRenderDrawColor(ren, 0, 0, 0, 0);
				SDL_FRect cell = { (float)x, (float)y, (float)(cellsize - 1), (float)(cellsize - 1) };
				SDL_RenderFillRect(ren, &cell);
			}
		}
	}
}


int main(int argc, char* argv[]) {
	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window* window = SDL_CreateWindow("RayCaster", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);



	bool running = true;
	SDL_Event event;
	while (running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				running = false;
			}
			else if (event.type == SDL_EVENT_KEY_DOWN) {
				movePlayer(renderer, &player, event);
			}
			else if (event.type == SDL_EVENT_MOUSE_MOTION) {
				movePlayerCursor(event);
			}
			else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
				editMap();
			}
		}
		SDL_SetWindowSize(window, WINDOW_WIDTH, WINDOW_HEIGHT);
		SDL_SetRenderDrawColor(renderer, 92, 112, 127, 0.8);
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