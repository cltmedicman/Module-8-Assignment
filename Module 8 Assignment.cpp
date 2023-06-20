#include <GLFW/glfw3.h>
#include <linmath.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>

using namespace std;

const float DEG2RAD = 3.14159 / 180;

void processInput(GLFWwindow* window);

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE };
enum ONOFF { ON, OFF };


class Brick
{
public:
	float red, green, blue;
	float x, y, width;
	BRICKTYPE brick_type;
	ONOFF onoff;
	int hits; // keep track of number of hits
	int maxHits; // maximum hits a brick can take

	Brick(BRICKTYPE bt, float xx, float yy, float ww, float rr, float gg, float bb, int mh)
	{
		brick_type = bt; x = xx; y = yy, width = ww; red = rr, green = gg, blue = bb;
		onoff = ON;
		hits = 0; // initialize hits to 0
		maxHits = mh; // set the maximum hits a brick can take
	};

	void hit() {
		hits++;
		if (hits >= maxHits) {
			onoff = OFF;
		}
		else {
			// modify color each time it's hit (as an example, we're decreasing the green component)
			green -= 0.1f;
			if (green < 0.0f) green = 0.0f;
		}
	}

	void drawBrick()
	{
		if (onoff == ON)
		{
			double halfside = width / 2;

			glColor3d(red, green, blue);
			glBegin(GL_POLYGON);

			glVertex2d(x + halfside, y + halfside);
			glVertex2d(x + halfside, y - halfside);
			glVertex2d(x - halfside, y - halfside);
			glVertex2d(x - halfside, y + halfside);

			glEnd();
		}
	}
};


class Circle
{
public:
	float red, green, blue;
	float radius;
	float x;
	float y;
	float speed = 0.03;
	int direction; // 1=up 2=right 3=down 4=left 5 = up right   6 = up left  7 = down right  8= down left
	float acceleration = 0;
	float friction = 0;
	bool alive;

	Circle(double xx, double yy, double rr, int dir, float rad, float r, float g, float b)
	{
		x = xx;
		y = yy;
		radius = rr;
		red = r;
		green = g;
		blue = b;
		radius = rad;
		direction = dir;
		alive = true;
	}

	bool checkCollision(Circle* other) {
		// Simple circle-circle collision detection based on distance between centers
		float dist = sqrt(pow(x - other->x, 2) + pow(y - other->y, 2));
		return dist < radius + other->radius;
	}

	void CheckCollision(Brick* brk)
	{
		if (brk->brick_type == REFLECTIVE)
		{
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width))
			{
				/*direction = GetRandomDirection();
				x = x + 0.03;
				y = y + 0.04;*/

				// Implement proper physics reflection
				float normalAngle;

				// Determine the normal based on brick position
				// (you might want to implement a more precise collision detection)
				if (x <= brk->x) normalAngle = 0;     // from left
				else if (y >= brk->y) normalAngle = 90; // from top
				else if (x > brk->x) normalAngle = 180;  // from right
				else normalAngle = 270; // from bottom

				// Reflect the circle's direction using physics rule: theta' = -theta + 2*normal
				direction = -direction + 2 * normalAngle;

				// Apply friction
				speed -= friction;
				// Clamp speed to 0 if negative
				if (speed < 0) speed = 0;

				// Apply acceleration to change speed
				speed += acceleration;
			}
		}
		else if (brk->brick_type == DESTRUCTABLE)
		{
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width))
			{
				// brk->onoff = OFF;
				brk->hit(); // call the hit method on the brick
			}
		}

	}

	int GetRandomDirection()
	{
		return (rand() % 8) + 1;
	}

	void MoveOneStep()
	{
		if (direction == 1 || direction == 5 || direction == 6)  // up
		{
			if (y > -1 + radius)
			{
				y -= speed;
			}
			else
			{
				direction = GetRandomDirection();
			}
		}

		if (direction == 2 || direction == 5 || direction == 7)  // right
		{
			if (x < 1 - radius)
			{
				x += speed;
			}
			else
			{
				direction = GetRandomDirection();
			}
		}

		if (direction == 3 || direction == 7 || direction == 8)  // down
		{
			if (y < 1 - radius) {
				y += speed;
			}
			else
			{
				direction = GetRandomDirection();
			}
		}

		if (direction == 4 || direction == 6 || direction == 8)  // left
		{
			if (x > -1 + radius) {
				x -= speed;
			}
			else
			{
				direction = GetRandomDirection();
			}
		}
	}

	void DrawCircle()
	{
		if (alive) {
			glColor3f(red, green, blue);
			glBegin(GL_POLYGON);
			for (int i = 0; i < 360; i++) {
				float degInRad = i * DEG2RAD;
				glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
			}
		}
		/*glColor3f(red, green, blue);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 360; i++) {
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
		}*/
		glEnd();
	}
};


vector<Circle> world;

Brick paddle(REFLECTIVE, 0.0, -0.8, 0.2, 1, 1, 1, 0);
const float PADDLE_SPEED = 0.05f;

int main(void) {
	srand(time(NULL));

	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(480, 480, "Random World of Circles", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	Brick brick(REFLECTIVE, -0.4, 0.2, 0.2, 1, 1, 0, 0);
	Brick brick2(DESTRUCTABLE, -0.2, 0, 0.2, 1, 1, 0, 10);
	Brick brick3(DESTRUCTABLE, 0, -0.2, 0.2, 1, 1, 0, 10);
	Brick brick4(REFLECTIVE, 0.2, 0, 0.2, 1, 1, 0, 0);
	Brick brick5(REFLECTIVE, 0.4, 0.2, 0.2, 1, 1, 0, 0);
	Brick paddle(REFLECTIVE, 0.0, -0.8, 0.2, 1, 1, 1, 0);
	
	

	while (!glfwWindowShouldClose(window)) {
		//Setup View
		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window);

		//Movement
		for (int i = 0; i < world.size(); i++)
		{
			world[i].CheckCollision(&brick);
			world[i].CheckCollision(&brick2);
			world[i].CheckCollision(&brick3);
			world[i].CheckCollision(&brick4);
			world[i].CheckCollision(&brick5);
			world[i].CheckCollision(&paddle);

			for (int j = i + 1; j < world.size(); j++)
			{
				if (world[i].checkCollision(&world[j]))
				{
					world[i].alive = false;
					world[j].alive = false;
				}
			}

			world[i].MoveOneStep();
			world[i].DrawCircle();

		}

		brick.drawBrick();
		brick2.drawBrick();
		brick3.drawBrick();
		brick4.drawBrick();
		brick5.drawBrick();
		paddle.drawBrick();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate;
	exit(EXIT_SUCCESS);
}


void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		paddle.x -= PADDLE_SPEED;  // Move the paddle to the left
	}

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		paddle.x += PADDLE_SPEED;  // Move the paddle to the right
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		double r, g, b;
		r = rand() / 10000;
		g = rand() / 10000;
		b = rand() / 10000;
		Circle B(0, 0, 02, 2, 0.05, r, g, b);
		world.push_back(B);
	}
}
