#define PROGRAM_MEMORY_ADDRESS 0x0200
#define RAM_SIZE 4096

// Ram
typedef struct Ram {
	void(*set_sprites)(struct Ram*);
	uint8_t data[RAM_SIZE];
} Ram;

void set_sprites(Ram* ram);
int load_game_from_file(Ram* ram, char* filename);
Ram* build_ram(char* game_path);
uint8_t* get_sprite_for_hexadecimal(Ram* ram, int number);
uint8_t* get_sprite_from_memory(Ram* ram, int number_of_bytes, uint16_t address);
