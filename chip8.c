#include <stdlib.h>
#include <stdio.h>
#include "./chip8.h"

Chip* build_chip() {
	Chip* chip = malloc(sizeof *chip);

	chip->pc = 0x0200;
	chip->sp = 0;
	chip->i = 0;
    chip->dt = 0;
    chip->st = 0;

	return chip;
}

void update_timers(Chip* chip) {
    if (chip->dt> 0) chip->dt--;
    if (chip->st > 0) {
        // If you want a beep, you can print something or control the speaker manually
        // system("osascript -e 'beep'");
        chip->st--;
    }
}