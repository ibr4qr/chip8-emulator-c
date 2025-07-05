#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <SDL2/SDL.h>

#include "./keyboard.h"

int KeyboardMap[512];

// keypad mapping
void setup_keyboard() {
    memset(KeyboardMap, 0xFF, sizeof(int) * 512); // SDL keycodes are >255

    // SDL_Keycode values, e.g., SDLK_1, SDLK_q, etc.
    KeyboardMap[53] = 0x1;
    KeyboardMap[54] = 0x2;
    KeyboardMap[55] = 0x3;
    KeyboardMap[56] = 0xC;
    KeyboardMap[114] = 0x4;
    KeyboardMap[116] = 0x5;
    KeyboardMap[121] = 0x6;
    KeyboardMap[117] = 0xD;
    KeyboardMap[102] = 0x7;
    KeyboardMap[103] = 0x8;
    KeyboardMap[104] = 0x9;
    KeyboardMap[106] = 0xE;
    KeyboardMap[118] = 0xA;
    KeyboardMap[98] = 0x0;
    KeyboardMap[110] = 0xB;
    KeyboardMap[109] = 0xF;
}

void reset(Keyboard* keyboard) {
	memset(keyboard->keys, 0, sizeof(int) * 16);
}

int is_key_down(int key, Keyboard* keyboard) {
	return keyboard->keys[key] == KeyUp; 
}

int is_key_up(int key, Keyboard* keyboard) {
	return keyboard->keys[key] == KeyDown;
}

void press_up_key(int key, Keyboard* keyboard) {
	keyboard->keys[key] = 0;

}

void press_down_key(int key, Keyboard* keyboard) {
	keyboard->keys[key] = 1;	
}

void keyboard_update_state(Keyboard* keyboard, Display* display) {
	SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
			display->quit = true;
        } else {
			 // display->quit = true;
			 if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
				SDL_Keycode sdl_key = event.key.keysym.sym;
				printf("Sdl key: %d\n", (int)sdl_key);
				uint8_t chip8_key = KeyboardMap[(int)sdl_key];

				if (chip8_key <= 0xF) {
					if (event.type == SDL_KEYDOWN) {
						keyboard->press_down_key(chip8_key, keyboard);
					} else if (event.type == SDL_KEYUP) {
						keyboard->press_up_key(chip8_key, keyboard);
					}
				}
   			 }
		}
    }
}

int is_at_least_one_key_pressed(Keyboard* keyboard) {
	for(uint8_t key = 0; key < 16; key++) {
		if(keyboard->keys[key]) {
			return key;
		}
	}
	
	return -1;
}


Keyboard* build_keyboard() {
	setup_keyboard();
	Keyboard* keyboard = malloc(sizeof *keyboard);
	memset(keyboard->keys, 0, sizeof keyboard->keys);
	keyboard->is_key_down = *is_key_down;
	keyboard->is_key_up = *is_key_up;
	keyboard->press_up_key = *press_up_key;
	keyboard->press_down_key = *press_down_key;
       	keyboard->reset = *reset;
	return keyboard;
}
