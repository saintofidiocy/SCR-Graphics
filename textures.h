#ifndef TEXTURES_H
#define TEXTURES_H

#include "types.h"
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glext.h>


typedef struct {
  GLuint tex;
  u32 colors[256];
} paldata;

typedef struct {
  u8 enabled;
  u8 steps;
  u8 timer;
  u16 start;
  u16 stop;
} colorcycle;

GLuint loadDDS(u8* data, u32 file_size);
GLuint loadBMPLum(u8* data, u32 w, u32 h);
GLuint loadBMPPal(u8* data, u32 w, u32 h);

bool updatePal(paldata* pal);
void cyclePal(paldata* pal, colorcycle* cc);

GLuint texture_loadDDS(const char* path);
GLuint loadDDSCasc(const char* path);


#endif
