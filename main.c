#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#include "./display.h"
#include "./keyboard.h"
#include "./ram.h"
#include "./chip8.h"

#include <SDL2/SDL.h>

// Define screen dimensions
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// Define MAX and MIN macros
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

#define DEBUG_CHIP 0

uint8_t random_and_mask(uint8_t kk)
{
	return (arc4random_uniform(256) & kk);
}

void enableRawMode()
{
	struct termios oldt, newt;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;

	// Disable canonical mode and echo
	newt.c_lflag &= ~(ICANON | ECHO);

	// Apply the new settings
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

uint64_t current_millis()
{
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return (uint64_t)(ts.tv_sec * 1000ull + ts.tv_nsec / 1000000ull);
}

void evaluate(Chip *chip, Ram *ram, Display *display, Keyboard *keyboard)
{
	uint64_t last_timer_tick = current_millis();

	while (!display->quit)
	{
		uint8_t high = ram->data[chip->pc];
		uint8_t low = ram->data[chip->pc + 1];
		uint16_t instruction = (high << 8) | low;

		keyboard_update_state(keyboard, display);

		// // Timer update at 60Hz
		uint64_t now = current_millis();
		if (now - last_timer_tick >= 16)
		{ // ~16 ms
			update_timers(chip);
			last_timer_tick = now;
		}

		if (DEBUG_CHIP)
		{
			printf("%04x\t%04x\t", chip->pc, instruction);
		}

		switch (instruction)
		{
		case 0x00e0:
		{
			if (DEBUG_CHIP)
				printf("Clear Display\n");
			display->clear(display);
			break;
		}
		case 0x00ee:
		{
			uint16_t address = chip->stack[chip->sp];
			chip->sp--;
			chip->pc = address;
			if (DEBUG_CHIP)
				printf("Returing to %04x from subroutine\n", address);
			continue;
		}
		default:
		{
			uint8_t opcode = (instruction & 0xF000) >> 12;
			uint16_t argument = instruction & 0x0FFF;
			switch (opcode)
			{
			case 0:
				if (DEBUG_CHIP)
					printf("Jump to machine code routine in %04x\n", argument);
				break;
			case 1:
				if (DEBUG_CHIP)
					printf("Jump to location %04x\n", argument);
				chip->pc = argument;
				continue;
			case 2:
				chip->sp++;
				chip->stack[chip->sp] = chip->pc + 2;
				chip->pc = argument;
				if (DEBUG_CHIP)
					printf("Call subroutine in %04x\n", argument);
				continue;
			case 3:
			{
				uint8_t x = (high & 0x0F); // Extract the lower nibble from high byte
				uint8_t kk = low;
				if (DEBUG_CHIP)
					printf("Skip next instruction if V%d == %02x, current value: %d \n", x, kk, chip->v[x]);

				if (chip->v[x] == kk)
				{
					chip->pc += 4;
					continue;
				}
				break;
			}
			case 4:
			{
				uint8_t x = (high & 0x0F); // Extract the lower nibble from high byte
				uint8_t kk = low;
				if (DEBUG_CHIP)
					printf("Skip next instruction if V%d != %02x, current value: %d \n", x, kk, chip->v[x]);
				if (chip->v[x] != kk)
				{
					chip->pc += 4;
					continue;
				}
				break;
			}
			case 5:
			{
				uint8_t x = (instruction & 0x0F00) >> 8;
				uint8_t y = (instruction & 0x00F0) >> 4;
				if (DEBUG_CHIP)
					printf("Skip next instruction if V%d == V%d\n", x, y);
				if (chip->v[x] == chip->v[y])
				{
					chip->pc += 4;
					continue;
				}
				break;
			}
			case 6:
			{
				uint8_t x = (high & 0x0F); // Extract the lower nibble from high byte
				uint8_t kk = low;
				if (DEBUG_CHIP)
					printf("V%d = %d\n", x, kk);
				chip->v[x] = kk;
				break;
			}
			case 7:
			{
				uint8_t x = (high & 0x0F); // Extract the lower nibble from high byte
				uint8_t kk = low;
				if (DEBUG_CHIP)
					printf("V%d = V%d(%d) + %02x\n", x, x, chip->v[x], kk);
				chip->v[x] += kk;
				break;
			}
			case 8:
			{
				uint8_t x = (instruction & 0x0F00) >> 8;
				uint8_t y = (instruction & 0x00F0) >> 4;
				uint8_t last_nibble = instruction & 0x000F;
				switch (last_nibble)
				{
				case 0:
					if (DEBUG_CHIP)
						printf("Set V%d = V%d\n", x, y);
					chip->v[x] = chip->v[y];
					break;
				case 1:
					if (DEBUG_CHIP)
						printf("Set V%d OR V%dx\n", x, y);
					chip->v[x] |= chip->v[y];
					break;
				case 2:
					if (DEBUG_CHIP)
						printf("Set V%d AND V%d\n", x, y);
					chip->v[x] &= chip->v[y];
					break;
				case 3:
					if (DEBUG_CHIP)
						printf("Set V%d XOR V%d\n", x, y);
					chip->v[x] ^= chip->v[y];
					break;
				case 4:
				{
					uint16_t result = chip->v[x] + chip->v[y];
					chip->v[0xF] = result > 255 ? 1 : 0;
					chip->v[x] = result & 0xFF;
					if (DEBUG_CHIP)
						printf("Set V%d = V%d + V%d, set VF if carry, V%d = %d + %d", x, x, y, x, chip->v[x], chip->v[y]);
					break;
				}
				case 5:
					if (DEBUG_CHIP)
						printf("Set Vx = Vx - Vy, set VF = NOT borrow.\n");
					chip->v[0xF] = (chip->v[x] >= chip->v[y]) ? 1 : 0;
					chip->v[x] -= chip->v[y];
					break;
				case 6:
					if (DEBUG_CHIP)
						printf("Set Vx = Vx SHR 1.\n");
					chip->v[x] >>= 1;
					break;
				case 7:
					if (DEBUG_CHIP)
						printf("Set Vx = Vy - Vx, set VF = NOT borrow.\n");
					chip->v[0xF] = (chip->v[y] >= chip->v[x]) ? 1 : 0;
					chip->v[x] = chip->v[y] - chip->v[x];
					break;
				case 0xe:
					if (DEBUG_CHIP)
						printf("Set Vx = Vx SHL 1\n");
					chip->v[x] <<= 1;
					break;
				default:
					printf("Something went wrong, this instruction is not supported");
				}
				break;
			}
			case 9:
			{
				uint8_t x = (instruction & 0x0F00) >> 8;
				uint8_t y = (instruction & 0x00F0) >> 4;

				uint8_t last_nibble = instruction & 0x000F;

				switch (last_nibble)
				{
				case 0:
					if (DEBUG_CHIP)
						printf("Skip next instruction if Vx != Vy.\n");
					if (chip->v[x] != chip->v[y])
					{
						chip->pc += 4;
						continue;
					}
					break;
				default:
					printf("Something went wrong, this instruction is not supported");
				}
				break;
			}
			case 0xa:
				if (DEBUG_CHIP)
					printf("I = %02x\n", argument);
				chip->i = argument;
				break;
			case 0xb:
				if (DEBUG_CHIP)
					printf("Jump to location %04x + the value in V0(%d)\n", argument, chip->v[0]);
				chip->pc = argument + chip->v[0];
				continue;
			case 0xc:
			{
				uint8_t x = (high & 0x0F); // Extract register index (Vx)
				uint8_t kk = low;		   // Immediate value (kk)
				if (DEBUG_CHIP)
					printf("Set V%X = random byte AND 0x%02X.\n", x, kk);
				chip->v[x] = random_and_mask(kk);
				break;
			}
			case 0xd:
			{
				uint8_t x = (instruction & 0x0F00) >> 8;
				uint8_t y = (instruction & 0x00F0) >> 4;
				uint8_t number_of_bytes = instruction & 0x000F;

				if (DEBUG_CHIP)
					printf("Display %d bytes sprite starting at memory location %04x at (%d, %d), set VF = collision\n", number_of_bytes, chip->i, chip->v[x], chip->v[y]);

				uint8_t *sprite = get_sprite_from_memory(ram, number_of_bytes, chip->i);
				uint8_t collision = display->draw(chip->v[x], chip->v[y], sprite, number_of_bytes, display);
				if (collision)
				{
					chip->v[0xf] = 1;
				}
				// if(!DEBUG_CHIP) {
				// 	display->render(display);
				// }
				break;
			}
			case 0xe:
				switch (low)
				{
				case 0x9e:
				{
					uint8_t x = (instruction & 0x0F00) >> 8;
					if (DEBUG_CHIP)
						printf("Skip next instruction if key %d is pressed.\n", chip->v[x]);
					if (keyboard->is_key_down(chip->v[x], keyboard))
					{
						chip->pc += 4;
						continue;
					}
					break;
				}
				case 0xa1:
				{
					uint8_t x = (instruction & 0x0F00) >> 8;
					if (DEBUG_CHIP)
						printf("Skip next instruction if key %d is not pressed\n", chip->v[x]);
					if (keyboard->is_key_up(chip->v[x], keyboard))
					{
						chip->pc += 4;
						continue;
					}
					break;
				}
				default:
					printf("Opcode not supported");
				}
				break;
			case 0xf:
			{
				uint8_t x = (instruction & 0x0F00) >> 8;
				uint8_t dt = chip->dt;
				switch (low)
				{
				case 0x07:
					if (DEBUG_CHIP)
						printf("V%d = Delay Timer, V%d = %02x\n", x, x, dt);
					chip->v[x] = dt;
					break;
				case 0x0a:
					if (DEBUG_CHIP)
						printf("Wait for a key press, store the value of the key in V%d\n", x);

					int pressed_key = is_at_least_one_key_pressed(keyboard);
					if (pressed_key == -1)
					{
						continue;
					}
					else
					{
						chip->v[x] = pressed_key;
					}
					break;
				case 0x15:
					chip->dt = chip->v[x];
					if (DEBUG_CHIP)
						printf("Set delay timer = %02x\n", chip->v[x]);
					break;
				case 0x18:
					chip->st = chip->v[x];
					if (DEBUG_CHIP)
						printf("Set sound timer = Vx.\n");
					break;
				case 0x1e:
					if (DEBUG_CHIP)
						printf("Set I = I + V%d(%d).\n", x, chip->v[x]);
					chip->i += chip->v[x];
					break;
				case 0x29:
				{
					if (DEBUG_CHIP)
						printf("Set I = location of sprite for digit %d.\n", chip->v[x]);
					uint8_t digit = chip->v[x];
					uint16_t digit_sprite_starting_address = digit * 5;
					chip->i = digit_sprite_starting_address;
					break;
				}
				case 0x33:
				{
					if (DEBUG_CHIP)
						printf("Store BCD representation of %d in memory locations %04x, %04x, and %04x\n", chip->v[x], chip->i, chip->i + 1, chip->i + 2);
					uint8_t value = chip->v[x];
					ram->data[chip->i] = value / 100;
					ram->data[chip->i + 1] = (value / 10) % 10;

					// Unità
					ram->data[chip->i + 2] = value % 10;
					break;
				}
				case 0x55:
				{
					if (DEBUG_CHIP)
						printf("Store registers V0 through Vx in memory starting at location I.\n");
					uint16_t starting_address = chip->i;
					for (int i = 0; i <= x; i++)
					{
						ram->data[starting_address + i] = chip->v[i];
					}
					break;
				}
				case 0x65:
				{
					if (DEBUG_CHIP)
						printf("Read registers V0 through V%d from memory starting at location %04x\n", x, chip->i);
					uint16_t starting_address = chip->i;
					for (int i = 0; i <= x; i++)
					{
						chip->v[i] = ram->data[starting_address + i];
					}
					break;
				}
				default:
					printf("Opcode not supported!");
				}
			}
			}
		}
		}
		if (!DEBUG_CHIP)
		{
			render_sdl(display);
		}
		chip->pc += 2;
		if (DEBUG_CHIP)
		{
			sleep(1);
		}
		// // Optionally sleep a bit to reduce CPU usage
		struct timespec ts = {.tv_sec = 0, .tv_nsec = 100000}; // 1ms
		nanosleep(&ts, NULL);
	};
}

int main(int argc, char **argv)
{
	// printf("\033[?25l");
	// printf("\033[?1049h"); // Enter alternate screen buffer
	// enableRawMode();

	if (argc < 2)
	{
		printf("Missing path to game\n");
		return 0;
	}

	char *game_path = argv[1];
	Chip *chip = build_chip();
	Ram *ram = build_ram(game_path);
	// keyboard
	Keyboard *keyboard = build_keyboard();
	// demo rendering sprite
	Display *display = get_display();
	evaluate(chip, ram, display, keyboard);
	// uint8_t* sprite = get_sprite_from_memory(ram, 6, 0x02ea);
	// display->draw(2, 12, sprite, 6, display);
	// display->render(display);

	// printf("\033[?1049l"); // Return to normal terminal

	display->cleanup(display);
	return 0;
}