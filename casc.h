#include "types.h"

bool loadCascArchive();
void closeCascArchive();

// Loads whole file in to new buffer
u8* loadCascFile(const u8* path, u32* size_out);

// Loads partial file in existing or internal buffer
HANDLE openCascFile(const u8* path);
u8* readCascData(HANDLE hfile, void* buf, u32 offset, u32 size, u32* read);
void closeCascFile(HANDLE hfile);
