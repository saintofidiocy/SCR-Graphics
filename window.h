#ifndef WINDOW_H
#define WINDOW_H

bool initwnd(u32 w, u32 h, u32 scale, bool fullscreen);
void dispwnd();
void resize(u32 w, u32 h, u32 scale);
void closewnd();

// Called by window, exists in main
bool update(u32 ticks);
void render();

#endif
