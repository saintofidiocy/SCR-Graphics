#include <stdio.h>
#include <windows.h>
#include "textures.h"
#include "casc.h"

// DDS code adapted from https://gist.github.com/tilkinsc/13191c0c1e5d6b25fbe79bbd2288a673#file-load_dds-c-L44

typedef struct {
  u32 size;
  u32 flags;
  u32 fourCC;
  u32 RGBBitCount;
  u32 RBitMask;
  u32 GBitMask;
  u32 BBitMask;
  u32 ABitMask;
} DDS_PIXELFORMAT;

typedef struct {
  u32 filetype;
  u32 size;
  u32 flags;
  u32 height;
  u32 width;
  u32 pitchOrLinearSize;
  u32 depth;
  u32 mipMapCount;
  u32 reserved1[11];
  DDS_PIXELFORMAT ddspf;
  u32 caps;
  u32 caps2;
  u32 caps3;
  u32 caps4;
  u32 reserved2;
} DDS_HEADER;

PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D = NULL;

// Loads a texture from DDS data
GLuint loadDDS(u8* data, u32 file_size){
  u32 data_size;
 	DDS_HEADER* header = NULL;
	 
	 u8 blockSize;
	 u32 format;
		u32 mipMapCount;
	 
	 GLuint tid = 0;
	 
	 // Get function pointer if it doesn't exist
	 if(glCompressedTexImage2D == NULL){
    glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC)wglGetProcAddress("glCompressedTexImage2D");
  }
	 
	 header = (DDS_HEADER*)data;
	 
  if(header->filetype != INTSTR('DDS ')){
    puts("Error: Invalid header");
    printf("%08x == %08x?\n", header->filetype, INTSTR('DDS '));
    return 0;
  }
  
  // Check if mipmaps are even used?
  mipMapCount = header->mipMapCount;
  if(mipMapCount == 0) mipMapCount = 1;
	 
	 
	 switch(header->ddspf.fourCC){
	   case INTSTR('DXT1'):
      format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
      blockSize = 8;
      break;
    case INTSTR('DXT3'):
      format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
      blockSize = 16;
      break;
    case INTSTR('DXT5'):
      format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
      blockSize = 16;
      break;
    case INTSTR('DX10'):
      // wut
    default:
      printf("Error: Unknown format: %.4s\n", header->ddspf.fourCC);
      return 0;
  }
  // BC4U/BC4S/ATI2/BC55/R8G8_B8G8/G8R8_G8B8/UYVY-packed/YUY2-packed unsupported
	 
	 
  // allocate new unsigned char space with file_size - (file_code + header_size) magnitude
  // read rest of file
  data_size = file_size - sizeof(DDS_HEADER);
	 
	 
  // prepare new incomplete texture
 	glGenTextures(1, &tid);
 	if(tid == 0){
    puts("Error: glGenTextures");
    return 0;
  }
  
	 
  // bind the texture
  // make it complete by specifying all needed parameters and ensuring all mipmaps are filled
 	glBindTexture(GL_TEXTURE_2D, tid);
 	if(mipMapCount > 1){
  		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	  	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipMapCount-1); // opengl likes array length of mipmaps
		  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // don't forget to enable mipmaping
  }else{ // Don't enable mipmapping if there aren't any
		  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  }
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	 
  // prepare some variables
		u32 offset = sizeof(DDS_HEADER);
		u32 size = 0;
		u32 w = header->width;
		u32 h = header->height;
		
  
  // loop through sending block at a time with the magic formula
  // upload to opengl properly, note the offset transverses the pointer
  // assumes each mipmap is 1/2 the size of the previous mipmap
  u32 i;
		for(i=0; i<mipMapCount; i++) {
		 	if(w == 0 || h == 0) { // discard any odd mipmaps 0x1 0x2 resolutions
		 	 	mipMapCount--;
		 		 continue;
		 	}
		 	size = ((w+3)/4) * ((h+3)/4) * blockSize;
		 	glCompressedTexImage2D(GL_TEXTURE_2D, i, format, w, h, 0, size, data + offset);
		 	offset += size;
		 	w /= 2;
		 	h /= 2;
		}
  // discard any odd mipmaps, ensure a complete texture
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipMapCount-1);
  // unbind
	 glBindTexture(GL_TEXTURE_2D, 0);
	 
	 return tid;
}


// Loads a texture from 8-bit greyscale data
GLuint loadBMPLum(u8* data, u32 w, u32 h){
	 GLuint tid = 0;
	 
  if(*(u32*)(data) != INTSTR('BMP ')){
    puts("Error: Invalid file type");
    printf("%08x == %08x?\n", *(u32*)(data), INTSTR('BMP '));
    return 0;
  }
  
  // prepare new incomplete texture
 	glGenTextures(1, &tid);
 	if(tid == 0){
    puts("Error: glGenTextures");
    return 0;
  }
  
  // bind the texture
 	glBindTexture(GL_TEXTURE_2D, tid);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  
  // load as 8-bit greyscale data
 	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data + 4);
 	
  // unbind
	 glBindTexture(GL_TEXTURE_2D, 0);
	 
	 return tid;
}


// Loads a texture from raw 8-bit paletted data
GLuint loadBMPPal(u8* data, u32 w, u32 h){
	 GLuint tid = 0;
  
  // prepare new incomplete texture
 	glGenTextures(1, &tid);
 	if(tid == 0){
    puts("Error: glGenTextures");
    return 0;
  }
  
  // bind the texture
 	glBindTexture(GL_TEXTURE_2D, tid);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  
  // load palette index to red channel only -- alternatively load as a luminance so palette index is R, G, and B channels
 	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, data);
 	
  // unbind
	 glBindTexture(GL_TEXTURE_2D, 0);
	 
	 return tid;
}


// Loads the color data to the palette texture in a palette
bool updatePal(paldata* pal){
  if(pal == NULL) return true; // successfully didn't load a pal?
  
  if(pal->tex == 0){ // palette hasn't been initialized
   	glGenTextures(1, &pal->tex);
   	if(pal->tex == 0){
      puts("Error: glGenTextures");
      return false;
    }
    // bind & initialize the texture
   	glBindTexture(GL_TEXTURE_2D, pal->tex);
  		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
   	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pal->colors);
  }else{
    // bind & update texture
   	glBindTexture(GL_TEXTURE_2D, pal->tex);
   	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 1, GL_RGBA, GL_UNSIGNED_BYTE, pal->colors);
  }
  
  // unbind
	 glBindTexture(GL_TEXTURE_2D, 0);
	 
	 return true;
}

// Rotates some palette colors based on a color cycle structure
void cyclePal(paldata* pal, colorcycle* cc){
  u32 i,j;
  u32 tmp;
  bool update = false;
  
  for(j=0;j<8;j++){
    if(!cc[j].enabled) continue;
    if(cc[j].timer){
      cc[j].timer--;
    }else{
      cc[j].timer = cc[j].steps;
      tmp = pal->colors[cc[j].stop];
      for(i = cc[j].stop; i > cc[j].start; i--){
        pal->colors[i] = pal->colors[i-1];
      }
      pal->colors[cc[j].start] = tmp;
      update = true;
    }
  }
  
  if(update){
    updatePal(pal);
  }
}


// Loads a DDS image from a disk file to a texture
GLuint texture_loadDDS(const char* path) {
  u32 file_size;
	 GLuint tid = 0;
 	FILE* f = fopen(path, "rb");
 	if(f == NULL){
    printf("Could not open file \"%s\"\n", path);
    return 0;
  }
  
 	fseek(f, 0, SEEK_END);
 	file_size = ftell(f);
	 fseek(f, 0, SEEK_SET);
	 
 	u8* buffer = malloc(file_size);
 	if(buffer == NULL){
    puts("Error: Memory");
    fclose(f);
    return 0;
  }
  fread(buffer, 1, file_size, f);
	 fclose(f);
	 
	 tid = loadDDS(buffer, file_size);
	 
	 free(buffer);
	 return tid;
}

// Loads a DDS image from a CASC file to a texture
GLuint loadDDSCasc(const char* path) {
	 GLuint tid = 0;
  HANDLE cascfile = NULL;
  u8* data = NULL;
  u32 size = 0;
  
	 cascfile = openCascFile(path);
  if(cascfile == NULL){
    puts("Error: Could not load casc file");
    return tid;
  }
	 data = readCascData(cascfile, NULL, -1, -1, &size);
  if(data != NULL){
    tid = loadDDS(data, size);
  }
	 return tid;
}

