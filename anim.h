#include "types.h"
#include "textures.h"

// ----- .anim File Data -----

typedef struct {
  u32 filetype;  // 0x4D494E41, "ANIM"
  u16 version;   // SD = 0x0101, HD2 = 0x0202, HD = 0x0204
  u16 unk2;      // null? more bytes for version?
  u16 layers;
  u16 entries;   // 999
  u8 layernames[10][32];
  // int entryptrs[entries] // for version 0x0101, pointers to anim_entry/anim_ref's
} anim_header;

typedef struct {
  u32 ptr;
  u32 size;
  u16 texWidth; // width/height of texture image
  u16 texHeight;
} anim_img;

typedef struct {
  u16 frames; // if == 0 use anim_ref
  
  u16 unk2; // 0xFFFF?
  u16 grpWidth;  // width/height of whole GRP frame -- 0 in SD images, to be retrieved from existing GRP's?
  u16 grpHeight;
  
  u32 frameptr; // pointer to frame data
  
  anim_img imgs[10]; // 1 per layer in the header
} anim_entry;

typedef struct {
  u16 frames; // if != 0 use anim_entry
  
  u16 refid; // image ID to refer to
  u32 unk0;
  u32 unk1; // always 0?
  u16 unk2; // who knows
} anim_ref;

typedef struct {
  u16 x;
  u16 y;
  s16 xoffs;
  s16 yoffs;
  u16 width;
  u16 height;
  u16 unk1; // 0?
  u16 unk2; // 0?
} frame;



// ----- anim memory data -----

#define ANIM_TEX_COUNT 2

typedef struct {
  u32 frames;
  u32 width;      // grp width
  u32 height;     // grp height
  float scWidth;  // 1/texWidth
  float scHeight; // 1/twxHeight
  struct {        // texture id's -- any unused must be 0
    GLuint diffuse;
    //bright?
    GLuint teamcolor;
    //emissive?
    //normal?
    //specular?
    //ao_depth?
  } tex;
  frame framedata[1]; // variable length ... could be a pointer, but w\e
} anim;


void drawAnim(anim* a, u32 x, u32 y, u32 frame, u32 playercolor, u32 multcolor, bool halluc);

anim* loadAnim(const u8* path);
anim* loadAnimSD(u8* data, u32 file_size, u32 index);
void unloadAnim(anim* a);

