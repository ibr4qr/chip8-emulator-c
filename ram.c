#include <stdlib.h>
#include <stdio.h>
#include "./ram.h"

uint8_t sprites[5*16] = {
	// zero
	0xF0,
	0x90,
	0x90,
	0x90,
	0xF0,

	// one
	0x20,
	0x60,
	0x20,
	0x20,
	0x70,

	//two
	0xF0,
	0x10,
	0xF0,
	0x80,
	0xF0,

	// three
	0xF0,
	0x10,
	0xF0,
	0x10,
	0xF0,

	// four
	0x90,
	0x90,
	0xF0,
	0x10,
	0x10,

	// five
	0xF0,
	0x80,
	0xF0,
	0x10,
	0xF0,


	//six
	0xF0,
	0x80,
	0xF0,
	0x90,
	0xF0,

	// seven
	0xF0,
	0x10,
	0x20,
	0x40,
	0x40,

	// eight
	0xF0,
	0x90,
	0xF0,
	0x90,
	0xF0,

	
	// nine
	0xF0,
	0x90,
	0xF0,
	0x10,
	0xF0,


	// A
	0xF0,
	0x90,
	0xF0,
	0x90,
	0x90,

	// B
	0xF0,
	0x90,
	0xF0,
	0x90,
	0xF0,

	// C
	0xF0,
	0x80,
	0x80,
	0x80,
	0xF0,
	
	// D
	0xF0,
	0x90,
	0x90,
	0x90,
	0xF0,

	// E
	0xF0,
	0x80,
	0xF0,
	0x80,
	0xF0,

	// F 
	0xF0,
	0x80,
	0xF0,
	0x80,
	0x80,
};


void set_sprites(Ram* ram) {
	// we have 16 sprites for numbers
	// let's load them starting from address 0
	for(int i = 0; i < 5 * 16; i++) {
		//printf("Loading sprite item: %02x\n", sprites[i]);
		ram->data[i] = sprites[i];
	}
}

// load game from file
int load_game_from_file(Ram* ram, char* filename) {
        	FILE *file;
file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

        // Read contents byte by byte
    unsigned char byte;
    int counter = 0;
    while (fread(&byte, 1, 1, file) == 1) {
	ram->data[PROGRAM_MEMORY_ADDRESS+counter] = byte;
	// printf("%02x ", byte);
	// if((counter + 1 ) % 16 == 0) printf("\n");
	counter++;
    }

     fclose(file);
	

     return EXIT_SUCCESS;
}

Ram* build_ram(char* path_game) {
	Ram* ram = malloc(sizeof *ram);
	

    load_game_from_file(ram, path_game);	

	ram->set_sprites = *set_sprites;

	ram->set_sprites(ram);
	return ram;
}

uint8_t* get_sprite_for_hexadecimal(Ram* ram, int number) {
	uint8_t* sprite = malloc(sizeof(uint8_t) * 40);
	int starting_address = number * 5;
	uint8_t* raw_sprite = malloc(sizeof(uint8_t) * 5);


	for(int i = starting_address; i < starting_address + 5; i++) {
		raw_sprite[i - starting_address] = ram->data[i];
	}

	int counter = 0;


	for(int i = 0; i < 5; i++) {
		uint8_t sprite_item = raw_sprite[i];
		for (int i = 7; i >= 0; i--) {
        		// Shift right i positions and mask with 1
       			 unsigned char bit = (sprite_item >> i) & 1;
			 sprite[counter] = bit;
			 counter++;
   		 }
	}

	return sprite;
}

uint8_t* get_sprite_from_memory(Ram* ram, int number_of_bytes, uint16_t address) {
	uint8_t* sprite = malloc(sizeof(uint8_t) * 8 * number_of_bytes);
	uint8_t* raw_sprite = malloc(sizeof(uint8_t) * number_of_bytes);
	

	for(int i = address; i < address + number_of_bytes; i++) {
		raw_sprite[i - address] = ram->data[i];
	}

	int counter = 0;


	for(int i = 0; i < number_of_bytes; i++) {
		uint8_t sprite_item = raw_sprite[i];
		for (int i = 7; i >= 0; i--) {
        		// Shift right i positions and mask with 1
       			 unsigned char bit = (sprite_item >> i) & 1;
			 sprite[counter] = bit;
			 counter++;
   		 }
	}

	return sprite;
}