#include <stdio.h>
#include "shader.h"


shader r_sprite = {0};
shader r_tile = {0};
shader r_palette = {0};
shader r_water = {0};

// Shader & uniform information
struct {
  shader* r;
  u8 vertpath[24];
  u8 fragpath[24];
  struct {
    u32 id;
    u8 name[16];
  } uniforms[UNIFORM_MAX_COUNT];
} shaderinfo[] = {
  {&r_sprite, "", "shaders\\sprite.txt", {
    {U_SPRITE_SPRITETEX,      "spriteTex"},
    {U_SPRITE_TEAMCOLORTEX,   "teamcolorTex"},
    {U_SPRITE_HALLUCINATE,    "hallucinate"},
    {U_SPRITE_MULTIPLYCOLOR,  "multiplyColor"},
    {U_SPRITE_TEAMCOLOR,      "teamColor"},
    0}
  },
  {&r_tile, "", "shaders\\tile.txt", {
    {U_TILE_SPRITETEX,        "spriteTex"},
    {U_TILE_MULTIPLYCOLOR,    "multiplyColor"},
    0}
  },
  {&r_palette, "", "shaders\\palette.txt", {
    {U_PALETTE_SPRITETEX,     "spriteTex"},
    {U_PALETTE_SAMPLETEX,     "sampleTex"},
    {U_PALETTE_MULTIPLYCOLOR, "multiplyColor"},
    0}
  },
  {&r_water, "", "shaders\\water.txt", {
    {U_WATER_SPRITETEX,       "spriteTex"},
    {U_WATER_MASKTEX,         "maskTex"},
    {U_WATER_SAMPLETEX,       "sampleTex"},
    {U_WATER_SAMPLETEX2,      "sampleTex2"},
    {U_WATER_SAMPLETEX3,      "sampleTex3"},
    {U_WATER_SAMPLETEX4,      "sampleTex4"},
    {U_WATER_DATA,            "data"},
    0}
  },
  {0}
};

/*
#define U_HEAT_SPRITETEX           0
#define U_HEAT_SAMPLETEX           1
#define U_HEAT_MAPCOORD            2
#define U_HEAT_INVRESOLUTION       3
#define U_HEAT_WATER_DATA          4
*/


bool loadShaders();
u32 compileShader(u32 type, const u8 *filename);
u32 linkShader(GLuint vertShader, GLuint fragShader);
static void show_info_log(GLuint object, PFNGLGETSHADERIVPROC glGet__iv, PFNGLGETSHADERINFOLOGPROC glGet__InfoLog);


bool initShaders(){
  // load OpenGL shader functions
  glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
  glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
  glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
  glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
  glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
  glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
  glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
  glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
  glUniform4fv = (PFNGLUNIFORM4FVPROC)wglGetProcAddress("glUniform4fv");
  glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
  glUniform1f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
  glUniform4f = (PFNGLUNIFORM4FPROC)wglGetProcAddress("glUniform4f");
  glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
  glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
  glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
  glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
  glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
  glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
  glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
  glMultiTexCoord2i = (PFNGLMULTITEXCOORD2IPROC)wglGetProcAddress("glMultiTexCoord2i");
  glMultiTexCoord2f = (PFNGLMULTITEXCOORD2FPROC)wglGetProcAddress("glMultiTexCoord2f");
  
  return loadShaders();
}

void deleteShaders(){
  u32 i;
  glUseProgram(0);
  
  for(i = 0; shaderinfo[i].r != NULL; i++){
    glDeleteProgram(shaderinfo[i].r->program);
    //glDeleteShader(shaderinfo[i].r->vertShader);
    glDeleteShader(shaderinfo[i].r->fragShader); 
  }
}


bool loadShaders(){
  u32 i,j;
  
  for(i = 0; shaderinfo[i].r != NULL; i++){
    printf("Compiling %s ...\n", shaderinfo[i].fragpath);
    //if((shaderinfo[i].r->vertShader = compileShader(GL_VERTEX_SHADER, "???")) == 0){
    //  puts("Error making vertex shader.");
    //  return false;
    //}
    if((shaderinfo[i].r->fragShader = compileShader(GL_FRAGMENT_SHADER, shaderinfo[i].fragpath)) == 0){
      puts("Error making fragment shader.");
      return false;
    }
    if((shaderinfo[i].r->program = linkShader(shaderinfo[i].r->vertShader, shaderinfo[i].r->fragShader)) == 0){
      puts("Error making shader program.");
      return false;
    }
    for(j = 0; j < UNIFORM_MAX_COUNT && shaderinfo[i].uniforms[j].name[0] != 0; j++){
      shaderinfo[i].r->uniforms[shaderinfo[i].uniforms[j].id] = glGetUniformLocation(shaderinfo[i].r->program, shaderinfo[i].uniforms[j].name);
    }
  }
  
  return true;
}

u32 compileShader(u32 type, const u8 *filename){
  u32 size, shader;
  u8* data;
  u32 shader_ok;
  
  puts(filename);
  FILE * pFile = fopen(filename, "rb");
  if(pFile == NULL) return 0;
  fseek(pFile, 0, SEEK_END);
  size = ftell(pFile);
  rewind(pFile);
  data = malloc(size);
  fread(data, 1, size, pFile);
  
  shader = glCreateShader(type);
  glShaderSource(shader, 1, (const char**)&data, &size);
  glCompileShader(shader);
  free(data);
  
  glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
  if(!shader_ok) {
    fprintf(stderr, "Failed to compile %s:\n", filename);
    show_info_log(shader, glGetShaderiv, glGetShaderInfoLog);
    glDeleteShader(shader);
    return 0;
  }
  show_info_log(shader, glGetShaderiv, glGetShaderInfoLog);
  return shader;
}

u32 linkShader(GLuint vertShader, GLuint fragShader){
  GLint program_ok;
  GLuint program = glCreateProgram();
  //glAttachShader(program, vertShader);
  glAttachShader(program, fragShader);
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
  if(!program_ok){
    puts("Failed to link shader program:");
    show_info_log(program, (PFNGLGETSHADERIVPROC)glGetProgramiv, glGetProgramInfoLog);
    glDeleteProgram(program);
    return 0;
  }
  show_info_log(program, (PFNGLGETSHADERIVPROC)glGetProgramiv, glGetProgramInfoLog);
  return program;
}

static void show_info_log(GLuint object, PFNGLGETSHADERIVPROC glGet__iv, PFNGLGETSHADERINFOLOGPROC glGet__InfoLog){
  GLint log_length;
  u8 *log;
  
  glGet__iv(object, GL_INFO_LOG_LENGTH, &log_length);
  log = malloc(log_length);
  glGet__InfoLog(object, log_length, NULL, log);
  puts(log);
  free(log);
}

