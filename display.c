#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "./display.h"

#define PIXEL_SIZE 10  // Each CHIP-8 pixel becomes a 10x10 square


void clear(Display* display) {
	for(int i = 0; i < DISPLAY_HEIGHT * DISPLAY_WIDTH; i++) {
		display->current_frame[i] = 0;
		display->previous_frame[i] = -1;
	}
}

int draw (int x, int y, uint8_t* sprite, int number_of_bytes, Display* display) {
	int bit_counter = 0;
	uint8_t collision = 0;
	
	for(int counter = 0; counter < number_of_bytes; counter++) {

		int start = (y + counter ) * DISPLAY_WIDTH + x;
		int end = start + 8;

		
		for(int display_counter = start; display_counter < end; display_counter++){
			uint8_t sprite_pixel = sprite[bit_counter];

			if (sprite_pixel == 1) {
				if (display->current_frame[display_counter] == 1) {
					collision = 1;
				}
				display->current_frame[display_counter] ^= 1;
			}

			bit_counter++;
		}
	}
	return collision;		
}


void render(Display* display) {
	char buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT * 16]; 
	int offset = 0;

	for(int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
		if(display->current_frame[i] != display->previous_frame[i]) {
			int pixel = display->current_frame[i];
			int y = i / DISPLAY_WIDTH + 10; 
			int x = i % DISPLAY_WIDTH + 10;  

			if (pixel) {
				offset += snprintf(buffer + offset, sizeof(buffer) - offset,
								   "\033[%d;%dH\033[47m \033[0m", y, x); // white block
			} else {
				offset += snprintf(buffer + offset, sizeof(buffer) - offset,
								   "\033[%d;%dH ", y, x); 
			}
			display->previous_frame[i] = pixel;
		}
	}

	// render batch
	fwrite(buffer, 1, offset, stdout);
	fflush(stdout); 
}

void render_sdl(Display* display) {

	SDL_SetRenderDrawColor(display->renderer, 0, 0, 0, 255); // black
    SDL_RenderClear(display->renderer);

    SDL_Rect pixel_rect;
    pixel_rect.w = PIXEL_SIZE;
    pixel_rect.h = PIXEL_SIZE;

	for(int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
		if(display->current_frame[i] != display->previous_frame[i]) {
			int pixel = display->current_frame[i];
			int y = i / DISPLAY_WIDTH;
			int x = i % DISPLAY_WIDTH;

				if (pixel) {
				SDL_SetRenderDrawColor(display->renderer, 255, 255, 255, 255); // white
			} else {
				continue; // Skip drawing black pixels, background already cleared
			}

			pixel_rect.x = x * PIXEL_SIZE;
			pixel_rect.y = y * PIXEL_SIZE;
			SDL_RenderFillRect(display->renderer, &pixel_rect);

		}
	}

	SDL_RenderPresent(display->renderer);
}


void cleanup(Display* display) {
	 SDL_DestroyRenderer(display->renderer);
	 SDL_DestroyWindow(display->window);
	 SDL_Quit();
}

Display* get_display() {
  Display* display = malloc(sizeof *display);

  // set initial state of the current frame and previous one
  for(int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
	display->current_frame[i] = 0;
	display->previous_frame[i] = -1;
  }

  // support for SDL
  SDL_Init(SDL_INIT_VIDEO);

  SDL_Window *window = SDL_CreateWindow("Basic C SDL project",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT,
                                          SDL_WINDOW_SHOWN);

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  display->renderer = renderer;	
  display->window = window;


  display->clear = *clear;
  display->draw = *draw;
  display->render = *render;
  display->quit = false;
  return display;
}