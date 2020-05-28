#define _USE_MATH_DEFINES
#include <math.h>
#include "polyline.h"

enum directionEnum
{
	north,
	northEast,
	east,
	southEast,
	south,
	southWest,
	west,
	northWest
};
struct Pipe
{
	float x1;
	float y1;
	float x2;
	float y2;
	float theta;
	directionEnum direction;
};
struct Particle
{
	float x;
	float y;
	float translateX;
	float translateY;
};
struct ParticleArray
{
	Particle *particle;
	int particleLength;
};
Gdiplus::GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;
ParticleArray *particles;
int particlesLength;
Pipe pipes[6];
const int pipesLength = 6;
const int mainShapePenWidth = 10;
const float velocity = 10;

void gdiplusStartup()
{
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

void gdiplusShutdown()
{
	Gdiplus::GdiplusShutdown(gdiplusToken);
}

static void initMainShape()
{
	float tmp[7][2] = {
		{300.0f, 100.0f},
		{100.0f, 100.0f},
		{100.0f, 400.0f},
		{300.0f, 400.0f},
		{150.0f, 170.0f},
		{270.0f, 170.0f},
		{150.0f, 370.0f}
	};

	for (int i = 0; i < pipesLength; ++i)
	{
		float theta = 0;
		directionEnum direction;
		//! horizontal
		if (tmp[i][1] == tmp[i + 1][1])
		{
			//! west
			if (tmp[i][0] > tmp[i + 1][0])
			{
				direction = west;
			}
			//! east
			else
			{
				direction = east;
			}
		}
		//! vertical
		else if (tmp[i][0] == tmp[i + 1][0])
		{
			//! north
			if (tmp[i][1] > tmp[i + 1][1])
			{
				direction = north;
			}
			//! south
			else
			{
				direction = south;
			}
		}
		//! diagonal
		else
		{
			float dx = tmp[i][0] - tmp[i + 1][0];
			float dy = tmp[i][1] - tmp[i + 1][1];
			theta = atan2(dy, dx);

			//! bottom right to top left
			if (tmp[i][0] > tmp[i + 1][0] && tmp[i][1] > tmp[i + 1][1])
			{
				direction = northWest;
			}
			//! bottom left to top right
			else if (tmp[i][0] < tmp[i + 1][0] && tmp[i][1] > tmp[i + 1][1])
			{
				direction = northEast;
			}
			//! top right to bottom left
			else if (tmp[i][0] > tmp[i + 1][0] && tmp[i][1] < tmp[i + 1][1])
			{
				direction = southWest;
			}
			//! top left to bottom right
			else
			{
				direction = southEast;
			}
		}
		pipes[i] = { tmp[i][0], tmp[i][1], tmp[i + 1][0], tmp[i + 1][1], theta, direction };
	}
}

static void initParticles()
{
	particles = (ParticleArray *)malloc(sizeof(ParticleArray) * (pipesLength - 1));
	particlesLength = pipesLength;
	for (int i = 0; i < pipesLength; ++i)
	{
		float margin = 20.0f;
		int numParticles = hypot(fabsf(pipes[i].x1 - pipes[i].x2), fabsf(pipes[i].y1 - pipes[i].y2)) / (mainShapePenWidth + margin) + 2;
		float particleSize = mainShapePenWidth;

		particles[i].particle = (Particle *)malloc(sizeof(Particle) * numParticles);
		particles[i].particleLength = numParticles;
		for (int j = 0; j < numParticles; ++j)
		{
			float x = pipes[i].x1;
			float y = pipes[i].y1;
			float translateX = 0;
			float translateY = 0;
			float mainAxisDiff = (margin + mainShapePenWidth) * (j) - margin;
			float crossAxisDiff = mainShapePenWidth / 2.0f;

			if (pipes[i].direction == north || pipes[i].direction == south)
			{
				x -= crossAxisDiff;
				if (pipes[i].direction == north)
					y -= mainAxisDiff;
				else
					y += mainAxisDiff;
			}
			else if (pipes[i].direction == east || pipes[i].direction == west)
			{
				y -= crossAxisDiff;
				if (pipes[i].direction == west)
					x -= mainAxisDiff;
				else
					x += mainAxisDiff;
			}
			else
			{
				float dx = mainAxisDiff * cos(pipes[i].theta);
				float dy = mainAxisDiff * sin(pipes[i].theta);

				// bottom right to top left
				if (pipes[i].direction == northWest)
				{
					x -= dx;
					y -= dy;
					translateX = particleSize / 2;
					translateY = particleSize / 2 * -1;
				}
				// bottom left to top right
				else if (pipes[i].direction == northEast)
				{
					x += dx;
					y -= dy;
				}
				// top right to bottom left
				else if (pipes[i].direction == southWest)
				{
					x -= dx;
					y += dy;
					translateX = translateY = particleSize / 2 * -1;
				}
				// top left to bottom right
				else
				{
					x += dx;
					y += dy;
				}
			}
			particles[i].particle[j] = { x, y, translateX, translateY };
		}
	}
}

void initPaint()
{
	initMainShape();
	initParticles();
}

static void paintMainShape(Gdiplus::Graphics &graphics)
{
	const Gdiplus::Color penColor(255, 0, 0, 255);
	Gdiplus::Pen pen(penColor, mainShapePenWidth);

	for (int i = 0; i < pipesLength; ++i)
	{
		int x1 = pipes[i].x1;
		int y1 = pipes[i].y1;
		int x2 = pipes[i].x2;
		int y2 = pipes[i].y2;
		graphics.DrawLine(&pen, x1, y1, x2, y2);
	}
}

static void setParticleInRange(float *x, float *y, int pipesIndex)
{
	float marginAndParticleSize = 30;
	switch (pipes[pipesIndex].direction)
	{
	case north:
		if (*y < pipes[pipesIndex].y2 - marginAndParticleSize)
			*y = pipes[pipesIndex].y1;
		break;
	case northEast:
		break;
	case east:
		if (*x > pipes[pipesIndex].x2 + marginAndParticleSize)
			*x = pipes[pipesIndex].x1;
		break;
	case south:
		if (*y > pipes[pipesIndex].y2 + marginAndParticleSize)
			*y = pipes[pipesIndex].y1;
		break;
	case southWest:
		if (*x < pipes[pipesIndex].x2 - marginAndParticleSize)
			*x = pipes[pipesIndex].x1;
		if (*y > pipes[pipesIndex].y2 + marginAndParticleSize)
			*y = pipes[pipesIndex].y1;
		break;
	case west:
		if (*x < pipes[pipesIndex].x2 - marginAndParticleSize)
			*x = pipes[pipesIndex].x1;
		break;
	case northWest:
		if (*x < pipes[pipesIndex].x2 - marginAndParticleSize)
			*x = pipes[pipesIndex].x1;
		if (*y < pipes[pipesIndex].y2 - marginAndParticleSize)
			*y = pipes[pipesIndex].y1;
		break;
	}
}

static void paintParticles(Gdiplus::Graphics &graphics)
{
	Gdiplus::Color circleColor(255, 255, 0, 0);
	Gdiplus::SolidBrush solidBrush(circleColor);
	float particleSize = mainShapePenWidth;


	//graphics.ResetClip();
	//Gdiplus::Point polyPoints[4] = {
	//	{ 300, 95 },
	//	{ 100, 95 },
	//	{ 100, 105 },
	//	{ 300, 105 }
	//};
	//Gdiplus::GraphicsPath path;
	//path.AddPolygon(polyPoints, 4);

	//Gdiplus::Point polyPoints2[4] = {
	//	{ 95, 100 },
	//	{ 95, 400 },
	//	{ 105, 400 },
	//	{ 105, 100 }
	//};
	//path.AddPolygon(polyPoints2, 4);

	//Gdiplus::Region region(&path);
	//graphics.SetClip(&region);
	//Gdiplus::Pen pen(Gdiplus::Color(255, 0, 255, 0));
	//graphics.DrawPath(&pen, &path);

	for (int i = 0; i < particlesLength; ++i)
	{
		//Gdiplus::Point polyPoints[4] = {
		//	{ points[i].X - mainShapePenWidth / 2, points[i].Y - mainShapePenWidth / 2 },
		//	{ points[i].X + mainShapePenWidth / 2, points[i].Y + mainShapePenWidth / 2 },
		//	{ points[i + 1].X - mainShapePenWidth / 2, points[i + 1].Y - mainShapePenWidth / 2 },
		//	{ points[i + 1].X + mainShapePenWidth / 2, points[i + 1].Y + mainShapePenWidth / 2 }
		//};

		//Gdiplus::GraphicsPath path;
		//path.AddPolygon(polyPoints, 4);
		//Gdiplus::Region region(&path);
		//graphics.SetClip(&region);

		//Gdiplus::Pen pen(Gdiplus::Color(255, 0, 255, 0));
		//graphics.DrawPath(&pen, &path);

		//int x1 = points[i].X;
		//int y1 = points[i].Y;
		//int x2 = points[i + 1].X;
		//int y2 = points[i + 1].Y;

		//float dx = abs(points[i].X - points[i + 1].X);
		//float dy = abs(points[i].Y - points[i + 1].Y);
		//float theta = atan2(dy, dx);

		//switch (particles[i].particle[0].direction)
		//{
		//case north:
		//	break;
		//case northEast:
		//	break;
		//case east:
		//	break;
		//case south:
		//	break;
		//case southWest:
		//	break;
		//case west:
		//	Gdiplus::Point polyPoints[4] = {
		//		{ x1, y1 - mainShapePenWidth / 2 },
		//		{ x2, y1 - mainShapePenWidth / 2 },
		//		{ x2, y2 - mainShapePenWidth / 2 },
		//		{ 300, 105 }
		//	};
		//	break;
		//case northWest:
		//	break;
		//}
		float dx = 0;
		float dy = 0;
		switch (pipes[i].direction)
		{
		case north:
			dy -= velocity;
			break;
		case northEast:
			break;
		case east:
			dx += velocity;
			break;
		case south:
			dy += velocity;
			break;
		case southWest:
			dx -= velocity * cos(pipes[i].theta);
			dy += velocity * sin(pipes[i].theta);
			break;
		case west:
			dx -= velocity;
			break;
		case northWest:
			dx -= velocity * cos(pipes[i].theta);
			dy -= velocity * sin(pipes[i].theta);
			break;
		}

		for (int j = 0; j < particles[i].particleLength; ++j)
		{
			Particle *particle = &particles[i].particle[j];
			particle->x += dx;
			particle->y += dy;
			setParticleInRange(&particle->x, &particle->y, i);

			graphics.TranslateTransform(particle->x, particle->y);
			graphics.RotateTransform(pipes[i].theta * 180.0f / M_PI);
			graphics.TranslateTransform(-1 * particle->x + particle->translateX, -1 * particle->y + particle->translateY);
			graphics.FillRectangle(&solidBrush, particle->x, particle->y, particleSize, particleSize);
			graphics.ResetTransform();
		}
	}
}

void paint(HDC hdc)
{
	Gdiplus::Graphics graphics(hdc);
	const int mainShapePenWidth = 10;
	paintMainShape(graphics);
	paintParticles(graphics);
}
