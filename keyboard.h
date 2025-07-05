#include "./display.h"

enum {
	KeyUp = 1,
	KeyDown = 0
};


typedef struct Keyboard {
	int keys[16];
	int(*is_key_down)(int key, struct Keyboard*);
	int(*is_key_up)(int key, struct Keyboard*);
	void (*press_up_key)(int key, struct Keyboard*);
	void (*press_down_key)(int key, struct Keyboard*);
	void (*reset)(struct Keyboard*);
} Keyboard;


void setup_keyboard(void);
void reset(Keyboard* keyboard);
int is_key_down(int key, Keyboard* keyboard);
int is_key_up(int key, Keyboard* keyboard);
void press_up_key(int key, Keyboard* keyboard);
void press_down_key(int key, Keyboard* keyboard);
void keyboard_update_state(Keyboard* keyboard, Display* display);
int is_at_least_one_key_pressed(Keyboard* keyboard);
Keyboard* build_keyboard(void);