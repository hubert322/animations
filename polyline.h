#ifndef POLYLINE_H
#define POLYLINE_H

#include <objidl.h>
#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")

extern void gdiplusStartup();
extern void gdiplusShutdown();
extern void initPaint();
extern void paint(HDC hdc);

#endif
