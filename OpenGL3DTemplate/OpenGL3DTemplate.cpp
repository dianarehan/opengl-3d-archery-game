#include <iostream>
#include <stdio.h>
#include <glut.h>
#include <math.h>
#include <irrKlang.h>
#include <thread>
using namespace irrklang;

float rotAng;
//screen size
int xCord = 1000, yCord = 700;

//ground and wall dimensions
const float groundSize = 5.0f;
const float wallHeight = 3.0f;
const float wallThickness = 0.1f;
const float ringThickness = 3.0f;

//wall positions
float backWallX = 0.0f, backWallY = wallHeight / 2, backWallZ = -groundSize / 2;
float leftWallX = -groundSize / 2, leftWallY = wallHeight / 2, leftWallZ = 0.0f;
float rightWallX = groundSize / 2, rightWallY = wallHeight / 2, rightWallZ = 0.0f;

//camera variables for free movement and rotation
float camX = 0.0f, camY = 2.0f, camZ = 5.5f;
float camYaw = 0.0f, camPitch = -0.2f;
enum View { FREE_VIEW, TOP_VIEW, SIDE_VIEW, FRONT_VIEW };
View currentView = FREE_VIEW;

//functions signatures
void InitializeGLUT(int argc, char** argv);
void InitializeCallbacks();
void InitializeOpenGL();
void playSound(const char* soundFile);
void InitializeSound();

//sound data
ISoundEngine* engine;

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

void DrawArrow(float startX, float startY, float startZ, float length) {
    glLineWidth(2);
    // Draw the arrow shaft
    glBegin(GL_LINES);
    glColor3f(0.2588f, 0.2431f, 0.2510f);
    glVertex3f(startX, startY, startZ);  // Arrow start
    glVertex3f(startX + length, startY, startZ);  // Arrow end
    glEnd();

    // Draw the arrowhead
    glBegin(GL_TRIANGLES);
    glColor3f(0.6784f, 0.6392f, 0.6588f);
    glVertex3f(startX + length, startY, startZ);  // Tip of the arrowhead
    glVertex3f(startX + length - 0.05f, startY + 0.02f, startZ); // Bottom left of the arrowhead
    glVertex3f(startX + length - 0.05f, startY - 0.02f, startZ); // Bottom right of the arrowhead
    glEnd();

    // Draw the fletching (feathers)
    glBegin(GL_TRIANGLES);
    glColor3f(0.6314f, 0.0745f, 0.1412f);
    glVertex3f(startX, startY, startZ);  // Base of the fletching
    glVertex3f(startX - 0.05f, startY + 0.02f, startZ); // Top left of the fletching
    glVertex3f(startX - 0.05f, startY - 0.02f, startZ); // Top right of the fletching
    glEnd();
}

void DrawBow(float x, float y, float z) {
    // Draw the main curve of the bow using a series of small line segments
    glPushMatrix();
    glLineWidth(2);
    glTranslatef(x, y-0.2, z); // Position the bow relative to the player
    glRotatef(270, 0.0f, 1.0f, 0.0f); // Rotate the bow on the y-axis
    glRotatef(-90, 0.0f, 0.0f, 1.0f); // Rotate the bow on the x-axis
    glColor3f(0.55f, 0.27f, 0.07f); // Brown color for the bow

    glBegin(GL_LINE_STRIP); // Use a line strip to form a curve
    float bowRadius = 0.4f;
    for (int i = -9; i <= 9; i++) { // Draw half-circle to represent bow curve
        float angle = (float)i / 18.0f * 3.14159f; // Half circle (180 degrees)
        glVertex3f(bowRadius * cos(angle), bowRadius * sin(angle), 0.0f);
    }
    glEnd();

    // Draw the bowstring
    glBegin(GL_LINES);
    glColor3f(0.8588f, 0.6706f, 0.6941f);
    glVertex3f(0.0, bowRadius, 0.0f);  // Top of the bow
    glVertex3f(0.0, -bowRadius, 0.0f); // Bottom of the bow
    glEnd();

    // Draw the arrow
    DrawArrow(0.05f, 0.0f, 0.0f, 0.35f);

    glPopMatrix();
}

void DrawPlayer(float x, float y, float z) {
    // Draw the body
    glPushMatrix();
    glTranslatef(x, y, z);
    glColor3f(0.5f, 0.6f, 1.0f);
    glScalef(0.5f, 0.8f, 0.2f);
    glutSolidCube(1);
    glPopMatrix();
    DrawBow(x - 0.35f, y, z); // Position the bow near the player's left hand
    // Draw the head
    glPushMatrix();
    glTranslatef(x, y + 0.68f, z);
    glColor3f(1.0f, 0.8f, 0.6f); // Skin color for the head
    glutSolidSphere(0.25f, 20, 20);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(x - 0.1f, y + 0.75f, z + 0.2f); // Adjust position for left eye
    glColor3f(0.0f, 0.0f, 0.0f); // Black color for the eyes
    glutSolidSphere(0.05f, 10, 10); // Left eye
    glPopMatrix();

    // Draw the right eye
    glPushMatrix();
    glTranslatef(x + 0.1f, y + 0.75f, z + 0.2f); // Adjust position for right eye
    glColor3f(0.0f, 0.0f, 0.0f); // Black color for the eyes
    glutSolidSphere(0.05f, 10, 10); // Right eye
    glPopMatrix();

    // Draw the mouth
    glPushMatrix();
    glTranslatef(x, y + 0.68f, z + 0.25f); // Adjust position for the mouth
    glColor3f(0.8f, 0.0f, 0.0f); // Red color for the mouth
    glRotatef(180.0f, 1.0f, 0.0f, 0.0f); // Rotate to face forward

    // Draw half-circle mouth (smile)
    glBegin(GL_POLYGON);
    const int num_segments = 20; // Number of segments for a smooth curve
    float radius = 0.08f; // Radius of the mouth
    for (int i = 0; i <= num_segments; i++) {
        float angle = (float)i / (float)num_segments * 3.14159f; // Half circle (180 degrees)
        glVertex2f(radius * cos(angle), radius * sin(angle));
    }
    glEnd();

    glPopMatrix();

    // Draw the left leg
    glPushMatrix();
    glTranslatef(x - 0.15f, y - 0.7f, z);
    glColor3f(0.0f, 0.0f, 1.0f); // Blue color for the legs
    glScalef(0.2f, 0.6f, 0.2f);
    glutSolidCube(1);
    glPopMatrix();

    // Draw the right leg
    glPushMatrix();
    glTranslatef(x + 0.15f, y - 0.7f, z);
    glColor3f(0.0f, 0.0f, 1.0f); // Blue color for the legs
    glScalef(0.2f, 0.6f, 0.2f);
    glutSolidCube(1);
    glPopMatrix();

    // Draw the left hand
    glPushMatrix();
    glTranslatef(x - 0.30f, y+0.1, z);
    glColor3f(1.0f, 0.8f, 0.6f); // Skin color for the hands
    glScalef(0.1f, 0.6f, 0.2f);
    glutSolidCube(1);
    glPopMatrix();

    // Draw the right hand
    glPushMatrix();
    glTranslatef(x + 0.30f, y+0.1, z);
    glColor3f(1.0f, 0.8f, 0.6f); // Skin color for the hands
    glScalef(0.1f, 0.6f, 0.2f);
    glutSolidCube(1);
    glPopMatrix();
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

    glColor3f(0.125f, 0.271f, 0.094f);
    DrawGround(0.0f, 0.0f, 0.0f, groundSize); //ground

    glColor3f(0.8f, 0.8f, 0.8f);
    DrawWallWithRings(backWallX, backWallY, backWallZ, groundSize, wallHeight, wallThickness); //back
    glColor3f(0.9f, 0.9f, 0.8f);
    DrawWallWithRings(leftWallX, leftWallY, leftWallZ, wallThickness, wallHeight, groundSize, 90.0f); //left
    glColor3f(0.8f, 0.7f, 0.7f);
    DrawWallWithRings(rightWallX, rightWallY, rightWallZ, wallThickness, wallHeight, groundSize, -90.0f); //right
    glScalef(0.5f, 0.5f, 0.5f);

    DrawPlayer(0.0f, 1.0f, 0.0f);
    glFlush();
}

void Anim() {
    rotAng += 0.01;

    glutPostRedisplay();
}

void main(int argc, char** argv) {
	InitializeSound();
    InitializeGLUT(argc, argv);
    InitializeCallbacks();
    InitializeOpenGL();
    glutMainLoop();
    engine->drop();
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

void playSound(const char* soundFile) {
    if (engine) {
        engine->play2D(soundFile, false, false, true);
    }
}

void InitializeSound() {
    engine = createIrrKlangDevice();
    if (!engine) {
        std::cerr << "Could not initialize irrKlang!" << std::endl;
    }
}

/*std::thread soundThread1(playSound, "bg_sound.wav");
    std::thread soundThread2(playSound, "lose.wav");

    // Wait for the threads to finish
    soundThread1.join();
    soundThread2.join();*/