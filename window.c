#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "types.h"
#include "window.h"


#define className "wat"
HINSTANCE hInstance;
HWND mainwnd;
HDC hdc;
HGLRC hrc = NULL;


u32 mouse[3] = {0}; // {x, y, drag state}
u8 keyboard[256] = {0}; //vkey

u32 winwidth = 100;
u32 winheight = 100;
u32 winscale = 1;

LRESULT CALLBACK WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


bool initwnd(u32 w, u32 h, u32 scale, bool fullscreen){
  RECT size = {0,0,w,h};
  WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_OWNDC, WinProc, 0, 0, GetModuleHandle(NULL), NULL/*icon*/, NULL/*cursor*/, NULL, NULL, className, NULL};
  int scwidth = GetSystemMetrics(SM_CXSCREEN);
  int scheight = GetSystemMetrics(SM_CYSCREEN);
  //scAspect = (double)scWidth/(double)scHeight;
  
  hInstance = wc.hInstance;
  
  RegisterClassEx(&wc);
  AdjustWindowRect(&size, WS_OVERLAPPEDWINDOW, 0);
  size.right -= size.left;
  size.bottom -= size.top;
  size.left = (scwidth - size.right)/2;
  size.top = (scheight - size.bottom)/2;
  mainwnd = CreateWindowEx(WS_EX_APPWINDOW, className, "Window", WS_OVERLAPPEDWINDOW, size.left, size.top, size.right, size.bottom, NULL, NULL, hInstance, NULL); 
  
  
  PIXELFORMATDESCRIPTOR pfd = {sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0};
  int pf;
  
  hdc = GetDC(mainwnd);
  if(!(pf = ChoosePixelFormat(hdc, &pfd))){
    MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
    return;
  }
  SetPixelFormat(hdc, pf, &pfd);
  hrc = wglCreateContext(hdc);
  wglMakeCurrent(hdc, hrc);
  ShowWindow(mainwnd,SW_SHOW);
  SetForegroundWindow(mainwnd);
  SetFocus(mainwnd);
 
  //float bgColor[4] = {0.3f,0.3f,0.3f, 1.0f};
  //glClearColor(bgColor[0],bgColor[1],bgColor[2],bgColor[3]);
  //glClearColor(0.39f,0.58f,0.93f, 1.0f);
  glClearColor(0.0,0.0,0.0, 1.0f);
  glClearDepth(1.0f);
  //glDepthFunc(GL_LEQUAL);
  //glEnable(GL_DEPTH_TEST);
  glDisable(GL_DEPTH_TEST);
  //glShadeModel(GL_SMOOTH);
  //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  
  
  glDisable(GL_POLYGON_SMOOTH);
  //glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  //glEnable(GL_CULL_FACE);
  glAlphaFunc(GL_GREATER, 0.0f);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  //glDisable(GL_BLEND);
  glDisable(GL_LINE_SMOOTH);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  resize(w,h,scale);
}

void dispwnd(){
  MSG msg;
  u32 tc = 0;
  u32 tcnew;
  u32 ticks;
  while(msg.message != WM_QUIT){
    if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }else{
      tcnew = GetTickCount();
      if(tcnew > tc){
        ticks = tcnew - tc;
        tc = tcnew;
        if(update(ticks)) return;
      }
      SwapBuffers(hdc);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glLoadIdentity();
      render();
      glFlush();
    }
  }
}

void resize(u32 w, u32 h, u32 scale){
  if(!h){
    puts("Why is height zero?");
    h = 1;
  }
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  
  // scale render target
  w *= scale;
  h *= scale;
  glOrtho(0, w, h, 0, -1000.0, 1000.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  winwidth = w;
  winheight = h;
  winscale = scale;
  
  //render
  SwapBuffers(hdc);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
}

void closewnd(){
  wglMakeCurrent(NULL, NULL);
  wglDeleteContext(hrc);
  ReleaseDC(mainwnd, hdc);
  DestroyWindow(mainwnd);
  UnregisterClass(className,hInstance);
}


LRESULT CALLBACK WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
 switch(msg){
  case WM_CLOSE:
   PostQuitMessage(0);
   return 0;
  case WM_KILLFOCUS:
   //active = 0;
   break;
  case WM_ACTIVATE:
   //active = HIWORD(wParam) ? 0 : 1;
   return 0;
  /*case WM_SYSCOMMAND:
   switch(wParam){
    case SC_SCREENSAVE:
    case SC_MONITORPOWER:
     return 0;
   }
   break;*/
  case WM_MOUSEMOVE:
   mouse[0] = LOWORD(lParam);
   mouse[1] = HIWORD(lParam);
   mouse[2] = wParam;
   break;
  case WM_KEYDOWN:
   keyboard[wParam] |= 1;
   return 0;
  case WM_KEYUP:
   keyboard[wParam] &= ~1;
   return 0;
  case WM_SIZE:
   if(wParam != SIZE_MINIMIZED){
    resize(LOWORD(lParam), HIWORD(lParam), winscale);
    return 0;
   }
   break;
 }
 
 return DefWindowProc(hWnd,msg,wParam,lParam);
}
