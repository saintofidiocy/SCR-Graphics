#include <gl/glu.h>
#include "types.h"

// ----- star.spk File Data -----
typedef struct {
  u32 count;
  u16 layerWidth;
  u16 layerHeight;
} spk_layer;

typedef struct {
  u32 layerCount;
  u32 starOffs;
  spk_layer layers[5]; // technically layers[layerCount]
} spk_header;

typedef struct {
  u16 posx;
  u16 posy;
  u16 texx;
  u16 texy;
  u16 w;
  u16 h;
} spk_star;


// ----- star.dds.grp memory data -----
typedef struct {
  u32 layers;
  struct {
    float scWidth;
    float scHeight;
    GLuint tex;
  } tex[1];
} starGrp;


void drawStars(u32 x, u32 y, u32 drawW, u32 drawH, u32 multcolor);
bool loadStars(const u8* texPrefix);
void unloadStars();
