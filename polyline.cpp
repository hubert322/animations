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
//struct Pipe
//{
//	float x1;
//	float y1;
//	float x2;
//	float y2;
//	float 
//};
struct Particle
{
	float x;
	float y;
	float translateX;
	float translateY;
	float theta;
	directionEnum direction;
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
Gdiplus::Point points[7];
const int pointsLength = 7;
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
	points[0] = { 300, 100 };
	points[1] = { 100, 100 };
	points[2] = { 100, 400 };
	points[3] = { 300, 400 };
	points[4] = { 150, 170 };
	points[5] = { 270, 170 };
	points[6] = { 150, 370 };
}

static void initParticles()
{
	particles = (ParticleArray *)malloc(sizeof(ParticleArray) * (pointsLength - 1));
	particlesLength = pointsLength - 1;
	for (int i = 0; i < pointsLength - 1; ++i)
	{
		float margin = 20.0f;
		int numParticles = hypot(abs(points[i].X - points[i + 1].X), abs(points[i].Y - points[i + 1].Y)) / (mainShapePenWidth + margin) + 2;
		float particleSize = mainShapePenWidth;

		particles[i].particle = (Particle *)malloc(sizeof(Particle) * numParticles);
		particles[i].particleLength = numParticles;
		for (int j = 0; j < numParticles; ++j)
		{
			float x = points[i].X;
			float y = points[i].Y;
			float translateX = 0;
			float translateY = 0;
			float theta = 0;
			float mainAxisDiff = (margin + mainShapePenWidth) * (j) - margin;
			float crossAxisDiff = mainShapePenWidth / 2.0f;
			directionEnum direction;
			if (points[i].Y == points[i + 1].Y)
			{
				y -= crossAxisDiff;
				if (points[i].X > points[i + 1].X)
				{
					x -= mainAxisDiff;
					direction = west;
				}
				else
				{
					x += mainAxisDiff;
					direction = east;
				}
			}
			else if (points[i].X == points[i + 1].X)
			{
				x -= crossAxisDiff;
				if (points[i].Y > points[i + 1].Y)
				{
					y -= mainAxisDiff;
					direction = north;
				}
				else
				{
					y += mainAxisDiff;
					direction = south;
				}
			}
			else
			{
				float dx = abs(points[i].X - points[i + 1].X);
				float dy = abs(points[i].Y - points[i + 1].Y);

				theta = atan2(dy, dx);
				float xVal = mainAxisDiff * cos(theta);
				float yVal = mainAxisDiff * sin(theta);

				// bottom right to top left
				if (points[i].X > points[i + 1].X &&points[i].Y > points[i + 1].Y)
				{
					x -= xVal;
					y -= yVal;
					translateX = particleSize / 2;
					translateY = particleSize / 2 * -1;
					direction = northWest;
				}
				// bottom left to top right
				else if (points[i].X < points[i + 1].X && points[i].Y > points[i + 1].Y)
				{
					x += xVal;
					y -= yVal;
					direction = northEast;
				}
				// top right to bottom left
				else if (points[i].X > points[i + 1].X &&points[i].Y < points[i + 1].Y)
				{
					x -= xVal;
					y += yVal;
					translateX = translateY = particleSize / 2 * -1;
					direction = southWest;
				}
				// top left to bottom right
				else
				{
					x += xVal;
					y += yVal;
					direction = southEast;
				}

				if (points[i].Y < points[i + 1].Y)
				{
					theta *= -1;
				}
			}
			particles[i].particle[j] = { x, y, translateX, translateY, theta, direction };
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

	for (int i = 0; i < pointsLength - 1; ++i)
	{
		int x1 = points[i].X;
		int y1 = points[i].Y;
		int x2 = points[i + 1].X;
		int y2 = points[i + 1].Y;
		graphics.DrawLine(&pen, x1, y1, x2, y2);
	}
}

static void paintParticles(Gdiplus::Graphics &graphics)
{
	Gdiplus::Color circleColor(255, 255, 0, 0);
	Gdiplus::SolidBrush solidBrush(circleColor);
	float particleSize = mainShapePenWidth;

	graphics.ResetClip();
	Gdiplus::Point polyPoints[4] = {
		{ 300, 95 },
		{ 100, 95 },
		{ 100, 105 },
		{ 300, 105 }
	};
	Gdiplus::GraphicsPath path;
	path.AddPolygon(polyPoints, 4);

	Gdiplus::Point polyPoints2[4] = {
		{ 95, 100 },
		{ 95, 400 },
		{ 105, 400 },
		{ 105, 100 }
	};
	path.AddPolygon(polyPoints2, 4);

	Gdiplus::Region region(&path);
	graphics.SetClip(&region);
	Gdiplus::Pen pen(Gdiplus::Color(255, 0, 255, 0));
	graphics.DrawPath(&pen, &path);

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

		for (int j = 0; j < particles[i].particleLength; ++j)
		{
			Particle &particle = particles[i].particle[j];
			float &x = particle.x;
			float &y = particle.y;
			switch (particle.direction)
			{
			case north:
				y -= velocity;
				if (y < points[i + 1].Y - 30)
					y = points[i].Y;
				break;
			case northEast:
				break;
			case east:
				x += velocity;
				if (x > points[i + 1].X + 30)
					x = points[i].X;
				break;
			case south:
				y += velocity;
				if (y > points[i + 1].Y + 30)
					y = points[i].Y;
				break;
			case southWest:
				x -= velocity * cos(particle.theta * -1);
				y += velocity * sin(particle.theta * -1);
				if (x < points[i + 1].X - 30)
					x = points[i].X;
				if (y > points[i + 1].Y + 30)
					y = points[i].Y;
				break;
			case west:
				x -= velocity;
				if (x < points[i + 1].X - 30)
					x = points[i].X;
				break;
			case northWest:
				x -= velocity * cos(particle.theta);
				y -= velocity * sin(particle.theta);
				if (x < points[i + 1].X - 30)
					x = points[i].X;
				if (y < points[i + 1].Y - 30)
					y = points[i].Y;
				break;
			}
			graphics.TranslateTransform(x, y);
			graphics.RotateTransform(particle.theta * 180.0f / M_PI);
			graphics.TranslateTransform(-x + particle.translateX, -y + particle.translateY);
			graphics.FillRectangle(&solidBrush, x, y, particleSize, particleSize);
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
