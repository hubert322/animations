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
Pipe *pipes;
int pipesLength;
ParticleArray *particles;
int particlesLength;
const float pipeWidth = 10.0f;
const float margin = 20.0f;
const float particleWidth = 10.0f;
const float velocity = 10;

void gdiplusStartup()
{
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

void gdiplusShutdown()
{
	Gdiplus::GdiplusShutdown(gdiplusToken);
}

static void initPipes(float **points, int pointsLength)
{
	pipesLength = pointsLength - 1;
	pipes = (Pipe *)malloc(sizeof(Pipe) * pipesLength);
	for (int i = 0; i < pipesLength; ++i)
	{
		float theta = 0;
		directionEnum direction;
		//! horizontal
		if (points[i][1] == points[i + 1][1])
		{
			//! west
			if (points[i][0] > points[i + 1][0])
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
		else if (points[i][0] == points[i + 1][0])
		{
			//! north
			if (points[i][1] > points[i + 1][1])
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
			float dx = points[i][0] - points[i + 1][0];
			float dy = points[i][1] - points[i + 1][1];
			theta = atan2(dy, dx);

			//! bottom right to top left
			if (points[i][0] > points[i + 1][0] && points[i][1] > points[i + 1][1])
			{
				direction = northWest;
			}
			//! bottom left to top right
			else if (points[i][0] < points[i + 1][0] && points[i][1] > points[i + 1][1])
			{
				direction = northEast;
			}
			//! top right to bottom left
			else if (points[i][0] > points[i + 1][0] && points[i][1] < points[i + 1][1])
			{
				direction = southWest;
			}
			//! top left to bottom right
			else
			{
				direction = southEast;
			}
		}
		pipes[i] = { points[i][0], points[i][1], points[i + 1][0], points[i + 1][1], theta, direction };
	}
}

static void initParticles()
{
	particles = (ParticleArray *)malloc(sizeof(ParticleArray) * (pipesLength - 1));
	particlesLength = pipesLength;
	for (int i = 0; i < pipesLength; ++i)
	{
		int numParticles = hypot(fabsf(pipes[i].x1 - pipes[i].x2), fabsf(pipes[i].y1 - pipes[i].y2)) / (pipeWidth + margin) + 2;

		particles[i].particle = (Particle *)malloc(sizeof(Particle) * numParticles);
		particles[i].particleLength = numParticles;
		for (int j = 0; j < numParticles; ++j)
		{
			float x = pipes[i].x1;
			float y = pipes[i].y1;
			float translateX = 0;
			float translateY = 0;
			float mainAxisDiff = (margin + pipeWidth) * (j) - margin;
			float crossAxisDiff = pipeWidth / 2.0f;

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
				x -= dx;
				y -= dy;
				if (pipes[i].direction == northWest || pipes[i].direction == southEast)
				{
					translateX = particleWidth / 2;
					translateY = particleWidth / 2 * -1;
				}
				else if (pipes[i].direction == northEast || pipes[i].direction == southWest)
				{
					translateX = translateY = particleWidth / 2 * -1;
				}
			}
			particles[i].particle[j] = { x, y, translateX, translateY };
		}
	}
}

void initPaint()
{
	int pointsLength = 9;
	float **points = (float **)malloc(sizeof(float *) * pointsLength);
	for (int i = 0; i < pointsLength; ++i)
	{
		points[i] = (float *)malloc(sizeof(float) * 2);
	}
	points[0][0] = 300.0f; points[0][1] = 200.0f;
	points[1][0] = 300.0f; points[1][1] = 100.0f;
	points[2][0] = 100.0f; points[2][1] = 100.0f;
	points[3][0] = 100.0f; points[3][1] = 400.0f;
	points[4][0] = 300.0f; points[4][1] = 400.0f;
	points[5][0] = 100.0f; points[5][1] = 500.0f;
	points[6][0] = 400.0f; points[6][1] = 650.0f;
	points[7][0] = 600.0f; points[7][1] = 500.0f;
	points[8][0] = 500.0f; points[8][1] = 300.0f;
	initPipes(points, pointsLength);
	initParticles();
}

static Gdiplus::Point* getPolyPoints(directionEnum direction, float x1, float y1, float x2, float y2, float theta)
{
	Gdiplus::Point *polyPoints = (Gdiplus::Point *)malloc(sizeof(Gdiplus::Point) * 4);
	switch (direction)
	{
	case north:
	case south:
		polyPoints[0] = { (int)ceil(x1 - pipeWidth / 2.0), (int)ceil(y1) };
		polyPoints[1] = { (int)ceil(x1 - pipeWidth / 2.0), (int)ceil(y2) };
		polyPoints[2] = { (int)ceil(x2 + pipeWidth / 2.0), (int)ceil(y2) };
		polyPoints[3] = { (int)ceil(x2 + pipeWidth / 2.0), (int)ceil(y1) };
		break;
	case east:
	case west:
		polyPoints[0] = { (int)ceil(x1), (int)ceil(y1 - pipeWidth / 2.0) };
		polyPoints[1] = { (int)ceil(x1), (int)ceil(y1 + pipeWidth / 2.0) };
		polyPoints[2] = { (int)ceil(x2), (int)ceil(y2 + pipeWidth / 2.0) };
		polyPoints[3] = { (int)ceil(x2), (int)ceil(y2 - pipeWidth / 2.0) };
		break;
	case northEast:
	case southEast:
	case southWest:
	case northWest:
		polyPoints[0] = { (int)ceil(x1 + cos(theta) * pipeWidth / 2.0), (int)ceil(y1 - sin(theta) * pipeWidth / 2.0) };
		polyPoints[1] = { (int)ceil(x1 - cos(theta) * pipeWidth / 2.0), (int)ceil(y1 + sin(theta) * pipeWidth / 2.0) };
		polyPoints[2] = { (int)ceil(x2 - cos(theta) * pipeWidth / 2.0), (int)ceil(y2 + sin(theta) * pipeWidth / 2.0) };
		polyPoints[3] = { (int)ceil(x2 + cos(theta) * pipeWidth / 2.0), (int)ceil(y2 - sin(theta) * pipeWidth / 2.0) };
	}
	return polyPoints;
}

static void paintMainShape(Gdiplus::Graphics *graphics)
{
	const Gdiplus::Color penColor(255, 0, 0, 255);
	Gdiplus::Pen pen(penColor, pipeWidth);
	graphics->ResetClip();
	Gdiplus::GraphicsPath path;

	for (int i = 0; i < pipesLength; ++i)
	{
		float x1 = pipes[i].x1;
		float y1 = pipes[i].y1;
		float x2 = pipes[i].x2;
		float y2 = pipes[i].y2;
		graphics->DrawLine(&pen, x1, y1, x2, y2);

		Gdiplus::Point *polyPoints = getPolyPoints(pipes[i].direction, x1, y1, x2, y2, pipes[i].theta);
		path.AddPolygon(polyPoints, 4);
		delete polyPoints;
	}
	Gdiplus::Region region(&path);
	graphics->SetClip(&region);
	//Gdiplus::Pen pen2(Gdiplus::Color(255, 0, 255, 0));
	//graphics->DrawPath(&pen2, &path);
}

static void setDxDy(float *dx, float *dy, directionEnum direction, float theta)
{
	*dx = 0;
	*dy = 0;
	switch (direction)
	{
	case north:
		*dy -= velocity;
		break;
	case east:
		*dx += velocity;
		break;
	case south:
		*dy += velocity;
		break;
	case west:
		*dx -= velocity;
		break;
	case northEast:
	case southEast:
	case southWest:
	case northWest:
		*dx -= velocity * cos(theta);
		*dy -= velocity * sin(theta);
		break;
	}
}

static void setParticleInRange(float *x, float *y, int pipesIndex)
{
	switch (pipes[pipesIndex].direction)
	{
	case north:
		if (*y < pipes[pipesIndex].y2 - margin - particleWidth)
			*y = pipes[pipesIndex].y1;
		break;
	case northEast:
		if (*x > pipes[pipesIndex].x2 + margin - particleWidth || *y < pipes[pipesIndex].y2 - margin - particleWidth)
		{
			*x = pipes[pipesIndex].x1 + margin * cos(pipes[pipesIndex].theta);
			*y = pipes[pipesIndex].y1 + margin * sin(pipes[pipesIndex].theta);
		}
		break;
	case east:
		if (*x > pipes[pipesIndex].x2 + margin + particleWidth)
			*x = pipes[pipesIndex].x1;
		break;
	case southEast:
		if (*x > pipes[pipesIndex].x2 + margin + particleWidth || *y > pipes[pipesIndex].y2 + margin + particleWidth)
		{
			*x = pipes[pipesIndex].x1 + margin * cos(pipes[pipesIndex].theta);
			*y = pipes[pipesIndex].y1 + margin * sin(pipes[pipesIndex].theta);
		}
	case south:
		if (*y > pipes[pipesIndex].y2 + margin + particleWidth)
			*y = pipes[pipesIndex].y1;
		break;
	case southWest:
		if (*x < pipes[pipesIndex].x2 - margin - particleWidth || *y > pipes[pipesIndex].y2 + margin + particleWidth)
		{
			*x = pipes[pipesIndex].x1 + margin * cos(pipes[pipesIndex].theta);
			*y = pipes[pipesIndex].y1 + margin * sin(pipes[pipesIndex].theta);
		}
		break;
	case west:
		if (*x < pipes[pipesIndex].x2 - margin - particleWidth)
			*x = pipes[pipesIndex].x1;
		break;
	case northWest:
		if (*x < pipes[pipesIndex].x2 - margin - particleWidth || *y < pipes[pipesIndex].y2 - margin - particleWidth)
		{
			*x = pipes[pipesIndex].x1 + margin * cos(pipes[pipesIndex].theta);
			*y = pipes[pipesIndex].y1 + margin * sin(pipes[pipesIndex].theta);
		}
		break;
	}
}

static void paintParticles(Gdiplus::Graphics *graphics)
{
	Gdiplus::Color circleColor(255, 255, 0, 0);
	Gdiplus::SolidBrush solidBrush(circleColor);
	float particleWidth = pipeWidth;
	for (int i = 0; i < pipesLength; ++i)
	{
		float dx, dy;
		setDxDy(&dx, &dy, pipes[i].direction, pipes[i].theta);
		for (int j = 0; j < particles[i].particleLength; ++j)
		{
			Particle *particle = &particles[i].particle[j];
			particle->x += dx;
			particle->y += dy;
			setParticleInRange(&particle->x, &particle->y, i);

			graphics->TranslateTransform(particle->x, particle->y);
			graphics->RotateTransform(pipes[i].theta * 180.0f / M_PI);
			graphics->TranslateTransform(-1 * particle->x + particle->translateX, -1 * particle->y + particle->translateY);
			graphics->FillRectangle(&solidBrush, particle->x, particle->y, particleWidth, particleWidth);
			graphics->ResetTransform();
		}
	}
}

static void paintRandomStuff(Gdiplus::Graphics *graphics)
{
	Gdiplus::SolidBrush solidBrush(Gdiplus::Color(255, 0, 255, 0));
	graphics->FillEllipse(&solidBrush, 70, 70, 100, 100);
}

void paint(HDC hdc)
{
	Gdiplus::Graphics graphics(hdc);
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	Gdiplus::GraphicsContainer graphicsContainer;
	graphicsContainer = graphics.BeginContainer();
	paintMainShape(&graphics);
	paintParticles(&graphics);
	graphics.EndContainer(graphicsContainer);
	paintRandomStuff(&graphics);
}
