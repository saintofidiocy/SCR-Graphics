#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "window.h"

#include "anim.h"
#include "grp.h"
#include "spk.h"
#include "shader.h"
#include "casc.h"



#define TILESET_BADLANDS  0
#define TILESET_PLATFORM  1
#define TILESET_ASHWORLD  2
#define TILESET_INSTALL   3
#define TILESET_JUNGLE    4
#define TILESET_DESERT    5
#define TILESET_ICE       6
#define TILESET_TWILIGHT  7

u32 tunit[12] = {0x0404f4,0xcc480c,0x94b42c,0x9c4088,0x148cf8,0x143070,0xd0e0cc,0x38fcfc,0x088008,0x7cfcfc,0xb0c4ec,0xd46840};
u8 tilenames[8][12] = {"badlands","platform","ashworld","install","jungle","desert","ice","twilight"};

colorcycle cctable[4][8] = {{{1,8,0,1,6},{1,8,0,7,13},{1,8,0,248,254},{0}},
                            {{0}},
                            {{1,8,0,1,4},{1,8,0,5,8},{1,8,0,9,13},{0}},
                            {{1,8,0,1,13},{1,8,0,248,254},{0}}};
u8 tilecc[8] = {0,1,1,2,0,3,3,3};

struct {
  u8  prefix[8];
  u32 scale;
} textures[] = {
  {"SD/",  1}, // SD
  {"HD2/", 2}, // HD2
  {"",     4}, // HD
};

#define SKIN_CARBOT "carbot/"

u8 skinStr[] = "";
u8 skinImgs[999] = {0}; // 0 = use default texture, 1 = use skin texture
bool useSkin = false;

u32 textureDetail = 0;

// window.c -- will find a better way to do this later
extern u32 mouse[3]; // {x, y, drag state}
extern u8 keyboard[256]; //vkey
extern u32 winwidth;
extern u32 winheight;

unitsdat_t* unitsdat = NULL;
flingydat_t* flingydat = NULL;
spritesdat_t* spritesdat = NULL;
imagesdat_t* imagesdat = NULL;

anim* images[999] = {0};
u32 imagesref[999] = {0}; // imagesref[i] == i for a real image, imagesref[i] < i for a reference
grp* tileset = NULL;
u32 tilesetID = 0;
CV5* megatiles = NULL;
u32 tileCount = 0;

u16 mapWidth = 0;
u16 mapHeight = 0;
u16 MTXM[256*256] = {0};
u8 chkpath[1024] = "";

// Quick image draw list
#define UNIT_MAX  1700
#define SPRITE_MAX 499
struct {
  u16 x,y;
  u16 id;
  u8 player;
} imgs[UNIT_MAX+SPRITE_MAX] = {0};
u32 imgCount = 0;
u32 unitCount = 0;
u32 spriteCount = 0;



// HD tileset effects
typedef struct {
  u32 filetype; // 'KSMT'
  u16 unk1; // version? 0x0001
  u16 count;
  struct {
    u16 vr4id;  // [tileset].dds.vr4 index
    u16 maskid; // [tileset]_mask.dds.grp index
  } tileMasks[1]; // [count]
} tmsk_file;

tmsk_file* tmsk = NULL;
u32 getMaskID(u32 vr4id){ u32 i;for(i=0;i<tmsk->count;i++) if(tmsk->tileMasks[i].vr4id == vr4id) return tmsk->tileMasks[i].maskid; return 0xFFFFFFFF; }
grp* tileMask = NULL;
grp* water_normal_1 = NULL;
grp* water_normal_2 = NULL;
GLuint noise_dds = 0;
u32 n1frame = 0;
u32 n2frame = 0;
u32 nincrement = 0;

u32 tcinit = 0;



bool loadData();
void unloadData();

void loadCHK(const u8* path);

void loadSkin();



int main(int argc, char *argv[]){
  u32 w,h,t,i;
  w = 780;
  h = 642;
  t = textureDetail;
  for(i = 1; i < argc; i++){
    if(argv[i][0] == '-' && argv[i][2] == 0){
      switch(argv[i][1]){
        case 'w':
          i++;
          w = atoi(argv[i]);
          break;
        case 'h':
          i++;
          h = atoi(argv[i]);
          break;
        case 't':
          i++;
          t = atoi(argv[i]);
          break;
        case 'c':
          strcpy(skinStr, SKIN_CARBOT);
          break;
        case 's':
          strcpy(skinStr, argv[i]);
          if(skinStr[strlen(skinStr)-1] != '/') strcat(skinStr, "/");
          break;
        default:
          printf("Unknown command '%c'\n", argv[i][1]);
      }
    }else{
      strcpy(chkpath, argv[i]);
    }
  }
  if(t > 2){
    printf("Invalid texture detail %d\n", t);
    t = 0;
  }
  
  textureDetail = t;
  
  initwnd(w, h, textures[textureDetail].scale, false);
  if(loadData()){
    dispwnd();
  }else{
    puts("Error loading data.");
  }
  unloadData();
  closewnd();
  return 0;
}

// Runs every few ms, with ticks being the ms since the last frame
bool update(u32 ticks){
  if(keyboard[VK_ESCAPE]) return true; // exit
  
  // update SD color cycling
  if(tileset != NULL && tileset->pal != NULL){
    cyclePal(tileset->pal, cctable[tilecc[tilesetID]]);
  }
  
  // update HD water animation
  if(water_normal_1 != NULL){
    nincrement += ticks;
    if(nincrement >= 1000 && textureDetail > 0){
      nincrement = 0;
      n1frame++;
      if(n1frame >= water_normal_1->frames) n1frame = 0;
      n2frame++;
      if(n2frame >= water_normal_2->frames) n2frame = 0;
    }
  }
  
  return false;
}

// Redraws the screen
void render(){
  u32 i,j,t;
  u32 mask = 0;
  
  srand(tcinit); // tick count from the start of the program
  
  drawStars(0, 0, winwidth*textures[textureDetail].scale, winheight*textures[textureDetail].scale, 0xFFFFFFFF);
  
  float time = (float)(GetTickCount() - tcinit)/1000.;
  
  if(tileset != NULL){
    for(j = 0; j <= winheight/tileset->width; j++){
      for(i = 0; i <= winwidth/tileset->width; i++){
        
        if(mapWidth != 0 && mapHeight != 0){
          if(i >= mapWidth) continue;
          t = MTXM[j * mapWidth + i];
          if((t >> 4) >= tileCount) t = 0;
          t = megatiles[t >> 4].tiles[t & 15];
        }else{
          // no map, so draw something
          t = j*16+i + 1816;
        }
        if(textureDetail > 0){
          mask = getMaskID(t);
          if(mask == 0xFFFFFFFF || water_normal_1 == NULL){
            drawGRP(tileset, tileset->width/2 + tileset->width*i, tileset->height/2 + tileset->width*j, t, 0xFFFFFFFF);
          }else{
            drawGRPWater(tileset, tileset->width/2 + tileset->width*i, tileset->height/2 + tileset->width*j, t, tileMask->tex[mask], water_normal_1, water_normal_2, n1frame, n2frame, winwidth, winheight, time);
          }
        }else{
          drawGRP(tileset, tileset->width/2 + tileset->width*i, tileset->height/2 + tileset->width*j, t, 0xFFFFFFFF);
        }
      }
      if(mapHeight != 0 && j >= mapHeight) break;
    }
  }
  
  if(images[0] != NULL){
    for(i = 0; i < imgCount; i++){
      drawAnim(images[imgs[i].id], imgs[i].x*textures[textureDetail].scale, imgs[i].y*textures[textureDetail].scale, 0, tunit[imgs[i].player], 0xFFFFFFFF, false);
    }
    
    if(i == 0){ // no units, so draw something
      for(j=0;j<3;j++){
        for(i=0;i<4;i++){
         drawAnim(images[275], (64 + 128*i)*textures[textureDetail].scale, (48 + 96*j)*textures[textureDetail].scale, 0, tunit[j*4+i], 0xFFFFFFFF, false);
        }
      }
    }
  }
}


bool loadData(){
  u8 path[32];
  u32 i,j;
  
  tcinit = GetTickCount();
  
  if(initShaders() == false){
    puts("Error loading shaders.");
    return false;
  }
  
  puts("Loading CASC data ...");
  
  if(loadCascArchive() == false){
    printf("Error opening CASC (%08x)\n", GetLastError());
    return false;
  }
  
  // load suff
  u32 size;
  unitsdat = (unitsdat_t*)loadCascFile("arr/units.dat", &size);
  if(size != sizeof(unitsdat_t)) puts("units.dat");
  flingydat = (flingydat_t*)loadCascFile("arr/flingy.dat", &size);
  if(size != sizeof(flingydat_t)) puts("flingy.dat");
  spritesdat = (spritesdat_t*)loadCascFile("arr/sprites.dat", &size);
  if(size != sizeof(spritesdat_t)) puts("sprites.dat");
  imagesdat = (imagesdat_t*)loadCascFile("arr/images.dat", &size);
  if(size != sizeof(imagesdat_t)) puts("images.dat");
  
  if(chkpath[0] != 0) loadCHK(chkpath);
  
  // Attempt to load skin, if one is defined
  if(skinStr[0] != 0){
    loadSkin();
  }
  
  // If one was not loaded, ensure the skin name is empty
  if(!useSkin){
    skinStr[0] = 0;
  }
  
  u8* sddata = loadCascFile("SD/mainSD.anim", &size);
  if(sddata == NULL){
    puts("Error: Could not load mainSD.anim");
    closeCascArchive();
    return false;
  }
  anim_header* header = (anim_header*)(sddata);
  u32* entryptrs = (u32*)(sddata + sizeof(anim_header));
  anim_entry* entry;
  anim_ref* ref;
  
  if(header->filetype != INTSTR('ANIM') || header->version != 0x0101){
    puts("Error: Invalid mainSD.anim header");
    free(sddata);
    closeCascArchive();
    return false;
  }
  
  printf("Entries: %d\n", header->entries);
  j=0;
  for(i=0; i<header->entries; i++){
    entry = (anim_entry*)(sddata + entryptrs[i]);
    if(entry->frames == 0){
      ref = (anim_ref*)(entry);
      imagesref[i] = ref->refid;
      if(ref->refid > i){
        printf("Invalid ref ID? %d -> %d\n", i, ref->refid);
        images[i] = NULL;
      }else{
        //printf("%d -> %d\n", i, ref->refid);
        images[i] = images[ref->refid];
      }
    }else{
      imagesref[i] = i;
      if(textureDetail == 0){
        images[i] = loadAnimSD(sddata, size, i);
      }else{
        if(useSkin && skinImgs[i]){
          sprintf(path, "%sanim/%smain_%03d.anim", textures[textureDetail].prefix, skinStr, i);
        }else{
          sprintf(path, "%sanim/main_%03d.anim", textures[textureDetail].prefix, i);
        }
        images[i] = loadAnim(path);
      }
      if(images[i] == NULL){
        printf("Could not load image %d\n", i);
        images[i] = NULL;
      }
    }
  }
  free(sddata);
  
  sprintf(path, "tileset/%s.cv5", tilenames[tilesetID % 8]);
  megatiles = (CV5*)loadCascFile(path, &size);
  tileCount = size / sizeof(CV5);
  
  if(useSkin && textureDetail > 0){
    sprintf(path, "%s%stileset/%s.dds.vr4", textures[textureDetail].prefix, skinStr, tilenames[tilesetID % 8]);
  }else{
    sprintf(path, "%stileset/%s.dds.vr4", textures[textureDetail].prefix, tilenames[tilesetID % 8]);
  }
  printf("Loading tileset %s\n", path);
  tileset = loadGRP(path);
  
  if(useSkin && textureDetail > 0){
    sprintf(path, "%s%s", textures[textureDetail].prefix, skinStr);
  }else{
    strcpy(path, textures[textureDetail].prefix);
  }
  if(loadStars(path) == false){
    puts("Didn't load spk :(");
  }
  
  if(textureDetail > 0){
    // HD tileset effects for ashworld, badlands, desert, ice, jungle, twilight
    if(tilesetID != TILESET_PLATFORM && tilesetID != TILESET_INSTALL){
      sprintf(path, "%s%stileset/%s.tmsk", textures[textureDetail].prefix, skinStr, tilenames[tilesetID % 8]);
      tmsk = (tmsk_file*)loadCascFile(path, &size);
      if(tmsk == NULL) puts("Could not load tile mask IDs.");
      
      sprintf(path, "%s%stileset/%s_mask.dds.grp", textures[textureDetail].prefix, skinStr, tilenames[tilesetID % 8]);
      tileMask = loadGRP(path);
      if(tileMask == NULL) puts("Could not load tile masks.");
    }
    
    switch(tilesetID){
      case TILESET_BADLANDS:
      case TILESET_JUNGLE:
      case TILESET_ICE:
      case TILESET_TWILIGHT:
        // water
        sprintf(path, "%seffect/water_normal_1.dds.grp", textures[textureDetail].prefix);
        water_normal_1 = loadGRP(path);
        if(water_normal_1 == NULL) printf("Could not load %s\n", path);
        sprintf(path, "%seffect/water_normal_2.dds.grp", textures[textureDetail].prefix);
        water_normal_2 = loadGRP(path);
        if(water_normal_2 == NULL) printf("Could not load %s\n", path);
        break;
      case TILESET_ASHWORLD:
      case TILESET_DESERT: // maybe ????
        // heat
        sprintf(path, "%seffect/noise.dds", textures[textureDetail].prefix);
        noise_dds = loadDDSCasc(path);
        if(noise_dds == 0) printf("Could not load %s\n", path);
        break;
    }
  }
  
  closeCascArchive();
  
  printf("Loaded in %d seconds", (GetTickCount()-tcinit+999)/1000);
  return true;
}

void unloadData(){
  u32 i;
  
  // unbind all textures
  for(i = 0; i < 6; i++){
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, 0); 
  }
  
  for(i=0; i<999; i++){
    if(imagesref[i] == i){ // only unload the original instance
      unloadAnim(images[i]);
    }
    images[i] = NULL;
  }
  
  unloadGRP(tileset);
  
  unloadStars();
  
  if(tmsk != NULL) free(tmsk);
  unloadGRP(tileMask);
  unloadGRP(water_normal_1);
  unloadGRP(water_normal_2);
  glDeleteTextures(1, &noise_dds);
  
  deleteShaders();
  
  if(unitsdat != NULL) free(unitsdat);
  if(flingydat != NULL) free(flingydat);
  if(spritesdat != NULL) free(spritesdat);
  if(imagesdat != NULL) free(imagesdat);
  if(megatiles != NULL) free(megatiles);
}

u32 getSpriteImg(u32 id){
  if(id >= 517) id = 0;
  id = spritesdat->image[id];
  if(id >= 999) id = 0;
  return id;
}
u32 getUnitImg(u32 id){
  if(id >= 228) id = 0;
  id = unitsdat->flingy[id];
  if(id >= 209) id = 0;
  id = flingydat->sprite[id];
  return getSpriteImg(id);
}



// Very quick and dirty CHK parsing
void loadCHK(const u8* path){
  u8* data = NULL;
  u32 size = 0;
  
  printf("Loading CHK %s\n", path);
  
  FILE* f = fopen(path, "rb");
  if(f == NULL){
    puts("Could not open CHK.");
    return;
  }
  fseek(f, 0, SEEK_END);
  size = ftell(f);
  rewind(f);
  data = malloc(size);
  fread(data, 1, size, f);
  fclose(f);
  
  chkHeader* chk = NULL;
  UNIT* unit = NULL;
  THG2* sprite = NULL;
  u32 ptr = 0;
  u32 i,count;
  
  printf("sizeof(UNIT) == %d\nsizeof(THG2) == %d\n", sizeof(UNIT), sizeof(THG2));
  
  while(ptr >= 0 && (ptr + 8) <= size){
    chk = (chkHeader*)(data + ptr);
    if(chk->size > 0 && (ptr + chk->size + 8) <= size){
      switch(chk->name){
        case INTSTR('DIM '):
          if(chk->size == 4){
            mapWidth = chk->data[0];
            mapHeight = chk->data[1];
          }
          break;
        case INTSTR('ERA '):
          if(chk->size == 2){
            tilesetID = chk->data[0] & 7;
          }
          break;
        case INTSTR('MTXM'):
          if(chk->size <= 131072){
            memcpy(MTXM, chk->data, chk->size);
          }
          break;
        case INTSTR('UNIT'):
          if(chk->size % sizeof(UNIT) == 0){
            count = chk->size / sizeof(UNIT);
            for(i = 0; i < count && unitCount < UNIT_MAX; i++){
              imgs[imgCount].x = chk->unit[i].x;
              imgs[imgCount].y = chk->unit[i].y;
              imgs[imgCount].id = getUnitImg(chk->unit[i].id);
              imgs[imgCount].player = chk->unit[i].player;
              imgCount++;
              unitCount++;
            }
          }
          break;
        case INTSTR('THG2'):
          if(chk->size % sizeof(THG2) == 0){
            count = chk->size / sizeof(THG2);
            for(i = 0; i < count && spriteCount < SPRITE_MAX && unitCount < UNIT_MAX; i++){
              imgs[imgCount].x = chk->sprite[i].x;
              imgs[imgCount].y = chk->sprite[i].y;
              if(chk->sprite[i].flags & 0x1000){
                imgs[imgCount].id = getSpriteImg(chk->sprite[i].id);
                spriteCount++;
              }else{
                imgs[imgCount].id = getUnitImg(chk->sprite[i].id);
                unitCount++;
              }
              imgs[imgCount].player = chk->sprite[i].player;
              imgCount++;
            }
          }
          break;
      }
    }
    ptr += chk->size + 8;
  }
  
  free(data);
  
  puts("CHK loaded");
}



// Not at all a valid json parser, but as long as the skin.json file is at least somewhat well-formed then hopefully it will work
void loadSkin(){
  u8 path[260];
  u8 key[32];
  u32 i,j,val,depth;
  u32 size;
  u8* data = NULL;
  bool valid = false;
  
  for(i = 0; i < 999; i++) skinImgs[i] = 0;
  
  sprintf(path, "%sanim/%sskin.json", textures[textureDetail].prefix, skinStr);
  data = loadCascFile(path, &size);
  if(data == NULL){
    printf("Skin '%s' not found.", skinStr);
    return;
  }
  
  i = 0;
  while(i < size) {
    // find tag
    for(; i < size && data[i] != '\"'; i++);
    
    // get string
    for(i++,j=0; i < size && data[i] != '\"'; i++){
      if(j < 31) key[j++] = data[i];
      if(data[i] == '\\' && data[i+1] == '\"'){
        if(j < 31) key[j++] = '\"';
        i++;
      }
    }
    key[j] = 0;
    
    // go to value
    for(i++; i < size && isspace(data[i]); i++);
    if(data[i] != ':'){
      printf("Unexpected character '%c' following \"%s\"\n", data[i], key);
      break;
    }
    for(i++; i < size && isspace(data[i]); i++);
    
    // check value
    if(i < size){
      if(strcmp(key, "imageList") == 0 && data[i] == '['){
        i++;
        while(i < size && data[i] != ']'){
          for(; i < size && isspace(data[i]); i++);
          if(i < size && isdigit(data[i])){
            val = atoi(&(data[i]));
            for(; i < size && isdigit(data[i]); i++);
          }else{
            printf("Unexpected character '%c' in imageList\n", data[i]);
            i = size;
            break;
          }
          for(; i < size && (isspace(data[i]) || data[i] == ','); i++);
          if(val < 999) skinImgs[val] = 1;
        }
        if(i < size) valid = true;
        break;
        i++;
      }else if(data[i] == '['){
        for(; i < size && data[i] != ']'; i++);
        i++;
      }else if(data[i] == '\"'){
        for(i++,j=0; i < size && data[i] != '\"'; i++){
          if(data[i] == '\\') i++;
        }
        i++;
      }else{
        for(; i < size && data[i] != ','; i++);
      }
    }
    
    for(i++; i < size && isspace(data[i]); i++);
    if(i < size && data[i] == '}') break;
  };
  
  free(data);
  
  if(valid){
    useSkin = true;
  }
}

