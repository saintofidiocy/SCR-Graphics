#include <stdio.h>
#include <windows.h>
#include "casc.h"
#include "CascLib.h"

// Why, CascLib, why
#undef bool
#undef sprintf


HANDLE casc = NULL;

// internal buffer for parsing files in casc archive, to prevent excessive malloc/free use
u8* readbuf = NULL;
u32 bufMaxSize = 0;


// protobuf data
// https://developers.google.com/protocol-buffers/docs/encoding
#define WIRE_VARINT            0 // int32, int64, uint32, uint64, sint32, sint64, bool, enum
#define WIRE_64BIT             1 // fixed64, sfixed64, double
#define WIRE_LENGTH_DELIMITED  2 // string, bytes, embedded messages, packed repeated fields
#define WIRE_START_GROUP       3 // groups (deprecated)
#define WIRE_END_GROUP         4 // groups (deprecated)
#define WIRE_32BIT             5 // fixed32, sfixed32, float

// product.db format: https://gist.github.com/neivv/d4c822619c8c845d91ca35a52668d48e

// Arbitrary indexes for use in recursive function
#define MESSAGE_ROOT          0
#define MESSAGE_PRODUCT       1
#define MESSAGE_INSTALLATION  2

// used field ID's within the message types
#define ROOT_INSTALLEDPRODUCT 1
#define PRODUCT_VARIANT_UID   2
#define PRODUCT_INSTALL       3
#define INSTALLATION_PATH     1

const u8 sc_uid[] = "s1";

u8* getInstallPath();
bool getInstallPath_Proto(FILE* f, u32 start, u32 len, u32 message, u8** path);
u32 varint(FILE* f, u32* decodeH);



bool loadCascArchive(){
  // from SI https://discord.com/channels/258178661369249802/583406212272619571/739635572787773541
  /*    //    Try the uninstall key (SC:R only?)
    if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Starcraft\\"), 0, KEY_READ | KEY_WOW64_32KEY, &hBlizzSettings) == ERROR_SUCCESS ||
        ::RegOpenKeyEx(HKEY_CURRENT_USER,  TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Starcraft\\"), 0, KEY_READ | KEY_WOW64_32KEY, &hBlizzSettings) == ERROR_SUCCESS ||
        ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Starcraft\\"), 0, KEY_READ | KEY_WOW64_64KEY, &hBlizzSettings) == ERROR_SUCCESS ||
        ::RegOpenKeyEx(HKEY_CURRENT_USER,  TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Starcraft\\"), 0, KEY_READ | KEY_WOW64_64KEY, &hBlizzSettings) == ERROR_SUCCESS )
    {
        DWORD keyType;
        DWORD length = MAX_PATH;

        LSTATUS res = ::RegQueryValueExW(hBlizzSettings, L"InstallLocation", NULL, &keyType, reinterpret_cast<BYTE*>( tempStarcraftPath ), &length);

        ::RegCloseKey(hBlizzSettings);*/
  // /from SI

  u8* path = getInstallPath();
  if(path == NULL){
    puts("Error: Could not locate CASC archive");
    return false;
  }
  
  if(CascOpenStorage(path, 0, &casc) == false){
    free(path);
    return false;
  }
  free(path);
  return true;
}

void closeCascArchive(){
  CascCloseStorage(casc);
  
  if(readbuf != NULL){
    free(readbuf);
    readbuf = NULL;
    bufMaxSize = 0;
  }
}

// Allocates a buffer and loads the entirety of the file contents
u8* loadCascFile(const u8* path, u32* size_out){
  HANDLE cascfile = NULL;
  u8* data = NULL;
  u32 size = 0;
  u32 sizeread = 0;
  
  if(casc == NULL){
    puts("Error: No CASC archive loaded");
    return NULL;
  }
  
  if(CascOpenFile(casc, path, 0, CASC_OPEN_BY_NAME, &cascfile) == false){
    puts("Error: Could not open CASC file");
    return NULL;
  }
  
  size = CascGetFileSize(cascfile, NULL);
  if(size == CASC_INVALID_SIZE){
    puts("Error: Could not get file size");
  }else{
    data = malloc(size);
    if(data == NULL){
      puts("Error: Memory");
    }else{
      CascReadFile(cascfile, data, size, (DWORD*)&sizeread);
      if(size != sizeread){
        printf("Error: %d of %d read :(\n", sizeread, size);
        free(data);
        data = NULL;
      }
    }
  }
  
  CascCloseFile(cascfile);
  
  if(size_out != NULL){
    *size_out = size;
  }
  
  return data;
}



// Opens a casc file and returns the handle
HANDLE openCascFile(const u8* path){
  HANDLE cascfile = NULL;
  
  if(casc == NULL){
    puts("Error: No CASC archive loaded");
    return NULL;
  }
  
  if(CascOpenFile(casc, path, 0, CASC_OPEN_BY_NAME, &cascfile) == false){
    puts("Error: Could not open CASC file");
    return NULL;
  }
  
  return cascfile;
}

// Reads some of a file in to an existing buffer
// - if buf is NULL then an internal buffer is used
// - if offset is -1 then the file is read from the current position
// - if size is -1 then the entire file is read
u8* readCascData(HANDLE hfile, void* buf, u32 offset, u32 size, u32* read){
  if(size == -1){
    size = CascGetFileSize(hfile, NULL);
  }
  
  if(buf == NULL){
    if(size > bufMaxSize){
      readbuf = realloc(readbuf, size);
      if(readbuf == NULL) puts("Error allocating memory in readCascData");
      bufMaxSize = size;
    }
    buf = readbuf;
  }
  
  if(offset != -1){
    CascSetFilePointer(hfile, offset, NULL, FILE_BEGIN);
  }
  
  CascReadFile(hfile, buf, size, (DWORD*)read);
  if(size != *read) printf("CascReadFile size mismatch? %d != %d\n", size, *read);
  
  return buf;
}

// Closes external handle
void closeCascFile(HANDLE hfile){
  CascCloseFile(hfile);
}




// Attempts to find the starcraft install path from product.db
u8* getInstallPath(){
  u8* path = NULL;
  
  u8* programdata = getenv("PROGRAMDATA");
  if(programdata == NULL) return;
  
  u8 agentpath[0x400];
  sprintf(agentpath, "%s\\Battle.net\\Agent\\product.db", programdata);
  
  FILE* f = fopen(agentpath, "rb");
  if(f == NULL){
    puts("Error: Could not locate Battle.net Agent data.");
    return;
  }
  
  getInstallPath_Proto(f, 0, 0, MESSAGE_ROOT, &path);
  
  fclose(f);
  return path;
}

// Decode protobuf structures
bool getInstallPath_Proto(FILE* f, u32 start, u32 len, u32 message, u8** path){
  u8 b;
  u8 wire;
  u8 field;
  u32 i;
  u32 decode;
  u32 decodeH;
  bool exit = false;
  u32 instoffs = 0;
  u32 instlen = 0;
  u8 uid[4] = {0,0,0,0};
  
  while(!exit && !feof(f) && (message == MESSAGE_ROOT || ftell(f)-start < len)){
    b = fgetc(f);
    wire = b&7;
    field = b>>3;
    switch(wire){
      case WIRE_VARINT:
        decode = varint(f, &decodeH);
        //printf("varint %08x%08x\n", decodeH, decode);
        break;
      case WIRE_64BIT:
        fread(&decode, 1, 4, f);
        fread(&decodeH, 1, 4, f);
        //printf("64 bit %08x%08x\n", decodeH, decode);
        break;
      case WIRE_LENGTH_DELIMITED:
      {
        // get length
        decode = varint(f, &decodeH);
        if(decodeH != 0) puts("Nonzero high value in message length !?");
        //printf("len %d\n", decode);
        
        // Different behavior depending on message type
        switch(message){
          case MESSAGE_ROOT:
          {
            if(field == ROOT_INSTALLEDPRODUCT){
              exit = getInstallPath_Proto(f, ftell(f), decode, MESSAGE_PRODUCT, path);
              decode = 0; // already processed, but not necessarily exiting
            }
            break;
          }
          case MESSAGE_PRODUCT:
          {
            switch(field){
              case PRODUCT_INSTALL:
                instoffs = ftell(f);
                instlen = decode;
                if(strcmp(uid, sc_uid) == 0){
                  // don't need to continue processing this message since correct sc_uid was already found
                  exit = true;
                }
                break;
              case PRODUCT_VARIANT_UID:
                if(decode == strlen(sc_uid) && decodeH == 0){
                  for(i=0; i<decode; i++){
                    uid[i] = fgetc(f);
                  }
                  if(instlen != 0 && strcmp(uid, sc_uid) == 0){
                    // don't need to continue processing since correct uid and install field already found
                    exit = true;
                  }
                  decode = 0; // already processed, but not necessarily exiting
                  break;
                }
                break;
              default:
                break;
            }
            break;
          }
          case MESSAGE_INSTALLATION:
          {
            if(field == INSTALLATION_PATH){
              *path = malloc(decode+1);
              if((*path) == NULL){
                puts("Error: Memory");
                return true;
              }
              fread(*path, 1, decode, f);
              (*path)[decode] = 0;
              puts(*path);
              return true;
            }
            break;
          }
        }
        
        // Eat bytes for any unread string/whatever
        if(!exit && decode != 0){
          fseek(f, decode, SEEK_CUR);
        }
        break;
      }
      case WIRE_START_GROUP:
        //puts("Start group (deprecated)");
        break;
      case WIRE_END_GROUP:
        //puts("End group (deprecated)");
        break;
      case WIRE_32BIT:
        fread(&decode, 1, 4, f);
        //printf("32 bit %08x%08x\n", decodeH, decode);
        break;
      default:
        printf("Unknown wire type %d\n", wire);
        exit = true;
    }
  }
  
  // down here since fields 2 and 3 must both be found
  if(message == MESSAGE_PRODUCT && instlen != 0 && strcmp(uid, sc_uid) == 0){
    fseek(f, instoffs, SEEK_SET);
    exit = getInstallPath_Proto(f, instoffs, instlen, MESSAGE_INSTALLATION, path);
  }
  return exit;
}

u32 varint(FILE* f, u32* decodeH){
  u8 b;
  u8 s = 0;
  u64 out = 0;
  do{
    b = fgetc(f);
    out |= (b&0x7F) << s;
    s += 7;
  } while(b&0x80);
  *decodeH = (u32)(out >> 32);
  return (u32)(out);
}
