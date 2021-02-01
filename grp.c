#include <stdio.h>
#include "grp.h"
#include "shader.h"
#include "casc.h"

void drawGRP(grp* g, u32 x, u32 y, u32 frame, u32 multcolor){
  float w,h;
  
  if(frame >= g->frames) frame = 0;
  
  // get coords
  w = (float)g->width / 2.0;
  h = (float)g->height / 2.0;
  
  // shader params
  if(g->pal == NULL){
    // no palette
    glUseProgram(r_tile.program);
    glUniform1i(r_tile.uniforms[U_TILE_SPRITETEX], 0);
    glUniform4f(r_tile.uniforms[U_TILE_MULTIPLYCOLOR], GETRF(multcolor), GETGF(multcolor), GETBF(multcolor), GETAF(multcolor));
  }else{
    // paletted shader
    glUseProgram(r_palette.program);
    glUniform1i(r_palette.uniforms[U_PALETTE_SPRITETEX], 0);
    glUniform1i(r_palette.uniforms[U_PALETTE_SAMPLETEX], 1);
    glUniform4f(r_palette.uniforms[U_PALETTE_MULTIPLYCOLOR], GETRF(multcolor), GETGF(multcolor), GETBF(multcolor), GETAF(multcolor));
  }
  
  glPushMatrix();
  
  // bind & prep textures
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, g->tex[frame]);
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  
  // Load palette, if there is one
  if(g->pal != NULL){
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g->pal->tex);
  }
  
  // translate & draw
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef((float)x, (float)y, 0.0);
  
  glBegin(GL_QUADS);
  glTexCoord2i(0, 0);
  glVertex2f(-w, -h);
  glTexCoord2i(0, 1);
  glVertex2f(-w, h);
  glTexCoord2i(1, 1);
  glVertex2f(w, h);
  glTexCoord2i(1, 0);
  glVertex2f(w, -h);
  glEnd();
  
  // unbind texture
  glBindTexture(GL_TEXTURE_2D, 0);
  
  glPopMatrix();
}

void drawGRPWater(grp* g, u32 x, u32 y, u32 frame, GLuint mask, grp* n1, grp* n2, u32 nframe1, u32 nframe2, u32 draww, u32 drawh, float time){
  u32 i, xoffs, yoffs;
  float w,h;
  
  if(frame >= g->frames) frame = 0;
  if(nframe1 >= n1->frames) nframe1 = 0;
  if(nframe2 >= n2->frames) nframe2 = 0;
  
  // get coords
  w = (float)g->width / 2.0;
  h = (float)g->height / 2.0;
  xoffs = (x / g->width) & 3;
  yoffs = (y / g->height) & 3;
  
  // shader params
  glUseProgram(r_water.program);
  glUniform1i(r_water.uniforms[U_WATER_SPRITETEX], 0);
  glUniform1i(r_water.uniforms[U_WATER_MASKTEX], 1);
  glUniform1i(r_water.uniforms[U_WATER_SAMPLETEX], 2);
  glUniform1i(r_water.uniforms[U_WATER_SAMPLETEX2], 3);
  glUniform1i(r_water.uniforms[U_WATER_SAMPLETEX3], 4);
  glUniform1i(r_water.uniforms[U_WATER_SAMPLETEX4], 5);
  glUniform4f(r_water.uniforms[U_WATER_DATA], 0.0, 0.0, time, 0.0); // gameX, textureFade, time
  
  glPushMatrix();
  
  // bind & prep textures
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, g->tex[frame]);
  
  // texture/mask coordinates
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, mask);
  
  // large water tile coordinates
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glScalef(1.0/4.0, 1.0/4.0, 1.0);
  
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, n1->tex[nframe1]);
  
  // tile screen coordinates
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glScalef(1.0/(float)draww, 1.0/(float)drawh, 1.0);
  
  
  glActiveTexture(GL_TEXTURE3);
  nframe1++;
  if(nframe1 >= n1->frames) nframe1 = 0;
  glBindTexture(GL_TEXTURE_2D, n1->tex[nframe1]);
  
  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, n2->tex[nframe2]);
  glActiveTexture(GL_TEXTURE5);
  nframe2++;
  if(nframe2 >= n2->frames) nframe2 = 0;
  glBindTexture(GL_TEXTURE_2D, n2->tex[nframe2]);
  
  
  // translate & draw
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef((float)x, (float)y, 0.0);
  
  glBegin(GL_QUADS);
  glMultiTexCoord2i(GL_TEXTURE0, 0, 0);
  glMultiTexCoord2i(GL_TEXTURE1, xoffs+0, yoffs+0);
  glMultiTexCoord2i(GL_TEXTURE2, x-w, y-h);
  glVertex2f(-w, -h);
  glMultiTexCoord2i(GL_TEXTURE0, 0, 1);
  glMultiTexCoord2i(GL_TEXTURE1, xoffs+0, yoffs+1);
  glMultiTexCoord2i(GL_TEXTURE2, x-w, y+h);
  glVertex2f(-w, h);
  glMultiTexCoord2i(GL_TEXTURE0, 1, 1);
  glMultiTexCoord2i(GL_TEXTURE1, xoffs+1, yoffs+1);
  glMultiTexCoord2i(GL_TEXTURE2, x+w, y+h);
  glVertex2f(w, h);
  glMultiTexCoord2i(GL_TEXTURE0, 1, 0);
  glMultiTexCoord2i(GL_TEXTURE1, xoffs+1, yoffs+0);
  glMultiTexCoord2i(GL_TEXTURE2, x+w, y-h);
  glVertex2f(w, -h);
  glEnd();
  
  // unbind all textures
  for(i = 0; i < 6; i++){
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, 0); 
  }
  
  glPopMatrix();
}



grp* loadGRP(const u8* path){
  u32 i,j;
  grp_header header; // = (grp_header*)data;
  grp_file file;
  grp_bmp bmp;
  grp* g = NULL;
  u32 offset;
  
  HANDLE cascfile = NULL;
  u8* data = NULL;
  u32 size = 0;
  u32 sizeread = 0;
  
  cascfile = openCascFile(path);
  if(cascfile == NULL){
    puts("Error: Could not load casc file");
    return NULL;
  }
  
  readCascData(cascfile, &header, -1, sizeof(grp_header), &sizeread);
  if(sizeread != sizeof(grp_header)) puts("grp_header");
  
  if(header.frames == 0){
    puts("Error: Frames == 0 ?!?!?!");
    closeCascFile(cascfile);
    return NULL;
  }
  
  g = malloc(sizeof(grp) + (header.frames - 1)*sizeof(GLuint));
  if(g == NULL){
    printf("Error: Memory %s:%d\n", __FILE__, __LINE__);
    closeCascFile(cascfile);
    return NULL;
  }
  
  g->frames = header.frames;
  g->pal = NULL;
  memset(&g->tex, 0, sizeof(GLuint)*header.frames);
  
  // Apparently *.dds.grp doesn't always contain DDS files :megathink:
  if((header.unk1 & 0x0010) == 0){ // contains DDS files
    //grp_file* file = (grp_file*)(data + sizeof(grp_header));
    //g->width = file.width;
    //g->height = file.height;
    
    offset = sizeof(grp_header);
    
    for(i = 0; i < header.frames && offset < header.filesize; i++){
      //file = (grp_file*)(data + offset);
      readCascData(cascfile, &file, -1, sizeof(grp_file), &sizeread);
      if(sizeread != sizeof(grp_file)) puts("grp_file");
      
      if(i == 0){
        g->width = file.width;
        g->height = file.height;
      }
      
      if(file.width != g->width || file.height != g->height){
        puts("Warning: Texture size mismatch.");
      }
      if(file.size != 0){
        data = readCascData(cascfile, NULL, -1, file.size, &sizeread);
        if(sizeread != file.size) puts("dds file");
        g->tex[i] = loadDDS(data, file.size);
        if(g->tex[i] == 0) puts("Error loading texture");
      }
      offset += sizeof(grp_file) + file.size;
    }
  }else{ // contains paletted bitmap image
    //grp_bmp* bmp = (grp_bmp*)(data + sizeof(grp_header));
    readCascData(cascfile, &bmp, -1, sizeof(grp_bmp), &sizeread);
    if(sizeread != sizeof(grp_bmp)) puts("grp_bmp");
    u32 len = bmp.w * bmp.h;
    g->pal = malloc(sizeof(paldata));
    if(g->pal == NULL){
      puts("Error: Memory");
      free(g);
      closeCascFile(cascfile);
      return NULL;
    }
    g->width = bmp.w;
    g->height = bmp.h;
    g->pal->tex = 0;
    memcpy(g->pal->colors, bmp.pal, sizeof(u32)*256);
    if(updatePal(g->pal) == false){
      puts("Error: Could not load palette.");
      free(g->pal);
      free(g);
      closeCascFile(cascfile);
      return NULL;
    }
    
    offset = sizeof(grp_header) + sizeof(grp_bmp);
    
    for(i = 0; i < header.frames && offset < header.filesize; i++){
      data = readCascData(cascfile, NULL, -1, len, &sizeread);
      if(sizeread != len) puts("dds bmp");
      g->tex[i] = loadBMPPal(data, bmp.w, bmp.h);
      if(g->tex[i] == 0) puts("Error loading texture");
      offset += len;
    }
  }
  
  closeCascFile(cascfile);
  
  return g;
}

void unloadGRP(grp* g){
  if(g == NULL) return;
  glDeleteTextures(g->frames, g->tex);
  free(g);
}
