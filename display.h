#ifndef CHIP8_DISPLAY
#define CHIP8_DISPLAY
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
// Define MAX and MIN macros
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

// Define screen dimensions
#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600


typedef struct Display {
	int previous_frame[DISPLAY_WIDTH * DISPLAY_HEIGHT];
	int current_frame[DISPLAY_WIDTH * DISPLAY_HEIGHT];
	SDL_Renderer* renderer;
	SDL_Window* window;
	bool quit;
	void (*clear)(struct Display*);
	int (*draw)(int x, int y, uint8_t* sprite, int number_of_bytes, struct Display* display);
	void (*render)(struct Display*);
	void(*cleanup)(struct Display*);
} Display;

void clear(Display*);
int draw(int x, int y, uint8_t* sprite, int number_of_bytes, Display* display);
void render(Display*);
Display* get_display();
void render_sdl(Display* display);
void cleanup(Display* display);

#endif