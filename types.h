#ifndef TYPES_H
#define TYPES_H

// casts up to 4 characters in single quotes as an integer
#define INTSTR(x) (u32)((((x)&0xFF)<<24)|(((x)&0xFF00)<<8)|(((x)&0xFF0000)>>8)|(((x)&0xFF000000)>>24))

// Color to u32, or u32 to float conversions
//#define RGB(r,g,b) (u32)(r|(g<<8)|(b<<24)|0xFF000000)
#define RGBA(r,g,b,a) (u32)(r|(g<<8)|(b<<24)|(a<<32))
#define GETR(c) (c&0xFF)
#define GETG(c) ((c&0xFF00)>>8)
#define GETB(c) ((c&0xFF0000)>>16)
#define GETA(c) (((u32)c&0xFF000000)>>24)
#define GETRF(c) (c&0xFF)/255.
#define GETGF(c) ((c&0xFF00)>>8)/255.
#define GETBF(c) ((c&0xFF0000)>>16)/255.
#define GETAF(c) (((u32)c&0xFF000000)>>24)/255.

// Generic defines
typedef unsigned char  u8;
typedef   signed char  s8;
typedef unsigned short u16;
typedef   signed short s16;
typedef unsigned int   u32;
typedef   signed int   s32;
typedef unsigned long long int u64;
typedef   signed long long int s64;

typedef enum { false, true } bool;


typedef struct __attribute__((packed)) {
  u8  flingy[228];
  u16 subunit1[228];
  u16 subunit2[228];
  u16 infestation[96];
  u32 construction[228];
  u8  direction[228];
  u8  hasShields[228];
  u16 shields[228];
  u32 hp[228];
  u8  elevation[228];
  u8  unknown[228];
  u8  rank[228];
  u8  aicompidle[228];
  u8  aihumanidle[228];
  u8  returntoidle[228];
  u8  attackunit[228];
  u8  attackmove[228];
  u8  gndWeapon[228];
  u8  gndMaxHits[228];
  u8  airWeapon[228];
  u8  airMaxHits[228];
  u8  aiInternal[228];
  u32 abilFlags[228];
  u8  targetRange[228];
  u8  sightRange[228];
  u8  armorUpgrade[228];
  u8  unitDamageSize[228];
  u8  armor[228];
  u8  rightClickAction[228];
  u16 sndReady[106];
  u16 sndWhatStart[228];
  u16 sndWhatEnd[228];
  u16 sndPissStart[106];
  u16 sndPissEnd[106];
  u16 sndYesStart[106];
  u16 sndYesEnd[106];
  s16 placeBox[456];
  u16 addonPosition[192];
  u16 unitSize[912];
  u16 portrait[228];
  u16 mineralCost[228];
  u16 gasCost[228];
  u16 buildTime[228];
  u16 unknown1[228];
  u8  groupFlags[228];
  u8  supplyProvided[228];
  u8  supplyRequired[228];
  u8  spaceRequired[228];
  u8  spaceProvided[228];
  u16 buildScore[228];
  u16 destroyScore[228];
  u16 mapString[228];
  u8  broodwar[228];
  u16 availFlags[228];
} unitsdat_t;

typedef struct __attribute__((packed)) {
  u16 sprite[209];
  u32 topSpeed[209];
  u16 accel[209];
  u32 haltDistance[209];
  u8  turnRadius[209];
  u8  unused[209];
  u8  moveControl[209];
} flingydat_t;

typedef struct __attribute__((packed)) {
  u16 image[517];
  u8  healthBar[387];
  u8  unknown[517];
  u8  visible[517];
  u8  selCirImg[387];
  u8  selCirOffs[387];
} spritesdat_t;

typedef struct __attribute__((packed)) {
  u32 grpfile[999];
  u8  turns[999];
  u8  clickable[999];
  u8  fulliscript[999];
  u8  cloakdraw[999];
  u8  drawfunc[999];
  u8  remapfunc[999];
  u32 isid[999];
  u32 shieldol[999];
  u32 attackol[999];
  u32 damageol[999];
  u32 specialol[999];
  u32 landol[999];
  u32 liftol[999];
} imagesdat_t;


typedef struct __attribute__((packed)) {
  u32 instance;
  u16 x,y;
  u16 id;
  u16 relation;
  u32 validProps;
  u8 player;
  u8 hp,shields,energy;
  u32 resources;
  u16 hangar;
  u16 flags;
  u32 unused;
  u32 relatedUnit;
} UNIT;

typedef struct __attribute__((packed)) {
  u16 id;
  u16 x,y;
  u16 player;
  u16 flags;
} THG2;

typedef struct __attribute__((packed)) {
  u32 name;
  s32 size;
  union {
    u16 data[1];
    UNIT unit[1];
    THG2 sprite[1];
  };
} chkHeader;


typedef struct __attribute__((packed)) {
  u16 values[10];
  u16 tiles[16];
} CV5;

#endif
