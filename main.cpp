#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <glut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <ctime>

using namespace std;

#define DEG2RAD(a) (a * 0.0174532925)

int WIDTH = 1280;
int HEIGHT = 720;

GLuint tex;
char title[] = "3D Model Loader Sample";

// 3D Projection Options
GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 100;

class Vector3f {
public:
	float x, y, z;

	Vector3f(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) {
		x = _x;
		y = _y;
		z = _z;
	}

	Vector3f operator+(Vector3f v) {
		return Vector3f(x + v.x, y + v.y, z + v.z);
	}

	Vector3f operator-(Vector3f v) {
		return Vector3f(x - v.x, y - v.y, z - v.z);
	}

	Vector3f operator*(float n) {
		return Vector3f(x * n, y * n, z * n);
	}

	Vector3f operator/(float n) {
		return Vector3f(x / n, y / n, z / n);
	}

	Vector3f unit() {
		return *this / sqrt(x * x + y * y + z * z);
	}

	Vector3f cross(Vector3f v) {
		return Vector3f(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
};


class Camera {
public:
	Vector3f eye, center, up;

	Camera(float eyeX = 36.8206, float eyeY = 14.2025f, float eyeZ = 34.1467f, float centerX = 36.4127f, float centerY = 13.3818f, float centerZ = 33.7507f, float upX = 0.f, float upY = 1.f, float upZ = 0.f) {
		eye = Vector3f(eyeX, eyeY, eyeZ);
		center = Vector3f(centerX, centerY, centerZ);
		up = Vector3f(upX, upY, upZ);
	}

	void moveX(float d) {
		Vector3f right = up.cross(center - eye).unit();
		eye = eye + right * d;
		center = center + right * d;
	}

	void moveY(float d) {
		eye = eye + up.unit() * d;
		center = center + up.unit() * d;
	}

	void moveZ(float d) {
		Vector3f view = (center - eye).unit();
		eye = eye + view * d;
		center = center + view * d;
	}

	void rotateX(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + up * sin(DEG2RAD(a));
		up = view.cross(right);
		center = eye + view;
	}

	void rotateY(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + right * sin(DEG2RAD(a));
		right = view.cross(up);
		center = eye + view;
	}

	void look() {
		gluLookAt(
			eye.x, eye.y, eye.z,
			center.x, center.y, center.z,
			up.x, up.y, up.z
		);
	}
};


Vector3f Eye(20, 20, 20);
Vector3f At(0, 20, 0);
Vector3f Up(0, 1, 0);

int cameraZoom = 0;
Camera camera;
// Model Variables
Model_3DS model_person;
Model_3DS model_car;
Model_3DS model_barrier;
Model_3DS model_road;
Model_3DS model_building;
Model_3DS model_tree;
Model_3DS model_building1;
Model_3DS model_bench;
Model_3DS model_ring;
Model_3DS model_arrow;
Model_3DS model_bench_person;
Model_3DS model_bridge;
// Textures
GLTexture tex_ground;
GLTexture tex_road;
GLTexture tex_sun;


double carX = 14;
double carZ = 40;

bool scene1 = true;
//=======================================================================
// Lighting Configuration Function
//=======================================================================

double sunY = 0;
double lightt = 0;
int day = 1;
void InitLightSource()
{
	// Enable Lighting for this OpenGL Program
	glEnable(GL_LIGHTING);

	// Enable Light Source number 0
	// OpengL has 8 light sources
	glEnable(GL_LIGHT0);

	GLfloat lmodel_ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

	GLfloat l0Diffuse[] = { lightt, lightt, lightt, 1.0f };
	GLfloat l0Spec[] = { lightt, lightt, lightt, 1.0f };
	GLfloat l0Ambient[] = { lightt, lightt, lightt, 1.0f };
	GLfloat l0Position[] = { 0.0f, 0.0f, 0.0f, 1 };
	GLfloat l0Direction[] = { -1.0, 0.0, 0.0 };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, l0Diffuse);
	glLightfv(GL_LIGHT0, GL_AMBIENT, l0Ambient);
	glLightfv(GL_LIGHT0, GL_POSITION, l0Position);
	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 30.0);
	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 90.0);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, l0Direction);

	glEnable(GL_COLOR_MATERIAL);

	// Set Material Properties which will be assigned by glColor
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	GLfloat shininess[] = { 96.0f };

	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

	lightt += 0.001  * day;


	if (lightt >= 1) {
		day = -1;
	}
		
	if (lightt <= 0) {
		
		day = 1;
	}
}

void setupCamera() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(100, 1280 / 720, 0.0001, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	camera.look();
}

//=======================================================================
// Material Configuration Function
//======================================================================
void InitMaterial()
{
	// Enable Material Tracking
	glEnable(GL_COLOR_MATERIAL);

	// Sich will be assigneet Material Properties whd by glColor
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// Set Material's Specular Color
	// Will be applied to all objects
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);

	// Set Material's Shine value (0->128)
	GLfloat shininess[] = { 96.0f };
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

void Special(int key, int x, int y) {
	float a = 1.0;

	switch (key) {
	case GLUT_KEY_UP:
		camera.rotateX(a);
		break;
	case GLUT_KEY_DOWN:
		camera.rotateX(-a);
		break;
	case GLUT_KEY_LEFT:
		camera.rotateY(a);
		break;
	case GLUT_KEY_RIGHT:
		camera.rotateY(-a);
		break;
	}

	glutPostRedisplay();
}


void setupLights() {


	GLfloat lmodel_ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

	GLfloat l0Diffuse[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	GLfloat l0Spec[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	GLfloat l0Ambient[] = { 0.1f, 0.0f, 0.0f, 1.0f };
	GLfloat l0Position[] = { 10.0f, 0.0f, 0.0f, 1 };
	GLfloat l0Direction[] = { -1.0, 0.0, 0.0 };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, l0Diffuse);
	glLightfv(GL_LIGHT0, GL_AMBIENT, l0Ambient);
	glLightfv(GL_LIGHT0, GL_POSITION, l0Position);
	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 30.0);
	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 90.0);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, l0Direction);

	GLfloat l1Diffuse[] = { 0.0f, 1.0f, 0.0f, 1.0f };
	GLfloat l1Ambient[] = { 0.0f, 0.1f, 0.0f, 1.0f };
	GLfloat l1Position[] = { 0.0f, 10.0f, 0.0f, 1 };
	GLfloat l1Direction[] = { 0.0, -1.0, 0.0 };
	glLightfv(GL_LIGHT1, GL_DIFFUSE, l1Diffuse);
	glLightfv(GL_LIGHT1, GL_AMBIENT, l1Ambient);
	glLightfv(GL_LIGHT1, GL_POSITION, l1Position);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0);
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 90.0);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, l1Direction);

	GLfloat l2Diffuse[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	GLfloat l2Ambient[] = { 0.0f, 0.0f, 0.1f, 1.0f };
	GLfloat l2Position[] = { 0.0f, 0.0f, 10.0f, 1 };
	GLfloat l2Direction[] = { 0.0, 0.0, -1.0 };
	glLightfv(GL_LIGHT2, GL_DIFFUSE, l2Diffuse);
	glLightfv(GL_LIGHT2, GL_AMBIENT, l2Ambient);
	glLightfv(GL_LIGHT2, GL_POSITION, l2Position);
	glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 30.0);
	glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 90.0);
	glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, l2Direction);


}

//=======================================================================
// OpengGL Configuration Function
//=======================================================================
void myInit(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluPerspective(fovy, aspectRatio, zNear, zFar);
	//*******************************************************************************************//
	// fovy:			Angle between the bottom and top of the projectors, in degrees.			 //
	// aspectRatio:		Ratio of width to height of the clipping plane.							 //
	// zNear and zFar:	Specify the front and back clipping planes distances from camera.		 //
	//*******************************************************************************************//

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
	//*******************************************************************************************//
	// EYE (ex, ey, ez): defines the location of the camera.									 //
	// AT (ax, ay, az):	 denotes the direction where the camera is aiming at.					 //
	// UP (ux, uy, uz):  denotes the upward orientation of the camera.							 //
	//*******************************************************************************************//

	InitMaterial();

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_NORMALIZE);
}

//=======================================================================
// Render Ground Function
//=======================================================================
void RenderGround()
{
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);

	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-100, 0, -100);
	glTexCoord2f(5, 0);
	glVertex3f(100, 0, -100);
	glTexCoord2f(5, 5);
	glVertex3f(100, 0, 200);
	glTexCoord2f(0, 5);
	glVertex3f(-100, 0, 200);
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

//=======================================================================
// Display Function
//=======================================================================
double carx = -5, carz = 96, carDirection = 0;
double carEyeX = 29.1286, carEyeY = 3.18631, carEyeZ = 35.0416;

boolean personVisible = true;
void drawPerson() {
	if (personVisible) {
		glPushMatrix();
		glTranslated(0, 0, -50);
		glScalef(10, 10, 10);
		model_person.Draw();
		glPopMatrix();
	}
	glutPostRedisplay();
}

void drawStreet() {
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex_road.texture[0]);
	glTranslated(20, 1, 20);
	glRotated(45, 0, 1, 0);
	glScaled(0.5, 1, 0.5);
	glBegin(GL_QUADS);

	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(20, 0, -100);
	glTexCoord2f(1, -0);
	glVertex3f(20, 0, 20);
	glTexCoord2f(1, 1);
	glVertex3f(-20, 0, 20);
	glTexCoord2f(0, 1);
	glVertex3f(-20, 0, -100);

	glEnd();

	glPopMatrix();
}

void drawHouse() {
	glPushMatrix();
	glScaled(5, 5, 5);
	glRotated(45, 0, 1, 0);
	glRotatef(90.f, 1, 0, 0);
	model_building.Draw();
	glPopMatrix();
}

void drawTree() {
	glPushMatrix();
	glScaled(2.5, 2.5, 2.5);
	model_tree.Draw();
	glPopMatrix();
}

double ringAngle = 0;
void drawRing() {
	glPushMatrix();
	glTranslated(70, 1, 90);
	glScaled(10, 10, 10);
	glRotated(ringAngle, 0, 1, 0);
	model_ring.Draw();

	glPopMatrix();
	glutPostRedisplay();
}

bool ring1 = true, ring2 = true, ring3 = true, ring4 = true, ring5 = true, ring6 = true;
void drawRings1() {
	if (ring1) {
		glPushMatrix();
		glTranslated(-20 + 4, 0, -20 - 4);
		drawRing();
		glPopMatrix();
	}

	if (ring2) {
		glPushMatrix();
		glTranslated(-30 - 4, 0, -30 + 4);
		drawRing();
		glPopMatrix();
	}

	if (ring3) {
		glPushMatrix();
		glTranslated(-40 + 4, 0, -40 - 4);
		drawRing();
		glPopMatrix();
	}

	if (ring4) {
		glPushMatrix();
		glTranslated(-50 - 4, 0, -50 + 4);
		drawRing();
		glPopMatrix();
	}

	if (ring5) {
		glPushMatrix();
		glTranslated(-60 + 4, 0, -60 - 4);
		drawRing();
		glPopMatrix();
	}

	if (ring6) {
		glPushMatrix();
		glTranslated(-70 - 4, 0, -70 + 4);
		drawRing();
		glPopMatrix();
	}
}

void takeRings() {
	if (carx <= 4 && carx >= 0 && carz >= 69 && carz <= 84)
		ring1 = false;

	if (carx <= -5 && carx >= -11 && carz >= 55 && carz <= 71)
		ring2 = false;

	if (carx <= 4 && carx >= 0 && carz >= 43 && carz <= 57)
		ring3 = false;

	if (carx <= -5 && carx >= -11 && carz >= 31 && carz <= 40)
		ring4 = false;

	if (carx <= 4 && carx >= 0 && carz >= 17 && carz <= 30)
		ring5 = false;

	if (carx <= -5 && carx >= -11 && carz >= 12 && carz <= 21)
		ring6 = false;
}

void pickUp() {
	if (carx >= 30 && carx <= 46 && carz <= 0 && carz >= -7)
		personVisible = false;
}

void drawArrows() {
	glPushMatrix();
	glTranslated(0, 20, 0);
	glScaled(2, 2, 2);
	glRotated(45, 0, 1, 0);
	model_arrow.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslated(40, 20, -40);
	glScaled(2, 2, 2);
	glRotated(45, 0, 1, 0);
	model_arrow.Draw();
	glPopMatrix();
}

double obstacle1X = -17, obstacle1Z = 1;
double obstacle2X = -7, obstacle2Z = 1;
bool obstacle1 = true, obstacle2 = false;
void obstacleCar() {
	if (obstacle1) {
		glPushMatrix();
		glRotated(45, 0, 1, 0);
		glTranslatef(obstacle1X, 5, obstacle1Z);
		glScalef(1.5, 1.5, 1.5);
		model_car.Draw();
		glPopMatrix();
	}
	if (obstacle2) {
		glPushMatrix();
		glRotated(45, 0, 1, 0);
		glTranslatef(obstacle2X, 5, obstacle2Z);
		glScalef(1.5, 1.5, 1.5);
		model_car.Draw();
		glPopMatrix();
	}
}

void updateObstacles() {
	if (obstacle1) {
		if (obstacle1Z < 106) {
			obstacle1Z += 0.1;
		}
		else {
			obstacle1 = false;
			obstacle2 = true;
			obstacle1X = -17;
			obstacle1Z = 1;
		}
	}
	else {
		if (obstacle2Z < 106) {
			obstacle2Z += 0.1;
		}
		else {
			obstacle2 = false;
			obstacle1 = true;
			obstacle2X = -7;
			obstacle2Z = 1;
		}
	}
}

void didCollide() {
	if (obstacle1) {
		if (carx >= -9 && carx <= -3 && carz - obstacle1Z <= 2 && carz - obstacle1Z >= -20) {
			carx = -5;
			carz = 96;
		}
	}


	else {
		if (carx >= -1 && carx <= 6 && carz - obstacle2Z <= 2 && carz - obstacle2Z >= -20) {
			carx = -5;
			carz = 96;
		}
	}
}

void sun() {
	if (day == -1) 
		sunY -= 0.015;
	
	else 
		sunY += 0.015;
	
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, tex_sun.texture[0]);
	glTranslated(-20, sunY, 20);
	glRotatef(-90, 1, 0, 0);
	GLUquadricObj* sphere = gluNewQuadric();
	gluQuadricTexture(sphere, true);
	gluQuadricNormals(sphere, GLU_SMOOTH);
	gluSphere(sphere, 7, 50, 50);
	gluDeleteQuadric(sphere);
	glPopMatrix();

	glutPostRedisplay();
}

void changeScene() {
	if (carx >= 72 && carx <= 76 && carz <= 3 && carz >= -8) {
		scene1 = false; 
		camera.eye.x = 37.8253; camera.eye.y = 3.4058; camera.eye.z = 18.3971;
		camera.center.x = 37.4174; camera.center.y = 2.5851; camera.center.z = 18.0011;
	}
}

void myDisplay(void)
{
	setupCamera();
	
	if (scene1) {
		changeScene();
		didCollide();
		takeRings();
		pickUp();
		ringAngle += 0.5;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		

		glPushMatrix();
		sun();
		glPopMatrix();

		glPushMatrix();
		glTranslated(0, carx, 0);
		InitLightSource();
		glPopMatrix();


		glPushMatrix();
		glScaled(0.3, 0.3, 0.3);
		// Draw Ground
		glPushMatrix();
		glTranslated(0, -2, 0);
		glRotatef(45, 0, 1, 0);
		RenderGround();
		glPopMatrix();

		glPushMatrix();
		glTranslated(40, 0, 40);

		// Draw Car Model
		glPushMatrix();
		glRotated(45, 0, 1, 0);
		glTranslatef(carx - 10, 5, carz + 10);
		glRotated(45 + 180, 0, 1, 0);
		glScalef(3, 3, 3);
		glRotated(carDirection - 45, 0, 1, 0);
		model_car.Draw();
		glPopMatrix();

		glPushMatrix();
		updateObstacles();
		obstacleCar();
		glPopMatrix();

		// Draw Person Model

		drawPerson();

		glPushMatrix();
		glTranslated(-10, 0, -50);
		drawTree();
		glPopMatrix();

		glPushMatrix();
		glTranslated(-45, 0, 5);
		drawHouse();
		glPopMatrix();

		glPushMatrix();
		glTranslated(10, 0, 30);
		drawStreet();
		glPopMatrix();

		glPushMatrix();
		glTranslated(40, 0, 0);
		drawStreet();
		glPopMatrix();

		glPushMatrix();
		glTranslated(50, 0, 70);
		drawStreet();
		glPopMatrix();

		glPushMatrix();
		glTranslated(80, 0, 40);
		drawStreet();
		glPopMatrix();

		glPushMatrix();
		glTranslated(-20, 1, 20);
		glRotated(90, 0, 1, 0);
		drawStreet();
		glPopMatrix();

		glPushMatrix();
		glTranslated(20, 1, -20);
		glRotated(90, 0, 1, 0);
		drawStreet();
		glPopMatrix();

		glPushMatrix();
		drawRings1();
		glPopMatrix();


		glPushMatrix();
		drawArrows();
		glPopMatrix();


		glPushMatrix();
		glTranslated(40, 0, -40);
		glScaled(3, 3, 3);
		glRotated(90 + 45 + 180, 0, 1, 0);
		model_building1.Draw();
		glPopMatrix();

		glPushMatrix();
		glTranslated(70, 0, -10);
		glScaled(3, 3, 3);
		glRotated(90 + 45 + 180, 0, 1, 0);
		model_building1.Draw();
		glPopMatrix();

		glPushMatrix();
		glTranslated(100, 0, 20);
		glScaled(3, 3, 3);
		glRotated(90 + 45 + 180, 0, 1, 0);
		model_building1.Draw();
		glPopMatrix();


		glPushMatrix();
		glTranslated(-10, 0, 50);
		glScaled(3, 3, 3);
		glRotated(135, 0, 1, 0);
		model_building1.Draw();
		glPopMatrix();

		glPushMatrix();
		glTranslated(20, 0, 80);
		glScaled(3, 3, 3);
		glRotated(135, 0, 1, 0);
		model_building1.Draw();
		glPopMatrix();

		glPushMatrix();
		glTranslated(50, 0, 110);
		glScaled(3, 3, 3);
		glRotated(135, 0, 1, 0);
		model_building1.Draw();
		glPopMatrix();

		glPushMatrix();
		glTranslated(-5, 0, -15);
		glScaled(0.1, 0.1, 0.1);
		glRotated(45, 0, 1, 0);
		glRotated(-90, 1, 0, 0);
		model_bench.Draw();
		glPopMatrix();
		glPopMatrix();

		//sky box
		glPushMatrix();


		glPopMatrix();
		glPopMatrix();

		glPushMatrix();
		glTranslated(0, 0, 0);
		GLUquadricObj* qobj;
		qobj = gluNewQuadric();
		glTranslated(30, 0, 20);
		glRotated(90, 1, 0, 1);
		glBindTexture(GL_TEXTURE_2D, tex);
		gluQuadricTexture(qobj, true);
		gluQuadricNormals(qobj, GL_SMOOTH);
		gluSphere(qobj, 55, 100, 100);
		gluDeleteQuadric(qobj);
		glPopMatrix();

		glutSwapBuffers();
	}

	else {
		//setupCamera();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glPushMatrix();
		glRotated(45, 0, 1, 0);
		glTranslatef(carX, 2, carZ);
		glRotated(225, 0, 1, 0);
		glScalef(0.1, 0.1, 0.1);
		glRotated(carDirection - 45, 0, 1, 0);
		model_car.Draw();
		glPopMatrix();

		// Draw bridge Model 
		glPushMatrix();
		glTranslated(30, 0, 10);
		glRotatef(-45, 0, 1, 0);
		model_bridge.Draw();
		glPopMatrix();

		//sky box
		glPushMatrix();
		GLUquadricObj* qobj;
		qobj = gluNewQuadric();
		glTranslated(50, 0, 0);
		glRotated(90, 1, 0, 1);
		glBindTexture(GL_TEXTURE_2D, tex);
		gluQuadricTexture(qobj, true);
		gluQuadricNormals(qobj, GL_SMOOTH);
		gluSphere(qobj, 55, 100, 100);
		gluDeleteQuadric(qobj);

		glPopMatrix();

		glutSwapBuffers();
	}

}

//=======================================================================
// Keyboard Function
//=======================================================================
void myKeyboard(unsigned char button, int x, int y)
{
	float d = 1;
	
	switch (button) {

	case '1':
		camera.eye.x = carEyeX;
		camera.eye.z = carEyeZ;
		camera.eye.y = carEyeY;
		break;

	case 'o':
		carDirection = 0;
		carz -= 1;
		if (!scene1)
			carZ -= 0.1;
		carEyeX -= 1;
		carEyeZ -= 1;
		//camera.moveY(d);
		//camera.moveZ(d);
		break;

	case 'l':
	
		carDirection = 180;
		carz += 1;
		if (!scene1)
			carZ += 0.1;
		carEyeX += 1;
		carEyeZ += 1;
		//camera.moveY(-d);
		//camera.moveZ(-d);
		break;

	case ';':
		if(scene1)
			carDirection = -90;
		carx += 1;
		if (!scene1)
			carX += 0.1;
		carEyeX += 1;
		carEyeZ -= 1;
		break;

	case 'k':
		if(scene1)
			carDirection = 90;
		carx -= 1;
		if (!scene1)
			carX -= 0.1;

		carEyeX -= 1;
		carEyeZ += 1;
		break;

	case 'w':
		camera.moveY(d);
		break;
	case 's':
		camera.moveY(-d);
		break;
	case 'a':
		camera.moveX(d);
		break;
	case 'd':
		camera.moveX(-d);
		break;
	case 'q':
		camera.moveZ(d);
		break;
	case 'e':
		camera.moveZ(-d);
		break;
	}
	

	cout << camera.eye.x ;
	cout << " ";
	cout << camera.eye.y ;
	cout << " ";
	cout << camera.eye.z << endl;

	cout << camera.center.x;
	cout << " ";
	cout << camera.center.y;
	cout << " ";
	cout << camera.center.z << endl;

	glutPostRedisplay();
}

//=======================================================================
// Motion Function
//=======================================================================
void myMotion(int x, int y)
{
	y = HEIGHT - y;

	if (cameraZoom - y > 0)
	{
		Eye.x += -0.1;
		Eye.z += -0.1;
	}
	else
	{
		Eye.x += 0.1;
		Eye.z += 0.1;
	}

	cameraZoom = y;

	glLoadIdentity();	//Clear Model_View Matrix

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);	//Setup Camera with modified paramters

	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glutPostRedisplay();	//Re-draw scene 
}

//=======================================================================
// Mouse Function
//=======================================================================
void myMouse(int button, int state, int x, int y)
{
	y = HEIGHT - y;

	if (state == GLUT_DOWN)
	{
		cameraZoom = y;
	}
}

//=======================================================================
// Reshape Function
//=======================================================================
void myReshape(int w, int h)
{
	if (h == 0) {
		h = 1;
	}

	WIDTH = w;
	HEIGHT = h;

	// set the drawable region of the window
	glViewport(0, 0, w, h);

	// set up the projection matrix 
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (GLdouble)WIDTH / (GLdouble)HEIGHT, zNear, zFar);

	// go back to modelview matrix so we can move the objects about
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
}

//=======================================================================
// Assets Loading Function
//=======================================================================
void LoadAssets()
{
	// Loading Model files
	model_person.Load("Models/Man/casual.3ds");

	model_car.Load("Models/Car-Model/car.3ds");

	model_barrier.Load("Models/wall/wall.3ds");

	model_building.Load("Models/house/house.3ds");

	model_tree.Load("Models/tree/Tree1.3ds");

	model_building1.Load("Models/house/house1.3ds");

	model_bench.Load("Models/bench/bench.3ds");

	model_ring.Load("Models/ring/ring.3ds");

	model_arrow.Load("Models/arrow/arrow.3ds");

	model_bench_person.Load("Models/benchperson/benchperson.3ds");

	model_bridge.Load("Models/bridge/brije.3ds");
	// Loading texture files
	tex_ground.Load("Textures/ground2.bmp");
	tex_sun.Load("Textures/sand.bmp");
	tex_road.Load("Textures/road.bmp");
	loadBMP(&tex, "Textures/blu-sky-3.bmp", true);
}

//=======================================================================
// Main Function
//=======================================================================
void main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(WIDTH, HEIGHT);

	glutInitWindowPosition(100, 150);

	glutCreateWindow(title);

	glutSpecialFunc(Special);

	glutDisplayFunc(myDisplay);

	glutKeyboardFunc(myKeyboard);

	glutMotionFunc(myMotion);

	glutMouseFunc(myMouse);

	glutReshapeFunc(myReshape);

	myInit();

	LoadAssets();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);

	glutMainLoop();
}

