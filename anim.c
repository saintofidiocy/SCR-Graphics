#include <stdio.h>
#include <windows.h>
#include "anim.h"
#include "shader.h"
#include "casc.h"
#include "grpsizes.h"


void drawAnim(anim* a, u32 x, u32 y, u32 frame, u32 playercolor, u32 multcolor, bool halluc){
  u32 texx,texy,texw,texh;
  float l,t,r,b;
  
  if(frame >= a->frames) frame = 0;
  
  // get coords
  texx = a->framedata[frame].x;
  texy = a->framedata[frame].y;
  texw = a->framedata[frame].width;
  texh = a->framedata[frame].height;
  
  l = -(a->width/2.0) + a->framedata[frame].xoffs;
  t = -(a->height/2.0) + a->framedata[frame].yoffs;
  r = l + texw;
  b = t + texh;
  
  // shader params
  glUseProgram(r_sprite.program);
  glUniform1i(r_sprite.uniforms[U_SPRITE_SPRITETEX], 0);
  glUniform1i(r_sprite.uniforms[U_SPRITE_TEAMCOLORTEX], 1);
  glUniform1f(r_sprite.uniforms[U_SPRITE_HALLUCINATE], (float)halluc);
  glUniform4f(r_sprite.uniforms[U_SPRITE_MULTIPLYCOLOR], GETRF(multcolor), GETGF(multcolor), GETBF(multcolor), GETAF(multcolor));
  glUniform4f(r_sprite.uniforms[U_SPRITE_TEAMCOLOR], GETRF(playercolor), GETGF(playercolor), GETBF(playercolor), GETAF(playercolor));
  
  glPushMatrix();
  
  // bind & prep textures
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, a->tex.diffuse);
  
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glScalef(a->scWidth, a->scHeight, 1.0);
  glTranslatef(0.5, -0.5, 0.0); // fix half-pixel offset
  
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, a->tex.teamcolor);
  
  // translate & draw
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef((float)x, (float)y, 0.0);
  
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
 
  glPopMatrix();
}



anim* loadAnim(const u8* path){
  u32 i;
  anim_header header;
  anim_entry entry;
  anim* a = NULL;
  u32 texw,texh;
  
  HANDLE cascfile = NULL;
  u8* data = NULL;
  u32 size = 0;
  u32 sizeread = 0;
  
  cascfile = openCascFile(path);
  if(cascfile == NULL){
    puts("Error: Could not load casc file");
    return NULL;
  }
  
  readCascData(cascfile, &header, -1, sizeof(anim_header), &sizeread);
  if(sizeread != sizeof(anim_header)) puts("anim_header");
  readCascData(cascfile, &entry, -1, sizeof(anim_entry), &sizeread);
  if(sizeread != sizeof(anim_entry)) puts("anim_entry");
  
  if(header.filetype != INTSTR('ANIM')){
    puts("Error: Invalid header");
    printf("%08x == %08x?\n", header.filetype, INTSTR('ANIM'));
    closeCascFile(cascfile);
    return NULL;
  }
  
  if(header.version == 0x0101){
    puts("Cannot load SD anim with this function.");
    closeCascFile(cascfile);
    return NULL;
  }
  
  if(header.entries != 1){
    printf("WARNING: Some entries value other than 1? ??? %d\n", header.entries);
  }
  
  if(entry.frames == 0){
    puts("Error: Frames == 0 ?!?!?!");
    closeCascFile(cascfile);
    return NULL;
  }
  
  a = malloc(sizeof(anim) + (entry.frames - 1)*sizeof(frame));
  if(a == NULL){
    puts("Error: Memory");
    closeCascFile(cascfile);
    return NULL;
  }
  
  a->frames = entry.frames;
  a->width = entry.grpWidth;
  a->height = entry.grpHeight;
  texw = entry.imgs[0].texWidth;
  texh = entry.imgs[0].texHeight;
  a->scWidth = 1.0/(float)texw;
  a->scHeight = 1.0/(float)texh;
  memset(&a->tex, 0, sizeof(a->tex));
  
  for(i = 0; i < header.layers; i++){
    //printf("Layer: %s\n", header.layernames[i]);
    if(entry.imgs[i].ptr != 0){
      if(entry.imgs[i].texWidth != texw || entry.imgs[i].texHeight != texh){
        printf("Warning: Texture size mismatch %s\n", header.layernames[i]);
      }
      if(strcmp(header.layernames[i], "diffuse") == 0){
        data = readCascData(cascfile, NULL, entry.imgs[i].ptr, entry.imgs[i].size, &sizeread);
        if(sizeread != entry.imgs[i].size) puts("diffuse");
        a->tex.diffuse = loadDDS(data, entry.imgs[i].size);
        if(a->tex.diffuse == 0) puts("Error loading diffuse texture");
      }
      if(strcmp(header.layernames[i], "teamcolor") == 0){
        // TODO: 'BMP ' format
        data = readCascData(cascfile, NULL, entry.imgs[i].ptr, entry.imgs[i].size, &sizeread);
        if(sizeread != entry.imgs[i].size) puts("teamcolor");
        a->tex.teamcolor = loadDDS(data, entry.imgs[i].size);
        if(a->tex.teamcolor == 0) puts("Error loading teamcolor texture");
      }
    }
  }
  
  size = sizeof(frame) * entry.frames;
  readCascData(cascfile, a->framedata, entry.frameptr, size, &sizeread);
  if(sizeread != size) puts("framedata");
  
  closeCascFile(cascfile);
  
  return a;
}

anim* loadAnimSD(u8* data, u32 file_size, u32 index){
  u32 i;
  anim_header* header = (anim_header*)data;
  u32 entryPtr = *(u32*)(data + sizeof(anim_header) + index*sizeof(u32));
  anim_entry* entry = (anim_entry*)(data + entryPtr);
  anim* a = NULL;
  u32 texw,texh;
  
  if(entry->frames == 0){
    puts("Error: Frames == 0 ?!?!?!");
    return NULL;
  }
  
  a = malloc(sizeof(anim) + (entry->frames - 1)*sizeof(frame));
  if(a == NULL){
    puts("Error: Memory");
    return NULL;
  }
  
  a->frames = entry->frames;
  a->width = grpsizes[index].w;
  a->height = grpsizes[index].h;
  texw = entry->imgs[0].texWidth;
  texh = entry->imgs[0].texHeight;
  a->scWidth = 1.0/(float)texw;
  a->scHeight = 1.0/(float)texh;
  memset(&a->tex, 0, sizeof(a->tex));
  
  for(i = 0; i < header->layers; i++){
    //printf("Layer: %s\n", header->layernames[i]);
    if(entry->imgs[i].ptr != 0){
      if(entry->imgs[i].texWidth != texw || entry->imgs[i].texHeight != texh){
        printf("Warning: Texture size mismatch %s\n", header->layernames[i]);
      }
      if(strcmp(header->layernames[i], "diffuse") == 0){
        a->tex.diffuse = loadDDS(data + entry->imgs[i].ptr, entry->imgs[i].size);
        if(a->tex.diffuse == 0) puts("Error loading diffuse texture");
      }
      if(strcmp(header->layernames[i], "teamcolor") == 0){
        switch(*(u32*)(data + entry->imgs[i].ptr)){
          case INTSTR('DDS '):
            a->tex.teamcolor = loadDDS(data + entry->imgs[i].ptr, entry->imgs[i].size);
            break;
          case INTSTR('BMP '):
            a->tex.teamcolor = loadBMPLum(data + entry->imgs[i].ptr, entry->imgs[i].texWidth, entry->imgs[i].texHeight);
            break;
          default:
            printf("Unknown format: %.4s\n", data + entry->imgs[i].ptr);
            break;
        }
        if(a->tex.teamcolor == 0) puts("Error loading teamcolor texture");
      }
    }
  }
  
  memcpy(a->framedata, data + entry->frameptr, sizeof(frame) * entry->frames);
  
  return a;
}


void unloadAnim(anim* a){
  if(a == NULL) return;
  glDeleteTextures(ANIM_TEX_COUNT, &(a->tex.diffuse));
  free(a);
}
