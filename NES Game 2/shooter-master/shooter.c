#include <stdlib.h>
#include <string.h>
#include <nes.h>
#include "neslib.h"

#define NES_MAPPER 2	// mapper 2 (UxROM mapper)
#define NES_CHR_BANKS 0	// CHR RAM

// APU (sound) support
#include "apu.h"
//#link "apu.c"

// BCD arithmetic support
#include "bcd.h"
//#link "bcd.c"

// VRAM update buffer
#include "vrambuf.h"
//#link "vrambuf.c"

// famitone2 library
//#link "famitone2.s"

// music and sfx
//#link "music_aftertherain.s"
extern char after_the_rain_music_data[];;
typedef enum { SND_START, SND_HIT, SND_COIN, SND_JUMP } SFXIndex;
#define COLS 32
#define ROWS 28

//#define DEBUG_FRAMERATE

/*{pal:"nes",layout:"nes"}*/
const char PALETTE[32] = { 
  0x0F,

  0x10,0x06,0x28, 0x00,
  0x1C,0x1C,0x25, 0x00,
  0x01,0x10,0x20, 0x00,
  0x02,0x16,0x26, 0x00,
  
  0x17,0x24,0x3C, 0x00,
  0x17,0x1C,0x1C, 0x00,
  0x12,0x36,0x14, 0x00,
  0x10,0x16,0x0F
};

#define COLOR_PLAYER		6
#define COLOR_BULLET		3
#define COLOR_SCORE		2
#define COLOR_FORMATION		1
#define COLOR_ATTACKER		1
#define COLOR_BOMB		2
#define COLOR_EXPLOSION		3


unsigned char misses;


 /*{w:8,h:8,bpp:1,count:128,brev:1,np:2,pofs:8,remap:[0,1,2,4,5,6,7,8,9,10,11,12]}*/
const char TILESET[128*8*2] = {
0x00,0x42,0x00,0x18,0x00,0x24,0x00,0x81,0x81,0x00,0x24,0x00,0x18,0x00,0x42,0x00,0x38,0x7C,0x7C,0x7C,0x38,0x00,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x6C,0x6C,0x48,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x6C,0xFE,0x6C,0xFE,0x6C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0xFE,0xD0,0xFE,0x16,0xFE,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCE,0xDC,0x38,0x76,0xE6,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x6C,0x7C,0xEC,0xEE,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x38,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x70,0x70,0x70,0x70,0x70,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x70,0x38,0x38,0x38,0x38,0x38,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x6C,0x38,0x6C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x38,0xFE,0x38,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0E,0x1E,0x3C,0x78,0xF0,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x7C,0xEE,0xEE,0xEE,0xEE,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x78,0x38,0x38,0x38,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0x0E,0x7C,0xE0,0xEE,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFC,0x0E,0x3C,0x0E,0x0E,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3E,0x7E,0xEE,0xEE,0xFE,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFC,0xE0,0xFC,0x0E,0xEE,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0xE0,0xFC,0xEE,0xEE,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0xEE,0x1C,0x1C,0x38,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0xEE,0x7C,0xEE,0xEE,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0xEE,0xEE,0x7E,0x0E,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x60,0x00,0x00,0x60,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x60,0x00,0x00,0x60,0x60,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1C,0x38,0x70,0x70,0x38,0x1C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0x00,0x00,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x70,0x38,0x1C,0x1C,0x38,0x70,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0xEE,0x1C,0x38,0x00,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x7C,0xEE,0xEE,0xEE,0xE0,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0xEE,0xEE,0xEE,0xFE,0xEE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFC,0xEE,0xFC,0xEE,0xEE,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0xEE,0xE0,0xE0,0xEE,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF8,0xEC,0xEE,0xEE,0xEE,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0xE0,0xF0,0xE0,0xE0,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0xE0,0xF8,0xE0,0xE0,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0xE0,0xEE,0xEE,0xEE,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xEE,0xEE,0xFE,0xEE,0xEE,0xEE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0x38,0x38,0x38,0x38,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0E,0x0E,0x0E,0x0E,0xEE,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xEE,0xFC,0xF8,0xEC,0xEE,0xEE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xE0,0xE0,0xE0,0xE0,0xEE,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC6,0xEE,0xFE,0xFE,0xEE,0xEE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCE,0xEE,0xFE,0xFE,0xEE,0xE6,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0xEE,0xEE,0xEE,0xEE,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0xFC,0xEE,0xEE,0xEE,0xFC,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0xEE,0xEE,0xEE,0xEC,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFC,0xEE,0xEE,0xEE,0xFC,0xEE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0xE0,0x7C,0x0E,0xEE,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x38,0x38,0x38,0x38,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xEE,0xEE,0xEE,0xEE,0xEE,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xEE,0xEE,0xEE,0x6C,0x38,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xEE,0xEE,0xFE,0xFE,0xEE,0xC6,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xEE,0x7C,0x38,0x7C,0xEE,0xEE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xEE,0xEE,0xEE,0x7C,0x38,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x1C,0x38,0x70,0xE0,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*{w:16,h:16,bpp:1,count:16,brev:1,np:2,pofs:8,remap:[5,0,1,2,4,6,7,8,9,10,11,12]}*/  
0x0F,0x18,0x37,0x69,0x69,0x37,0x18,0x0F,0x0F,0x1F,0x3F,0x7F,0x7F,0x3F,0x1F,0x0F,
0x0F,0x1F,0x3F,0x79,0x79,0x3F,0x1F,0x0F,0x0F,0x18,0x37,0x6F,0x6F,0x37,0x18,0x0F,
0xF0,0xF8,0xFC,0x9E,0x9E,0xFC,0xF8,0xF0,0xF0,0x18,0xEC,0xF6,0xF6,0xEC,0x18,0xF0,
0xF0,0x18,0xEC,0x96,0x96,0xEC,0x18,0xF0,0xF0,0xF8,0xFC,0xFE,0xFE,0xFC,0xF8,0xF0,

0x00,0x00,0x04,0x08,0x04,0x03,0x03,0x0F,0x00,0x00,0x04,0x08,0x04,0x00,0x01,0x00,
0x03,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x30,0x02,0x04,0x18,0x00,0x00,0x00,0x00,
0x00,0x00,0x10,0x08,0x10,0x60,0xE0,0xE0,0x00,0x00,0x10,0x08,0x10,0x00,0x40,0x18,
0xE0,0xC0,0xC0,0x80,0x00,0x00,0x00,0x00,0x06,0x20,0x10,0x0C,0x00,0x00,0x00,0x00,

0x00,0x00,0x02,0x04,0x04,0x03,0x03,0x03,0x00,0x00,0x02,0x04,0x04,0x20,0x11,0x0C,
0x03,0x07,0x03,0x03,0x02,0x00,0x00,0x00,0x00,0x20,0x1C,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x08,0x08,0x10,0xE0,0xE0,0x00,0x00,0x00,0x08,0x08,0x10,0x00,0x40,
0xE0,0xC0,0xC0,0x00,0x00,0x00,0x00,0x00,0x10,0x0C,0x02,0x20,0x10,0x0C,0x00,0x00,

0x00,0x00,0x02,0x04,0x04,0x03,0x03,0x03,0x00,0x00,0x02,0x04,0x04,0x20,0x11,0x0C,
0x03,0x07,0x03,0x03,0x02,0x00,0x00,0x00,0x00,0x20,0x1C,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x08,0x08,0x10,0xE0,0xE0,0x00,0x00,0x00,0x08,0x08,0x10,0x00,0x40,
0xE0,0xC0,0xC0,0x00,0x00,0x00,0x00,0x00,0x10,0x0C,0x02,0x20,0x10,0x0C,0x00,0x00,
  
0x00,0x00,0x01,0x01,0x01,0x01,0x03,0x03,0x00,0x00,0x01,0x11,0x08,0x04,0x01,0x20,
0x07,0x07,0x03,0x02,0x00,0x00,0x00,0x00,0x10,0x08,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x80,0x00,0x04,0xCC,0xF0,0xE0,0x00,0x00,0x80,0x00,0x04,0x0C,0x00,0x40,
0xC0,0xC0,0x80,0x00,0x00,0x00,0x00,0x00,0x20,0x10,0x08,0x80,0x40,0x20,0x00,0x00,

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x04,0x02,0x11,0x08,0x04,
0x03,0x03,0x07,0x01,0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x30,0x20,0x60,0xF0,0xFC,0x00,0x00,0x00,0x30,0x20,0x00,0x40,0x04,
0xFC,0xE0,0xC0,0x80,0x00,0x00,0x00,0x00,0x2C,0x00,0x20,0x18,0x80,0x40,0x60,0x00,
  
0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x07,0x00,0x00,0x00,0x10,0x10,0x08,0x04,0x00,
0x07,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x02,0x02,0x02,0x00,0x00,
0x00,0x00,0x00,0x08,0x34,0xE0,0xE0,0xF0,0x00,0x80,0x80,0x88,0x94,0x00,0x40,0x00,
0xFA,0xE4,0x00,0x00,0x00,0x00,0x00,0x00,0x2A,0x04,0x80,0x40,0x20,0x00,0x00,0x00,
  
0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x07,0x00,0x00,0x01,0x09,0x08,0x04,0x02,0x00,
0x0F,0x07,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x04,0x08,0x09,0x01,0x00,
0x00,0x00,0x00,0x00,0x08,0x14,0xE0,0xE0,0x00,0x00,0x00,0x00,0x88,0x94,0x00,0x40,
0xC0,0xE0,0xE0,0x14,0x08,0x00,0x00,0x00,0x00,0x40,0x00,0x94,0x88,0x00,0x00,0x00,

0x00,0x17,0x0F,0x1F,0x1F,0x3F,0x0F,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x60,
0x7F,0x3F,0x1F,0x0F,0x0F,0x0F,0x0F,0x0F,0x7F,0x3F,0x1F,0x00,0x00,0x06,0x0F,0x0F,
0x00,0xE0,0xF0,0xFC,0xFC,0xFC,0xFC,0xFC,0x00,0x18,0x08,0x04,0x04,0x00,0x0C,0x04,
0xF8,0xF8,0xF8,0xF8,0xF0,0x00,0x00,0x00,0xF8,0xF8,0x80,0x00,0x70,0x00,0x00,0x00,
// player (96)
0x3E,0x3E,0x3C,0x3C,0x3C,0x3C,0x3C,0x3C,0x02,0x02,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
// bullet (100)
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

0x00,0x00,0x00,0x08,0x04,0x03,0x06,0x05,0x00,0x00,0x00,0x0C,0x06,0x04,0x00,0x18,
0x05,0x06,0x07,0x00,0x00,0x00,0x00,0x00,0x18,0x00,0x08,0x09,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x08,0x90,0xE0,0x90,0xD0,0x00,0x00,0x00,0x18,0x30,0x10,0x00,0x0C,
0xF0,0x20,0xE0,0x80,0x00,0x00,0x00,0x00,0x0C,0x10,0x18,0xC8,0x00,0x00,0x00,0x00,

0x00,0x04,0x12,0x0C,0x42,0x30,0x00,0x19,0x00,0x00,0x00,0x0A,0x00,0x06,0x08,0x00,
0x11,0x04,0x0C,0x00,0x21,0x46,0x00,0x00,0x04,0x10,0x02,0x0C,0x00,0x00,0x00,0x00,
0x00,0x40,0x08,0x91,0xA2,0x04,0x80,0xCC,0x00,0x00,0x00,0x20,0x00,0x30,0x08,0x00,
0xC4,0x10,0x1A,0x80,0x20,0x10,0x00,0x00,0x10,0x04,0xA0,0x98,0x80,0x00,0x00,0x00,
 
0x61,0x20,0x00,0x04,0x00,0xC4,0x08,0x10,0x00,0x00,0x04,0x06,0x09,0x02,0x10,0x14,
0x10,0x00,0x42,0xC8,0x80,0x00,0x10,0x30,0x34,0x10,0x11,0x02,0x00,0x00,0x00,0x00,
0x86,0xC4,0x00,0x10,0x01,0x10,0x08,0x04,0x00,0x00,0x10,0x30,0x48,0x20,0x04,0x14,
0x04,0x00,0x20,0x89,0x00,0x84,0xC6,0x43,0x16,0x04,0x44,0xA0,0x00,0x00,0x00,0x00,
  
0x00,0x00,0x02,0x08,0x00,0x06,0x02,0x00,0x00,0x40,0x08,0x01,0x11,0x15,0x00,0x38,
0x30,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x00,0x10,0x00,0x04,0x40,0x00,0x00,
0x00,0x00,0x20,0x08,0x00,0x30,0x20,0x00,0x02,0x00,0x08,0x40,0x44,0x54,0x00,0x0E,
0x06,0x20,0x80,0x80,0x00,0x00,0x00,0x00,0x0C,0x80,0x04,0x80,0x00,0x11,0x00,0x00,  
};

#define CHAR(x) ((x)-' ')
#define BLANK 0

void clrscr() {
  vrambuf_clear();
  ppu_off();
  vram_adr(NAMETABLE_A);
  vram_fill(BLANK, 32*28);
  vram_adr(0x0);
  ppu_on_all();
}

char in_rect(byte x, byte y, byte x0, byte y0, byte w, byte h) {
  return ((byte)(x-x0) < w && (byte)(y-y0) < h); 
}

void draw_bcd_word(byte col, byte row, word bcd) {
  byte j;
  static char buf[5];
  buf[4] = CHAR('0');
  for (j=3; j<0x80; j--) {
    buf[j] = CHAR('0'+(bcd&0xf));
    bcd >>= 4;
  }
  vrambuf_put(NTADR_A(col, row), buf, 5);
}


#define NSPRITES 8	// max number of sprites
#define NBULLETS 8	// max number of missiles
#define YOFFSCREEN 240	// offscreen y position 

// sprite indexes
#define PLYRBULLET 7	
#define PLYRSPRITE 7	
#define BOOMSPRITE 6	// explosion sprite

// nametable entries
#define NAME_PLAYER	96
#define NAME_BULLET	100
#define NAME_BOMB	104
#define NAME_EXPLODE	112
#define NAME_ENEMY	68

#define SSRC_FORM1 64	
#define SDST_FORM1 128	

typedef struct {
  byte shape;
} FormationEnemy;

typedef struct {
  byte xpos;
  byte ypos;
  signed char dx;
  signed char dy;
} Bullet;

typedef struct {
  byte x;
  byte y;
  byte name;
  byte tag;
} Sprite;

#define ENEMIES_PER_ROW 8
#define ENEMY_ROWS 3
#define MAX_IN_FORMATION (ENEMIES_PER_ROW*ENEMY_ROWS)

FormationEnemy formation[MAX_IN_FORMATION];
Bullet bullets[NBULLETS];
Sprite vsprites[NSPRITES];

byte formation_offset_x;
signed char formation_direction;
byte current_row;
byte player_x;
const byte player_y = 190;
byte enemy_exploding;
byte enemies_left;
word player_score;

void copy_sprites() {
  byte i;
  byte oamid = 128; 
  for (i=0; i<NSPRITES; i++) {
    Sprite* spr = &vsprites[i];
    if (spr->y != YOFFSCREEN) {
      byte y = spr->y;
      byte x = spr->x;
      byte chr = spr->name;
      byte attr = spr->tag;
      if (attr & 0x40) chr += 2; 
      oamid = oam_spr(x, y, chr, attr, oamid);
      oamid = oam_spr(x+8, y, chr^2, attr, oamid);
    }
  }
  for (i=0; i<NBULLETS; i++) {
    Bullet* mis = &bullets[i];
    if (mis->ypos != YOFFSCREEN) {
      oamid = oam_spr(mis->xpos, mis->ypos, NAME_BULLET,
                      (i==7)?COLOR_BULLET:COLOR_BOMB,oamid);
    }
  }
  oam_hide_rest(oamid);
}

void add_score(word bcd) {
  player_score = bcd_add(player_score, bcd);
  draw_bcd_word(1, 1, player_score);
}

void clrobjs() {
  byte i;
  memset(vsprites, 0, sizeof(vsprites));
  for (i=0; i<NSPRITES; i++) {
    vsprites[i].y = YOFFSCREEN;
  }
  for (i=0; i<NBULLETS; i++) {
    bullets[i].ypos = YOFFSCREEN;
  }
}

void setup_formation() {
  byte i;
  memset(formation, 0, sizeof(formation));
  //memset(attackers, 0, sizeof(attackers));
  for (i=0; i<MAX_IN_FORMATION; i++) {
    byte flagship = i < ENEMIES_PER_ROW;
    formation[i].shape = flagship ? SDST_FORM1 : SDST_FORM1;
  }
  enemies_left = MAX_IN_FORMATION;
  formation_offset_x = 8;
}

void draw_row(byte row) {
  static char buf[32];
  register byte i;
  register byte x = formation_offset_x / 8;
  byte xd = (formation_offset_x & 7) * 3;
  byte y = 3 + row * 5;
  memset(buf, BLANK, sizeof(buf));
  for (i=0; i<ENEMIES_PER_ROW; i++) {
    byte shape = formation[i + row*ENEMIES_PER_ROW].shape;
    if (shape) {
      shape += xd;
      buf[x] = shape;
      buf[x+1] = shape+1;
      buf[x+2] = shape+2;
    }
    x += 3;
  }
  vrambuf_put(NTADR_A(0, y), buf, sizeof(buf));
}

void draw_next_row() {
  draw_row(current_row);
  if (++current_row == ENEMY_ROWS) {
    current_row = 0;
    formation_offset_x += formation_direction;
    if (formation_offset_x == 63) {
      formation_direction = -1;
    }
    else if (formation_offset_x == 8) {
      formation_direction = 1;
    }
  }
}

#define FLIPX 0x40
#define FLIPY 0x80
#define FLIPXY 0xc0

const byte DIR_TO_CODE[32] = {
  0|FLIPXY, 1|FLIPXY, 2|FLIPXY, 3|FLIPXY, 4|FLIPXY, 5|FLIPXY, 6|FLIPXY, 6|FLIPXY,
  6|FLIPX, 6|FLIPX, 5|FLIPX, 4|FLIPX, 3|FLIPX, 2|FLIPX, 1|FLIPX, 0|FLIPX,
  0, 1, 2, 3, 4, 5, 6, 6,
  6|FLIPY, 6|FLIPY, 5|FLIPY, 4|FLIPY, 3|FLIPY, 2|FLIPY, 1|FLIPY, 0|FLIPY,
};

const int SINTBL2[32] = {
  0, 25*2, 49*2, 71*2, 90*2, 106*2, 117*2, 125*2,
  127*2, 125*2, 117*2, 106*2, 90*2, 71*2, 49*2, 25*2,
  0*2, -25*2, -49*2, -71*2, -90*2, -106*2, -117*2, -125*2,
  -127*2, -125*2, -117*2, -106*2, -90*2, -71*2, -49*2, -25*2,
};

#define ISIN(x) (SINTBL2[(x) & 31])
#define ICOS(x) ISIN(x+8)

#define FORMATION_X0 0
#define FORMATION_Y0 26
#define FORMATION_XSPACE 24
#define FORMATION_YSPACE 30

byte get_target_x(byte formation_index) {
  byte column = (formation_index % ENEMIES_PER_ROW);
  return FORMATION_XSPACE*column + FORMATION_X0 + formation_offset_x;
}

byte get_target_y(byte formation_index) {
  byte row = formation_index / ENEMIES_PER_ROW;
  return FORMATION_YSPACE*row + FORMATION_Y0;
}

void draw_player() {
  vsprites[PLYRSPRITE].x = player_x;
  vsprites[PLYRSPRITE].y = player_y;
  vsprites[PLYRSPRITE].name = NAME_PLAYER;
  vsprites[PLYRSPRITE].tag = COLOR_PLAYER;
}

void move_player() {
  byte joy = pad_poll(0);
  // move left/right?
  if ((joy & PAD_LEFT) && player_x > 16) player_x = player_x - 3;
  if ((joy & PAD_RIGHT) && player_x < 224) player_x = player_x + 3;
  // shoot missile?
  if ((joy & PAD_A) && bullets[PLYRBULLET].ypos == YOFFSCREEN) {
    bullets[PLYRBULLET].ypos = player_y-8; // must be multiple of bullet speed
    bullets[PLYRBULLET].xpos = player_x+4; // player X position
    bullets[PLYRBULLET].dy = -4; // player bullet speed
  }
  vsprites[PLYRBULLET].x = player_x;

}

void move_bullets() {
  byte i;
  for (i=0; i<8; i++) { 
    if (bullets[i].ypos != YOFFSCREEN) {
      // hit the bottom or top?
      if ((byte)(bullets[i].ypos += bullets[i].dy) > YOFFSCREEN) {
        bullets[i].ypos = YOFFSCREEN;
      }
    }
  }
}

void new_player() {
  player_x = 120;
  draw_player();
}

void blowup_at(byte x, byte y) {
  vsprites[BOOMSPRITE].tag = COLOR_EXPLOSION;
  vsprites[BOOMSPRITE].name = NAME_EXPLODE; // TODO
  vsprites[BOOMSPRITE].x = x;
  vsprites[BOOMSPRITE].y = y;
  enemy_exploding = 1;
}

void animate_target_explosion() {
  if (enemy_exploding) {
    // animate next frame
    if (enemy_exploding >= 8) {
      enemy_exploding = 0; // hide explosion after 4 frames
      vsprites[BOOMSPRITE].y = YOFFSCREEN;
    } else {
      vsprites[BOOMSPRITE].name = NAME_EXPLODE + (enemy_exploding += 4); // TODO
    }
  }
}

void hide_player_missile() {
  bullets[PLYRBULLET].ypos = YOFFSCREEN;
}

void does_player_shoot_formation() {
  byte mx = bullets[PLYRBULLET].xpos + 4;
  byte my = bullets[PLYRBULLET].ypos;
  signed char row = (my - FORMATION_Y0) / FORMATION_YSPACE;
  if (bullets[PLYRBULLET].ypos == YOFFSCREEN)   {
    return;
  }
  if (row >= 0 && row < ENEMY_ROWS) {
    // ok if unsigned (in fact, must be due to range)
    byte xoffset = mx - FORMATION_X0 - formation_offset_x;
    byte column = xoffset / FORMATION_XSPACE;
    byte localx = xoffset - column * FORMATION_XSPACE;
    if (column < ENEMIES_PER_ROW && localx < 16) {
      char index = column + row * ENEMIES_PER_ROW;
      if (formation[index].shape) {
        formation[index].shape = 0;
        enemies_left--;
        blowup_at(get_target_x(index), get_target_y(index));
        hide_player_missile();
        add_score(10);
      }
    }
  }
}




void play_round() {
  register byte framecount;
  register byte t0;
  byte end_timer = 30;
  add_score(0);
  setup_formation();
  clrobjs();
  formation_direction = 1;
  framecount = 0;
  new_player();
  while (end_timer) {
    
    move_player();
    move_bullets();
    
    if (framecount & 1){
      does_player_shoot_formation();    
    }
    if (framecount & 3) {
    animate_target_explosion(); // continue...
    }
    if (!enemies_left) end_timer--;
      draw_next_row();
    
    
    vrambuf_flush();
    copy_sprites();
#ifdef DEBUG_FRAMERATE
    putchar(t0 & 31, 27, CHAR(' '));
    putchar(framecount & 31, 27, CHAR(' '));
#endif
    framecount++;
    t0 = nesclock();
#ifdef DEBUG_FRAMERATE
    putchar(t0 & 31, 27, CHAR('C'));
    putchar(framecount & 31, 27, CHAR('F'));
#endif
  }
}

#pragma codesize(100)

void set_shifted_pattern(const byte* src, word dest, byte shift) {
  static byte buf[16*3];
  byte y;
  for (y=0; y<16; y++) {
    byte a = src[y];
    byte b = src[y+32];
    buf[y] = a>>shift;
    buf[y+16] = b>>shift | a<<(8-shift);
    buf[y+32] = b<<(8-shift);
  }
  vram_adr(dest);
  vram_write(buf, sizeof(buf));
}

void setup_graphics() {
  byte i;
  word src;
  word dest;
  // copy background
  vram_adr(0x0);
  vram_write(TILESET, sizeof(TILESET));
  // copy sprites
  vram_adr(0x1000);
  vram_write(TILESET, sizeof(TILESET));
  // write shifted versions of formation targets
  src = SSRC_FORM1*16;
  dest = SDST_FORM1*16;
  for (i=0; i<8; i++) {
    if (i==4) src += 16;
    	set_shifted_pattern(&TILESET[src], dest, i);
    dest += 3*16;
  }
  
  vrambuf_clear();
  set_vram_update(updbuf);
}

void setup_sounds() {
  famitone_init(after_the_rain_music_data);
  nmi_set_callback(famitone_update);
}

void main() {  
  setup_graphics();
  apu_init();
  player_score = 0;
  setup_sounds();
  music_play(0);
  misses = 0;
  while (misses < 5) {
    pal_all(PALETTE);
    oam_clear();
    oam_size(1); // 8x16 sprites
    clrscr();
    play_round();
  }
  main();
}