#include <stdio.h>
#include <glut.h>
#include <math.h>

float rotAng;
//screen size
int xCord = 1000, yCord = 700;

//ground and wall dimensions
const float groundSize = 5.0f;
const float wallHeight = 3.0f;
const float wallThickness = 0.1f;
const float ringThickness = 3.0f;

//camera variables for free movement and rotation
float camX = 0.0f, camY = 2.0f, camZ = 5.5f;
float camYaw = 0.0f, camPitch = -0.2f;
enum View { FREE_VIEW, TOP_VIEW, SIDE_VIEW, FRONT_VIEW };
View currentView = FREE_VIEW;

//functions signatures
void InitializeGLUT(int argc, char** argv);
void InitializeCallbacks();
void InitializeOpenGL();

void DrawCircle(float x, float y, float radius) {
    glLineWidth(ringThickness);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 360; i++) {
        float theta = i * 3.14159f / 180.0f;
        glVertex2f(x + radius * cos(theta), y + radius * sin(theta));
    }
    glEnd();
    glLineWidth(1.0f);
}

void DrawOlympicRings() {
    const float radius = 0.5f;
    const float offset = 1.2f * radius;

    glColor3f(0.0f, 0.0f, 1.0f); // Blue
    DrawCircle(-offset, 0.0f, radius);

    glColor3f(0.0f, 0.0f, 0.0f); // Black
    DrawCircle(0.0f, 0.0f, radius);

    glColor3f(1.0f, 0.0f, 0.0f); // Red
    DrawCircle(offset, 0.0f, radius);

    glColor3f(1.0f, 1.0f, 0.0f); // Yellow
    DrawCircle(-offset / 2, -radius, radius);

    glColor3f(0.0f, 1.0f, 0.0f); // Green
    DrawCircle(offset / 2, -radius, radius);
}

void DrawWall(float x, float y, float z, float width, float height, float thickness) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(width, height, thickness);
    glutSolidCube(1);
    glPopMatrix();
}

void DrawWallWithRings(float x, float y, float z, float width, float height, float thickness, float rotationAngle = 0.0f) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(width, height, thickness);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    if (rotationAngle == 0.0f) {
        // Back wall
        glTranslatef(x, y, z + thickness / 2 + 0.01f); //slightly in front of the wall
    }
    else if (rotationAngle == 90.0f) {
        // Left wall
		glScaled(1.0f, 1.0f, -1.0f); //reflecting on z axis
        glTranslatef(thickness / 2 - 0.06f, y, z); 
    }
    else if (rotationAngle == -90.0f) {
        // Right wall
		glScaled(1.0f, 1.0f, -1.0f); //reflecting on z axis
        glTranslatef(-thickness / 2 + 0.06f, y, z); //slightly in front of the wall
    }
    glRotatef(rotationAngle, 0.0f, 1.0f, 0.0f); // Rotate the rings to align with the wall
	glScaled(0.5f, 0.5f, 0.5f);
    DrawOlympicRings();
    glPopMatrix();
}

void Keyboard(unsigned char key, int x, int y) {
    const float moveSpeed = 0.1f;
    const float turnSpeed = 0.05f;

    switch (key) {
    case 'w': //z positive
        camX += moveSpeed * sin(camYaw);
        camZ -= moveSpeed * cos(camYaw);
        break;
    case 's': //z negative
        camX -= moveSpeed * sin(camYaw);
        camZ += moveSpeed * cos(camYaw);
        break;
    case 'a': //x negative
        camX -= moveSpeed * cos(camYaw);
        camZ -= moveSpeed * sin(camYaw);
        break;
    case 'd': //x positive
        camX += moveSpeed * cos(camYaw);
        camZ += moveSpeed * sin(camYaw);
        break;
    case 'q': //y positive
        camY += moveSpeed;
        break;
    case 'e': //y negative
        camY -= moveSpeed;
        break;
    case 'j': //rotate on positive y
        camYaw -= turnSpeed;
        break;
    case 'l': //rotate on negative y
        camYaw += turnSpeed;
        break;
    case 'i': //rotate on positive x
        camPitch += turnSpeed;
        break;
    case 'k': //rotate on negative x
        camPitch -= turnSpeed;
        break;
    case 'f': //free view
        currentView = FREE_VIEW;
        break;
	case 't': //top view
		currentView = TOP_VIEW;
		break;
	case 'y': //side view
		currentView = SIDE_VIEW;
		break;
	case 'u': //front view
		currentView = FRONT_VIEW;
		break;
    }
	printf("camX: %f, camY: %f, camZ: %f, camYaw: %f, camPitch: %f\n", camX, camY, camZ, camYaw, camPitch);
    glutPostRedisplay();
}

void DrawGround(float x, float y, float z, float size) {
	glPushMatrix();
	glTranslatef(x, y, z);
	glScalef(size, 0.1f, size);
	glutSolidCube(1);
	glPopMatrix();
}

void SetCamera() {
    glLoadIdentity();
    switch (currentView) {
    case TOP_VIEW:
        gluLookAt(0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f);
        break;
    case SIDE_VIEW:
        gluLookAt(10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
        break;
    case FRONT_VIEW:
        gluLookAt(0.0f, 0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
        break;
    case FREE_VIEW:
        gluLookAt(camX, camY, camZ, camX + sin(camYaw), camY + sin(camPitch), camZ - cos(camYaw), 0.0f, 1.0f, 0.0f);
        break;
    }
}

void Display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//update the camera freely or to one of the views
    SetCamera();

    glColor3f(0.6f, 0.6f, 0.6f);
	DrawGround(0.0f, 0.0f, 0.0f, groundSize);

    glColor3f(0.8f, 0.8f, 0.8f);
    DrawWallWithRings(0.0f, wallHeight / 2, -groundSize / 2, groundSize, wallHeight, wallThickness); //back
    glColor3f(0.9f, 0.9f, 0.8f);
    DrawWallWithRings(-groundSize/2,  wallHeight / 2, 0.0f, wallThickness, wallHeight, groundSize, 90.0f); //left
    glColor3f(0.8f, 0.7f, 0.7f);
    DrawWallWithRings(groundSize/2 , wallHeight / 2, 0.0f, wallThickness, wallHeight, groundSize, -90.0f); //right

    glFlush();
}

void Anim() {
    rotAng += 0.01;

    glutPostRedisplay();
}

void main(int argc, char** argv) {

    InitializeGLUT(argc, argv);
    InitializeCallbacks();
    InitializeOpenGL();
    glutMainLoop();
    glutMainLoop();
}

void InitializeGLUT(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(xCord, yCord);
    glutInitWindowPosition(150, 50);
    glutCreateWindow("Game");
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
}

void InitializeCallbacks() {
    glutDisplayFunc(Display);
    glutIdleFunc(Anim);
    glutKeyboardFunc(Keyboard);
}

void InitializeOpenGL() {
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)xCord / (float)yCord, 0.1f, 300.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camX, camY, camZ, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
}