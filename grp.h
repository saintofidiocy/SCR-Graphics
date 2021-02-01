#include "types.h"
#include "textures.h"

// ----- .dds.grp File Data -----
typedef struct {
  u32 filesize;
  u16 frames;
  u16 unk1; // 0x1001=SD, 0x1002=HD2, 0x1004=HD, 0x1011=SD bitmaps
} grp_header;

typedef struct {
  u32 unk; // zero?
  u16 width;
  u16 height;
  u32 size;
} grp_file;

typedef struct { // header.unk1 == 0x1011
  u16 w;
  u16 h;
  u32 pal[256];
} grp_bmp;


// ----- dds.grp memory data -----
typedef struct {
  u32 frames;
  u32 width;      // grp width
  u32 height;     // grp height
  paldata* pal;
  GLuint tex[1]; // variable length
} grp;


void drawGRP(grp* g, u32 x, u32 y, u32 frame, u32 multcolor);
void drawGRPWater(grp* g, u32 x, u32 y, u32 frame, GLuint mask, grp* n1, grp* n2, u32 nframe1, u32 nframe2, u32 draww, u32 drawh, float time);

grp* loadGRP(const u8* path);
void unloadGRP(grp* g);
