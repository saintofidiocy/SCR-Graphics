#include <stdio.h>
#include "spk.h"
#include "grp.h"
#include "shader.h"
#include "casc.h"

spk_header* spk = NULL;
starGrp* stars = NULL;

void drawStar(u32 layer, u32 x, u32 y, spk_star* star);
bool loadStarGrp(const u8* path);



void drawStars(u32 x, u32 y, u32 drawW, u32 drawH, u32 multcolor){
  u32 i,j;
  
  if(spk == NULL) return;
  
  spk_star* star = (spk_star*)(((u8*)spk) + spk->starOffs);
  u32 index = 0;
  u32 layerstart = 0;
  
  u32 x1,x1i,x2,y1,y2;
  
  
  // shader params
  glUseProgram(r_tile.program);
  glUniform1i(r_tile.uniforms[U_TILE_SPRITETEX], 0);
  glUniform4f(r_tile.uniforms[U_TILE_MULTIPLYCOLOR], GETRF(multcolor), GETGF(multcolor), GETBF(multcolor), GETAF(multcolor));
  
  glPushMatrix();
  
  for(i = 0; i < spk->layerCount; i++){
    // bind & prep textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, stars->tex[i].tex);
    
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glScalef(stars->tex[i].scWidth, stars->tex[i].scHeight, 1.0);
    glTranslatef(0.5, -0.5, 0.0); // fix half-pixel offset
    
    // scale x,y
    
    x1i = (x / spk->layers[i].layerWidth) * spk->layers[i].layerWidth;
    x2 = ((x + drawW) / spk->layers[i].layerWidth + 1) * spk->layers[i].layerWidth;
    y1 = (y / spk->layers[i].layerHeight) * spk->layers[i].layerHeight;
    y2 = ((y + drawW) / spk->layers[i].layerHeight + 1) * spk->layers[i].layerHeight;
    
    layerstart = index;
    
    for(; y1 < y2; y1 += spk->layers[i].layerHeight){
      for(x1 = x1i; x1 < x2; x1 += spk->layers[i].layerWidth){
        index = layerstart;
        for(j = 0; j < spk->layers[i].count; j++){
          drawStar(i, x + x1, y + y1, &star[index]);
          index++;
        }
      }
    }
    
  }
  
  glPopMatrix();
}

void drawStar(u32 layer, u32 x, u32 y, spk_star* star){
  u32 texx,texy,texw,texh;
  float l,t,r,b;
  
  // get coords
  texx = star->texx;
  texy = star->texy;
  texw = star->w;
  texh = star->h;
  
  l = -(star->w/2.0);
  t = -(star->h/2.0);
  r = l + star->w;
  b = t + star->h;
  
  // translate & draw
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef((float)(x + star->posx), (float)(y + star->posy), 0.0);
  
  glBegin(GL_QUADS);
  glTexCoord2i(texx, texy);
  glVertex2f(l, t);
  glTexCoord2i(texx, texy+texh);
  glVertex2f(l, b);
  glTexCoord2i(texx+texw, texy+texh);
  glVertex2f(r, b);
  glTexCoord2i(texx+texw, texy);
  glVertex2f(r, t);
  glEnd();
}




bool loadStars(const u8* texPrefix){
  u32 i;
  u8 path[32];
  u32 size = 0;
  
  // load textures
  sprintf(path, "%sparallax/star.dds.grp", texPrefix);
  if(loadStarGrp(path) == false){
    puts("Error loading star.dds.grp");
    return false;
  }
  
  sprintf(path, "%sparallax/star.spk", texPrefix);
  spk = (spk_header*)loadCascFile(path, &size);
  if(spk == NULL){
    puts("Error loading star.spk");
    unloadStars();
    return false;
  }
  return true;
}


void unloadStars(){
  u32 i;
  if(stars != NULL){
    for(i=0; i<stars->layers; i++){
      glDeleteTextures(1, &stars->tex[i].tex);
    }
    free(stars);
    stars = NULL;
  }
  if(spk != NULL){
    free(spk);
    spk = NULL;
  }
}


bool loadStarGrp(const u8* path){
  u32 i;
  u32 size = 0;
  u8* data = loadCascFile(path, &size);
  if(data == NULL){
    printf("Could not find %s\n", path);
    return false;
  }
  
  grp_header* header = (grp_header*)data;
  grp_file* file = NULL;
  u32 offset;
  
  if(header->frames == 0){
    puts("Error: Frames == 0 ?!?!?!");
    free(data);
    return false;
  }
  
  stars = malloc(header->frames*(sizeof(starGrp) - sizeof(u32)) + sizeof(u32));
  if(stars == NULL){
    printf("Error: Memory %s:%d\n", __FILE__, __LINE__);
    free(data);
    return false;
  }
  
  stars->layers = header->frames;
  memset(&stars->tex, 0, header->frames*(sizeof(starGrp) - sizeof(u32)));
  offset = sizeof(grp_header);
  
  for(i = 0; i < header->frames && offset < header->filesize; i++){
    file = (grp_file*)(data + offset);
    
    stars->tex[i].scWidth = 1.0/(float)file->width;
    stars->tex[i].scHeight = 1.0/(float)file->height;
    if(file->size != 0){
      stars->tex[i].tex = loadDDS(data + offset + sizeof(grp_file), file->size);
      if(stars->tex[i].tex == 0) puts("Error loading texture");
    }
    offset += sizeof(grp_file) + file->size;
  }
  
  free(data);
  return true;
}

