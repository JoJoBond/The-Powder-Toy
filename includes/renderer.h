#ifndef RENDERER_H
#define RENDERER_H
#include <SDL/SDL.h>
#include "defines.h"
#include "hmap.h"

#define PIXELSIZE 4
#define PIXPACK(x) (x)
#define PIXRGB(r,g,b) (((r)<<16)|((g)<<8)|(b))
#define PIXARGB(a,r,g,b) (((a)<<24)|((r)<<16)|((g)<<8)|(b))
#define PIXR(x) ((x)>>16)
#define PIXG(x) (((x)>>8)&0xFF)
#define PIXB(x) ((x)&0xFF)

void Renderer_Init();

void Renderer_PrepareScreen();

void Renderer_ClearSecondaryBuffer();

void Renderer_Display();

void Renderer_SaveState(unsigned char slot);

void Renderer_RecallState(unsigned char slot);

void Renderer_SaveScreenshot(int w, int h);

_EXTERN_ _INLINE_ void Renderer_DrawPixel(int x, int y, pixel color);

_EXTERN_ _INLINE_ void Renderer_BlendPixel(int x, int y, int r, int g, int b, int a);

_EXTERN_ _INLINE_ void Renderer_AddPixel(int x, int y, int r, int g, int b, int a);

_EXTERN_ _INLINE_ void Renderer_DrawWallBlob(int x, int y, unsigned char cr, unsigned char cg, unsigned char cb);

_EXTERN_ _INLINE_ void Renderer_DrawPartBlob(int x, int y, unsigned char cr, unsigned char cg, unsigned char cb);

_EXTERN_ _INLINE_ void Renderer_AdditivePart(int id, float info1);

void Renderer_DrawDots(int x, int y, int h, int r, int g, int b, int a);

void Renderer_DrawLine(int x1, int y1, int x2, int y2, int r, int g, int b, int a);

_EXTERN_ _INLINE_ void Renderer_DrawRectangle( int x, int y, int w, int h, int r, int g, int b, int a);

_EXTERN_ _INLINE_ void Renderer_FillRectangle(int x, int y, int w, int h, int r, int g, int b, int a);

_EXTERN_ _INLINE_ void Renderer_ClearRectangle(int x, int y, int w, int h);

_EXTERN_ _INLINE_ int Renderer_DrawChar(int x, int y, int c, int r, int g, int b, int a);

_EXTERN_ _INLINE_ void Renderer_XORPixel(int x, int y);

_EXTERN_ _INLINE_ void Renderer_XORLine(int x1, int y1, int x2, int y2);

_EXTERN_ _INLINE_ void Renderer_XORRectangle(int x, int y, int w, int h);

_EXTERN_ _INLINE_ void Renderer_XORFullRectangle(int x, int y, int w, int h);

void Renderer_DrawZoom();

void Renderer_GrabPersistent();

void Renderer_ClearMenu();

void Renderer_DrawImage(pixel *img, int x, int y, int w, int h, int a);

void Renderer_DrawAir();

void Renderer_DrawFire();

void Renderer_DrawAdditiveParts();

#endif
