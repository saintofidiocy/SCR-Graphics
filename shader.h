#ifndef SHADER_H
#define SHADER_H

#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glext.h>
#include "types.h"

// arbitrary uniform ID's so they can all be included in a single array
#define U_SPRITE_SPRITETEX         0
#define U_SPRITE_TEAMCOLORTEX      1
#define U_SPRITE_HALLUCINATE       2
#define U_SPRITE_MULTIPLYCOLOR     3
#define U_SPRITE_TEAMCOLOR         4

#define U_TILE_SPRITETEX           0
#define U_TILE_MULTIPLYCOLOR       1

#define U_PALETTE_SPRITETEX        0
#define U_PALETTE_SAMPLETEX        1
#define U_PALETTE_MULTIPLYCOLOR    2

#define U_WATER_SPRITETEX          0
#define U_WATER_MASKTEX            1
#define U_WATER_SAMPLETEX          2
#define U_WATER_SAMPLETEX2         3
#define U_WATER_SAMPLETEX3         4
#define U_WATER_SAMPLETEX4         5
#define U_WATER_DATA               6

#define U_HEAT_SPRITETEX           0
#define U_HEAT_SAMPLETEX           1
#define U_HEAT_MAPCOORD            2
#define U_HEAT_INVRESOLUTION       3
#define U_HEAT_WATER_DATA          4

#define UNIFORM_MAX_COUNT          8

typedef struct {
  u32 vertShader;
  u32 fragShader;
  u32 program;
  
  u32 uniforms[UNIFORM_MAX_COUNT];
} shader;

PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLUNIFORM4FVPROC glUniform4fv;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM4FPROC glUniform4f;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLDELETEPROGRAMPROC glDeleteProgram;
PFNGLACTIVETEXTUREPROC glActiveTexture;
PFNGLMULTITEXCOORD2IPROC glMultiTexCoord2i;
PFNGLMULTITEXCOORD2FPROC glMultiTexCoord2f;


bool initShaders();
void deleteShaders();

extern shader r_sprite;
extern shader r_tile;
extern shader r_palette;
extern shader r_water;

#endif
