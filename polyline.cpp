#include "polyline.h"

struct PipeStyle
{
	Gdiplus::Color color;
	int width;
};

struct DashStyle
{
	Gdiplus::Color color;
	int width;
	int length;
	int spaceLength;
};

Gdiplus::GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;

Gdiplus::GraphicsPath pipePath;
Gdiplus::GraphicsPath pipePath2;

PipeStyle pipeStyle;
DashStyle dashStyle;

int dashOffset = 0;

void gdiplusStartup()
{
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

void gdiplusShutdown()
{
	Gdiplus::GdiplusShutdown(gdiplusToken);
}

static void setSamplePoints(float **points)
{
	points[0][0] = 300.0f; points[0][1] = 300.0f;
	points[1][0] = 300.0f; points[1][1] = 100.0f;
	points[2][0] = 100.0f; points[2][1] = 100.0f;
	points[3][0] = 100.0f; points[3][1] = 400.0f;
	points[4][0] = 300.0f; points[4][1] = 400.0f;
	points[5][0] = 100.0f; points[5][1] = 500.0f;
	points[6][0] = 400.0f; points[6][1] = 650.0f;
	points[7][0] = 600.0f; points[7][1] = 500.0f;
	points[8][0] = 500.0f; points[8][1] = 300.0f;
}

static void initPipe(float **points, int pointsArrSize)
{
	float xOffset = 600;
	for (int i = 0; i < pointsArrSize - 1; ++i)
	{
		float x1 = points[i][0];
		float y1 = points[i][1];
		float x2 = points[i + 1][0];
		float y2 = points[i + 1][1];
		pipePath.AddLine(x1, y1, x2, y2);
		pipePath2.AddLine(x1 + 600, y1, x2 + 600, y2);
	}
}

static void initStyles()
{
	Gdiplus::Color pipeColor(255, 0, 0, 255);
	int pipeWidth = 10;
	pipeStyle = { pipeColor, pipeWidth };

	Gdiplus::Color dashColor(255, 255, 0, 0);
	int dashWidth = 6;
	int dashLength = 5;
	int spaceLength = 3;
	dashStyle = { dashColor, dashWidth, dashLength, spaceLength };
}

void initPaint()
{
	//! Debug
	int pointsArrSize = 9;
	float **points = (float **)malloc(sizeof(float *) * pointsArrSize);
	for (int i = 0; i < pointsArrSize; ++i)
	{
		points[i] = (float *)malloc(sizeof(float) * 2);
	}
	setSamplePoints(points);

	initPipe(points, pointsArrSize);
	initStyles();
	
	//! Debug
	for (int i = 0; i < pointsArrSize; ++i)
	{
		free(points[i]);
	}
	free(points);
}

static void paintPipe(Gdiplus::Graphics *graphics, Gdiplus::GraphicsPath *pipePath, PipeStyle *pipeStyle)
{
	Gdiplus::Pen pen(pipeStyle->color, pipeStyle->width);
	pen.SetLineJoin(Gdiplus::LineJoinRound);
	graphics->DrawPath(&pen, pipePath);
}

static void paintDash(Gdiplus::Graphics *graphics, Gdiplus::GraphicsPath *pipePath, DashStyle *dashStyle, bool A, bool B)
{
	Gdiplus::Pen pen(dashStyle->color, dashStyle->width);

	const float dashPattern[2] = { dashStyle->length, dashStyle->spaceLength };
	pen.SetDashPattern(dashPattern, 2);

	int dashDirection = A - B;
	pen.SetDashOffset(dashOffset * dashDirection);

	graphics->DrawPath(&pen, pipePath);

	dashOffset = (dashOffset + 1) % (dashStyle->length + dashStyle->spaceLength);
}

void paint(HDC hdc)
{
	Gdiplus::Graphics graphics(hdc);
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

	paintPipe(&graphics, &pipePath, &pipeStyle);
	paintPipe(&graphics, &pipePath2, &pipeStyle);
	paintDash(&graphics, &pipePath, &dashStyle, true, false);
	paintDash(&graphics, &pipePath2, &dashStyle, false, true);
}
