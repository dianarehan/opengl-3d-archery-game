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

//camera variables for free movement and rotation
float camX = 0.0f, camY = 2.0f, camZ = 6.0f;
float camYaw = 0.0f, camPitch = 0.0f;


void DrawWall(float x, float y, float z, float width, float height, float thickness) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(width, height, thickness);
    glutSolidCube(1);
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
    }

    glutPostRedisplay();
}

void DrawGround(float x, float y, float z, float size) {
	glPushMatrix();
	glTranslatef(x, y, z);
	glScalef(size, 0.1f, size);
	glutSolidCube(1);
	glPopMatrix();
}

void Display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set the camera
    glLoadIdentity();
    gluLookAt(camX, camY, camZ, camX + sin(camYaw), camY + sin(camPitch), camZ - cos(camYaw), 0.0f, 1.0f, 0.0f);


    glColor3f(0.6f, 0.6f, 0.6f);
	DrawGround(0.0f, 0.0f, 0.0f, groundSize);

    glColor3f(0.8f, 0.8f, 0.8f);
    DrawWall(0.0f, wallHeight / 2, -groundSize / 2, groundSize, wallHeight, wallThickness); //back
    glColor3f(0.9f, 0.9f, 0.8f);
    DrawWall(-groundSize / 2, wallHeight / 2, 0.0f, wallThickness, wallHeight, groundSize); //left
    glColor3f(0.8f, 0.7f, 0.7f);
    DrawWall(groundSize / 2, wallHeight / 2, 0.0f, wallThickness, wallHeight, groundSize); //right

    glFlush();
}

void Anim() {
    rotAng += 0.01;

    glutPostRedisplay();
}

void main(int argc, char** argv) {
    glutInit(&argc, argv);

    glutInitWindowSize(xCord, yCord);
    glutInitWindowPosition(150, 50);

    glutCreateWindow("Game");
    glutDisplayFunc(Display);
    glutIdleFunc(Anim);

    glutKeyboardFunc(Keyboard);

    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)xCord / (float)yCord, 0.1f, 300.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camX, camY, camZ, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    glutMainLoop();
}