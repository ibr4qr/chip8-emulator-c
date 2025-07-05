
typedef struct Chip {
        uint16_t pc;
        uint8_t sp;
        uint16_t i;
        uint16_t stack[16];
        uint8_t v[16];
        uint8_t st;
        uint8_t dt;
} Chip;


Chip* build_chip(void);
void update_timers(Chip* chip);