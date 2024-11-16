#include <iostream>
#include <stdio.h>
#include <glut.h>
#include <math.h>
#include <irrKlang.h>
#include <thread>
using namespace irrklang;
#include <cstdlib>
#include <ctime>
#include <string>
#include <sstream>

float rotAng;
//screen size
int xCord=1000, yCord=700;

//ground and wall dimensions
const float groundSize= 5.0f;
const float wallHeight= 3.0f;
const float wallThickness=0.1f;
const float ringThickness =3.0f;

//wall positions
float backWallX=0.0f, backWallY = wallHeight/2, backWallZ = -groundSize/2;
float leftWallX=-groundSize/2, leftWallY = wallHeight/2,leftWallZ =0.0f;
float rightWallX= groundSize/2, rightWallY =wallHeight/2, rightWallZ =0.0f;

//wall colors
float backWallColor[3];
float leftWallColor[3];
float rightWallColor[3];
float myColors[][3]={
    {0.8f, 0.8f, 0.8f},
    {0.9f, 0.9f, 0.8f},
    {0.8f, 0.7f, 0.7f},
};
const int numColors= sizeof(myColors)/sizeof(myColors[0]);

//camera variables for free movement and rotation
float camX = 0.0f, camY = 2.0f, camZ = 5.5f;
float camYaw = 0.0f, camPitch = -0.2f;
enum View { FREE_VIEW, TOP_VIEW, SIDE_VIEW, FRONT_VIEW };
View currentView = FREE_VIEW;

//player position and rotation
float playerX = 0.0f,playerY =1.0f, playerZ =0.0f;
float playerRotation = 0.0f;
float currentRotation = 0.0f;
bool moveLeft=false;
bool moveRight=false;
bool moveForward=false;
bool moveBackward=false;

//functions signatures
void InitializeGLUT(int argc, char** argv);
void InitializeCallbacks();
void InitializeOpenGL();
void playSound(const char* soundFile, bool loop=false);

//sound data
irrklang::ISoundEngine* engine = irrklang::createIrrKlangDevice();
irrklang::ISound* backgroundSound = nullptr;
bool hasPlayedWinSound=false;
bool hasPlayedLoseSound=false;

//game data
bool isTimeUp=false;
int score=0;
float timeRemaining=30.0f;
static float elapsedTime = 0.0f;
bool winGame = false;

//windsock data
float windDirection =45.0f;
float windSpeed =0.5f;
bool isMoving =false;

//target data
float targetPosX =0.0f, targetPosY=1.5, targetPosZ=-3;
float targetSpeed = 0.01f;
bool isArrowActive = false;
float arrowPosX, arrowPosY, arrowPosZ;
float arrowSpeed = 0.1f;
bool moveQuiver = false;
bool moveTarget = false;

//podium data
bool changeColor = false;

//scoreboard data
bool screenColorChange =false;

void DrawCylinder(float base, float top, float height, int slices, int stacks) {
    GLUquadric* quad = gluNewQuadric();
    gluCylinder(quad, base, top, height, slices, stacks);
    gluDeleteQuadric(quad);
}

void DrawFence(float x, float y, float z, float poleHeight, float poleRadius, float barWidth, float barHeight, int numPoles, float spacing) {
    glColor3f(0.6f, 0.4f, 0.2f); //brown

    for (int i = 0; i < numPoles; i++) {
        glPushMatrix();
        float poleX = x + i * spacing;
        glTranslatef(poleX, y + poleHeight / 2, z);
        glRotatef(-90, 1.0f, 0.0f, 0.0f);
        DrawCylinder(poleRadius, poleRadius, poleHeight, 16, 8);
        glPopMatrix();
    }

    for (int i = 0; i < 2; i++) {
        glPushMatrix();
        glTranslatef(x + (numPoles - 1) * spacing / 2.0f, y + poleHeight * (0.3f + i * 0.4f), z);
        glScalef(numPoles * spacing, barHeight, poleRadius * 2.0f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
}

void InitializeColors() {
    srand(static_cast<unsigned int>(time(0)));

    int backWallIndex =rand()%numColors;
    int leftWallIndex;
    do {
        leftWallIndex=rand()%numColors;
    } while (leftWallIndex== backWallIndex);

    int rightWallIndex;
    do {
        rightWallIndex=rand() % numColors;
    } while (rightWallIndex== backWallIndex||rightWallIndex== leftWallIndex);

    for (int i = 0; i < 3;++i) {
        backWallColor[i]=myColors[backWallIndex][i];
        leftWallColor[i]=myColors[leftWallIndex][i];
        rightWallColor[i]=myColors[rightWallIndex][i];
    }
}

void UpdateColors(int value) {
    int backWallIndex = rand() % numColors;
    int leftWallIndex;
    do {
        leftWallIndex = rand() % numColors;
    } while (leftWallIndex == backWallIndex);

    int rightWallIndex;
    do {
        rightWallIndex = rand() % numColors;
    } while (rightWallIndex == backWallIndex || rightWallIndex == leftWallIndex);

    for (int i = 0; i < 3; ++i) {
        backWallColor[i] = myColors[backWallIndex][i];
        leftWallColor[i] = myColors[leftWallIndex][i];
        rightWallColor[i] = myColors[rightWallIndex][i];
    }

    glutTimerFunc(5000, UpdateColors, 0);
}

void DrawNumber1(float x, float y) {
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + 0.1f, y);
    glVertex2f(x + 0.1f, y + 0.5f);
    glVertex2f(x, y + 0.5f);
    glEnd();
}

void DrawNumber2(float x, float y) {
    //top
    glBegin(GL_QUADS);
    glVertex2f(x, y + 0.4f);
    glVertex2f(x + 0.3f, y + 0.4f);
    glVertex2f(x + 0.3f, y + 0.5f);
    glVertex2f(x, y + 0.5f);
    glEnd();

    //middle
    glBegin(GL_QUADS);
    glVertex2f(x, y + 0.2f);
    glVertex2f(x + 0.3f, y + 0.2f);
    glVertex2f(x + 0.3f, y + 0.3f);
    glVertex2f(x, y + 0.3f);
    glEnd();

    //bottom
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + 0.3f, y);
    glVertex2f(x + 0.3f, y + 0.1f);
    glVertex2f(x, y + 0.1f);
    glEnd();

    //right top
    glBegin(GL_QUADS);
    glVertex2f(x + 0.2f, y + 0.3f);
    glVertex2f(x + 0.3f, y + 0.3f);
    glVertex2f(x + 0.3f, y + 0.4f);
    glVertex2f(x + 0.2f, y + 0.4f);
    glEnd();

    //right bot
    glBegin(GL_QUADS);
    glVertex2f(x, y + 0.1f);
    glVertex2f(x + 0.1f, y + 0.1f);
    glVertex2f(x + 0.1f, y +0.2f);
    glVertex2f(x, y + 0.2f); 
    glEnd();
}

void DrawNumber3(float x, float y) {
    //top
    glBegin(GL_QUADS);
    glVertex2f(x, y + 0.4f);
    glVertex2f(x + 0.3f, y + 0.4f);
    glVertex2f(x + 0.3f, y + 0.5f);
    glVertex2f(x, y + 0.5f);
    glEnd();

    //middle
    glBegin(GL_QUADS);
    glVertex2f(x, y + 0.2f);
    glVertex2f(x + 0.3f, y + 0.2f);
    glVertex2f(x + 0.3f, y + 0.3f);
    glVertex2f(x, y + 0.3f);
    glEnd();

    //bottom
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + 0.3f, y);
    glVertex2f(x + 0.3f, y + 0.1f);
    glVertex2f(x, y + 0.1f);
    glEnd();

    //right top
    glBegin(GL_QUADS);
    glVertex2f(x + 0.2f, y + 0.3f);
    glVertex2f(x + 0.3f, y + 0.3f);
    glVertex2f(x + 0.3f, y + 0.4f);
    glVertex2f(x + 0.2f, y + 0.4f);
    glEnd();

    //right bottom
    glBegin(GL_QUADS);
    glVertex2f(x + 0.2f, y + 0.1f);
    glVertex2f(x + 0.3f, y + 0.1f);
    glVertex2f(x + 0.3f, y + 0.2f);
    glVertex2f(x + 0.2f, y + 0.2f);
    glEnd();
}

void DrawPodium(float posX, float posY, float posZ, float rotationAngle) {

    glPushMatrix();
    glTranslatef(posX, posY, posZ);
    glRotatef(rotationAngle, 0.0f, 1.0f, 0.0f);

    glColor3f(0.6f, 0.6f, 0.6f);//gray

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);
    glScalef(1.0f, 0.5f, 1.0f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-1.1f, -0.25f, 0.0f);
    glScalef(1.0f, 0.25f, 1.0f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(1.1f, -0.35f, 0.0f);
    glScalef(1.0f, 0.15f, 1.0f);
    glutSolidCube(1.0);
    glPopMatrix();
    std::srand(static_cast<unsigned>(std::time(nullptr)));
	if (changeColor) {
        float red = static_cast<float>(std::rand()) / RAND_MAX;
        float green = static_cast<float>(std::rand()) / RAND_MAX;
        float blue = static_cast<float>(std::rand()) / RAND_MAX;
        glColor3f(red, green, blue);
    }
	else {
        glColor3f(1.0f, 1.0f, 0.0f);//yellow
	}
    DrawNumber1(-0.05f, 0.1f);
    DrawNumber2(-1.2f, -0.1f);
    DrawNumber3(0.9f, -0.2f);

    glPopMatrix();
}

void DrawScoreboard(float x, float y, float z, float thickness, float width, float height) {

    glColor3f(0.0f, 0.0f, 0.0f);

    //outer frame
    glPushMatrix();
    glTranslatef(x, y, z-thickness/2.0f);
	glScaled(width, height, thickness);
    glutSolidCube(width);
    glPopMatrix();

    //inner screen
    if(screenColorChange)
	{
		float red = static_cast<float>(std::rand()) / RAND_MAX;
		float green = static_cast<float>(std::rand()) / RAND_MAX;
		float blue = static_cast<float>(std::rand()) / RAND_MAX;
		glColor3f(red, green, blue);
	}
	else {
        glColor3f(53.0f / 255.0f, 56.0f / 255.0f, 54.0f / 255.0f);//dark grey
    }
    glPushMatrix();
    glTranslatef(x, y, z - thickness / 2.0f+0.1f);
    glScaled(width, height, thickness);
    glutSolidCube(width - 0.2f);
    glPopMatrix();

    glColor3f(0.0f, 0.0f, 0.0f);
    //left stand
    glPushMatrix();
    glTranslatef(x-width/2.0f+0.2f, y-height, z-thickness / 2);
    glScalef(0.2f, height, 0.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    //right stand
    glPushMatrix();
    glTranslatef(x + width/2.0f -0.2f, y-height,z-thickness/2);
    glScalef(0.2f, height, 0.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glColor3f(1.0f, 0.0f, 0.0f); // Red color for the torus
    glPushMatrix();
    glTranslatef(x, y + height / 2.0f + 0.2f, z - thickness / 2.0f);
    glutSolidTorus(0.05, 0.2, 20, 20);
    glPopMatrix();
}

void DrawPole(float x, float y, float z) {
    float poleHeight = 2.0f;
    float poleRadius = 0.05f;

    GLUquadric* quad = gluNewQuadric();
    glPushMatrix();
    glColor3f(0.5f, 0.5f, 0.5f);
    glTranslatef(x,y, z);
    glRotated(90, 1.0f, 0.0f, 0.0f);
    gluCylinder(quad, poleRadius, poleRadius, poleHeight, 16, 16);
    glPopMatrix();
    gluDeleteQuadric(quad);
}

void DrawWindsock(float x, float y, float z) {
    int numStripes = 5;
    float sockLength = 2.5f;
    float sockRadius = 0.2f;
    DrawPole(x, y, z);

    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(windDirection, 0.0f, 1.0f, 0.0f);

    for (int i = 0; i < numStripes; ++i) {
        if (i % 2 == 0)
            glColor3f(1.0f, 0.65f, 0.0f);//orange
        else
            glColor3f(1.0f, 1.0f, 1.0f);

        GLUquadric* quad = gluNewQuadric();
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, -i * (sockLength / numStripes));
        gluCylinder(quad, sockRadius * (1 - 0.1f * i), sockRadius * (1 - 0.1f * i),
            sockLength / numStripes, 16, 16);
        glPopMatrix();
        gluDeleteQuadric(quad);
    }
    glPopMatrix();
}

void UpdateWind(int value) {
	if (!isMoving) return;
    windDirection += windSpeed * 10.0f;
    if (windDirection >= 360.0f) windDirection -= 360.0f;
    glutPostRedisplay();
    glutTimerFunc(50, UpdateWind, 0);
}

void DrawDisk(float radius, float innerRadius, float red, float green, float blue) {
    GLUquadric* quadric = gluNewQuadric();

    glColor3f(red, green, blue);
    gluDisk(quadric, innerRadius, radius, 100, 1);

    gluDeleteQuadric(quadric);
}

void DrawSolidCylinder(float radius, float height, float red, float green, float blue) {
    GLUquadric* quadric = gluNewQuadric();

    glColor3f(red, green, blue);
    gluCylinder(quadric, radius, radius, height, 100, 1);

    gluDeleteQuadric(quadric);
}

void DrawTarget(float x, float y, float z) {

    glPushMatrix();
    glTranslatef(x, y, z);

    DrawDisk(0.2f, 0.0f, 1.0f, 1.0f, 0.0f); //yellow
    DrawDisk(0.3f, 0.2f, 1.0f, 0.0f, 0.0f); //red
    DrawDisk(0.4f, 0.3f, 0.0f, 0.0f, 1.0f); //blue
    DrawDisk(0.5f, 0.4f, 0.0f, 0.0f, 0.0f); //black
    DrawDisk(0.6f, 0.5f, 1.0f, 1.0f, 1.0f); //white
    glTranslatef(0, 0, -0.05);
    DrawSolidCylinder(0.2f, 0.05f, 1.0f, 1.0f, 0.0f); //yellow
    DrawSolidCylinder(0.3f, 0.05f, 1.0f, 0.0f, 0.0f); //red
    DrawSolidCylinder(0.4f, 0.05f, 0.0f, 0.0f, 1.0f); //blue
    DrawSolidCylinder(0.5f, 0.05f, 0.0f, 0.0f, 0.0f); //black
    DrawSolidCylinder(0.6f, 0.05f, 1.0f, 1.0f, 1.0f); //white
    glPopMatrix();
    glPopMatrix();
}

void UpdateTarget(int value) {
    if (moveTarget) {
        do {
            targetPosX = rightWallX+leftWallX +static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (rightWallX - leftWallX));
        } while (targetPosX > rightWallX || targetPosX < leftWallX);
        moveTarget = false;
    }
    targetPosX += targetSpeed;

    if (targetPosX > rightWallX || targetPosX < leftWallX)
        targetSpeed = -targetSpeed;
    
    glutPostRedisplay();
    glutTimerFunc(16, UpdateTarget, 0);
}

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

    GLUquadricObj* quadratic = gluNewQuadric();
    glPushMatrix();
    glTranslatef(startX, startY, startZ);
    glRotatef(90, 0.0f, 1.0f, 0.0f);
	glColor3f(0.2588f, 0.2431f, 0.2510f);//dark grey
    gluCylinder(quadratic, 0.02f, 0.02f, length, 32, 32);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(startX + length, startY, startZ);
    glRotatef(90, 0.0f, 1.0f, 0.0f);
	glColor3f(0.6784f, 0.6392f, 0.6588f);//light grey
    gluCylinder(quadratic, 0.05f, 0.0f, 0.1f, 32, 32);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(startX-0.07, startY, startZ);
    glRotatef(90, 0.0f, 1.0f, 0.0f);
	glColor3f(0.6314f, 0.0745f, 0.1412f);//red
    gluCylinder(quadratic, 0.05f, 0.0f, 0.1f, 32, 32); 
    glPopMatrix();
}

void DrawTargetArrow() {
    if (!isArrowActive) return;
    glLineWidth(2);
    GLUquadricObj* quadratic = gluNewQuadric();
    glPushMatrix();
    glTranslatef(arrowPosX, arrowPosY, arrowPosZ);
    glRotatef(180, 0.0f, 1.0f, 0.0f);
    glColor3f(0.2588f, 0.2431f, 0.2510f); // dark grey
    gluCylinder(quadratic, 0.02f, 0.02f, 0.5f, 32, 32);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(arrowPosX, arrowPosY, arrowPosZ - 0.5f);
    glRotatef(180, 0.0f, 1.0f, 0.0f);
    glColor3f(0.6784f, 0.6392f, 0.6588f); // light grey
    gluCylinder(quadratic, 0.05f, 0.0f, 0.1f, 32, 32);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(arrowPosX, arrowPosY, arrowPosZ);
    glRotatef(180, 0.0f, 1.0f, 0.0f);
    glColor3f(0.6314f, 0.0745f, 0.1412f); // red
    gluCylinder(quadratic, 0.05f, 0.0f, 0.1f, 32, 32);
    glPopMatrix();
}

void DrawQuiver(float x, float y, float z) {
    GLUquadricObj* quad = gluNewQuadric();

    glPushMatrix();
    glTranslatef(x, y+0.5, z);
    glColor3f(0.4f, 0.2f, 0.1f);
    glRotatef(90, 1.0f, 0.0f, 0.0f);
    gluCylinder(quad, 0.35f, 0.35f, 1.0f, 32, 32);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(x, y-0.5, z);
    glRotatef(90, 1.0f, 0.0f, 0.0f);
    gluDisk(quad, 0.0f, 0.35f, 32, 1);
    glPopMatrix();

    
    int arrowCount = 5;
    std::srand(std::time(nullptr));
    for (int i = 0; i < arrowCount; i++) {
		float offsetX, offsetZ;

        if (moveQuiver) {
            offsetX = x + (i % 2 == 0? (std::rand() % 30 - 10) / 80.0f: -(std::rand() % 30 - 10) / 80.0f);
            offsetZ = z + (i % 2 == 0 ? (std::rand() % 30 - 10) / 80.0f : -(std::rand() % 30 - 10) / 80.0f);
        }
        else {
            offsetX = x + (i % 2 == 0 ? 0.2f / (i + 1) : -0.3f / (i + 1));
            offsetZ = z + (i % 2 == 0 ? -0.2f / (i + 1) : 0.3f / (i + 1));
        }
        glPushMatrix();
        glTranslatef(offsetX, y, offsetZ);
        glRotatef(-90, 1.0f, 0.0f, 0.0f);
        glScaled(1.0f, 2.0f, 1.0f);
        GLUquadricObj* quadratic = gluNewQuadric();
        glPushMatrix();
        glTranslatef(0, 0, 0.2f);
        glColor3f(0.2588f, 0.2431f, 0.2510f); // dark grey
        gluCylinder(quadratic, 0.02f, 0.02f, 0.5f, 32, 32);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0, 0,0.7f);
        glColor3f(1.0f, 0.0f, 0.0f);
        gluCylinder(quadratic, 0.05f, 0.0f, 0.1f, 32, 32);
        glPopMatrix();
        glPopMatrix();
    }
    gluDeleteQuadric(quad);
}

void DrawBow(float x, float y, float z) {
    glPushMatrix();
    glLineWidth(2);
    glTranslatef(x, y - 0.2f, z);
    glRotatef(270, 0.0f, 1.0f, 0.0f);
    glRotatef(-90, 0.0f, 0.0f, 1.0f);
    glColor3f(0.55f, 0.27f, 0.07f);

    glBegin(GL_QUAD_STRIP);
    float bowRadius = 0.45f;
    for (int i = -9; i <= 9; i++) {
        float angle = (float)i / 18.0f * 3.14159f;
        float xPos = bowRadius * cos(angle);
        float yPos = bowRadius * sin(angle);
        glVertex3f(xPos, yPos, 0.05f);
        glVertex3f(xPos, yPos, -0.05f);
    }
    glEnd();

    glBegin(GL_QUADS);
    glColor3f(0.8588f, 0.6706f, 0.6941f);
    glVertex3f(0.0, bowRadius, 0.05f);
    glVertex3f(0.0, -bowRadius, 0.05f);
    glVertex3f(0.0, -bowRadius, -0.05f);
    glVertex3f(0.0, bowRadius, -0.05f);
    glEnd();

    DrawArrow(0.05f, 0.0f, 0.0f, 0.32f);
    glPopMatrix();
}

void MovePlayer(float deltaTime) {
    const float moveSpeed = 2.0f;

    if (moveLeft&&playerX>=-groundSize+6*wallThickness) {
        playerX -= moveSpeed * deltaTime;
        playerRotation = -90.0f; //left
        currentRotation = playerRotation;
    }
    if (moveRight&&playerX<=groundSize-6*wallThickness) {
        playerX += moveSpeed * deltaTime;
        playerRotation = 90.0f; //right
        currentRotation = playerRotation;
    }
    if (moveForward&&playerZ>=-groundSize+6*wallThickness) {
        playerZ -= moveSpeed * deltaTime;
        playerRotation = 180.0f; //(positive Z)
        currentRotation = playerRotation;
    }
    if (moveBackward&&playerZ<=groundSize-6*wallThickness) {
        playerZ += moveSpeed * deltaTime;
        playerRotation = 0.0; //backward
        currentRotation = playerRotation;
    }
    if(isArrowActive) playerRotation = 180.0;
    else playerRotation=currentRotation; 
}

void DrawPlayer(float x, float y, float z) {
    
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(playerRotation, 0.0f, 1.0f, 0.0f); //rotate player around Y-axis

    //body
    glPushMatrix();
    glColor3f(0.5f, 0.6f, 1.0f);
    glScalef(0.5f, 0.8f, 0.2f);
    glutSolidCube(1);
    glPopMatrix();

    DrawBow(-0.35f, 0.0f, 0.0f);

    //head
    glPushMatrix();
    glTranslatef(0.0f, 0.68f, 0.0f);
    glColor3f(1.0f, 0.8f, 0.6f);
    glutSolidSphere(0.25f, 20, 20);
    glPopMatrix();

    //left eye
    glPushMatrix();
    glTranslatef(-0.1f, 0.75f, 0.2f);
    glColor3f(0.0f, 0.0f, 0.0f);
    glutSolidSphere(0.05f, 10, 10);
    glPopMatrix();

    //right eye
    glPushMatrix();
    glTranslatef(0.1f, 0.75f, 0.2f);
    glColor3f(0.0f, 0.0f, 0.0f);
    glutSolidSphere(0.05f, 10, 10);
    glPopMatrix();

    //mouth
    glPushMatrix();
    glTranslatef(0.0f, 0.68f, 0.25f);
    glColor3f(0.8f, 0.0f, 0.0f);
    glRotatef(180.0f, 1.0f, 0.0f, 0.0f);

    //half-circle mouth
    glBegin(GL_POLYGON);
    const int num_segments = 20;
    float radius = 0.08f;
    for (int i = 0; i <= num_segments; i++) {
        float angle = (float)i / (float)num_segments * 3.14159f;
        glVertex2f(radius * cos(angle), radius * sin(angle));
    }
    glEnd();
    glPopMatrix();

    //left leg
    glPushMatrix();
    glTranslatef(-0.15f, -0.7f, 0.0f);
    glColor3f(0.0f, 0.0f, 1.0f);
    glScalef(0.2f, 0.6f, 0.2f);
    glutSolidCube(1);
    glPopMatrix();

    //right leg
    glPushMatrix();
    glTranslatef(0.15f, -0.7f, 0.0f);
    glColor3f(0.0f, 0.0f, 1.0f);
    glScalef(0.2f, 0.6f, 0.2f);
    glutSolidCube(1);
    glPopMatrix();

    //left hand
    glPushMatrix();
    glTranslatef(-0.30f, 0.1f, 0.0f);
    glColor3f(1.0f, 0.8f, 0.6f);
    glScalef(0.1f, 0.6f, 0.2f);
    glutSolidCube(1);
    glPopMatrix();

    //right hand
    glPushMatrix();
    glTranslatef(0.30f, 0.1f, 0.0f);
    glColor3f(1.0f, 0.8f, 0.6f);
    glScalef(0.1f, 0.6f, 0.2f);
    glutSolidCube(1);
    glPopMatrix();

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
        glTranslatef(-thickness / 2 + 0.06f, y, z);
    }
    glRotatef(rotationAngle, 0.0f, 1.0f, 0.0f);
	glScaled(0.5f, 0.5f, 0.5f);
    DrawOlympicRings();
    glPopMatrix();
}

void Update(int value) {
    if (isArrowActive) {
        arrowPosZ -= arrowSpeed;

        if (arrowPosZ <= targetPosZ + 0.17f && arrowPosZ >= targetPosZ - 0.17f &&
            arrowPosX <= targetPosX + 0.17f && arrowPosX >= targetPosX - 0.17f &&
            arrowPosY <= targetPosY + 0.17f && arrowPosY >= targetPosY - 0.17f) {
            isArrowActive = false;
            score += 10;
			playSound("win1.wav",false);
            if(score>=120)
			    winGame = true;
            moveTarget = true;
        }

        if (arrowPosZ < -groundSize) {
            isArrowActive = false;
        }
    }
    elapsedTime += 0.016f;

    if (elapsedTime >= 1.0f) {
        elapsedTime = 0.0f;
        if (timeRemaining > 0) {
            timeRemaining -= 1.0f;
            if (timeRemaining <= 0) {
                timeRemaining = 0;
                isTimeUp = true;
            }
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, Update, 0);
}

void Keyboard(unsigned char key, int x, int y) {
    if (isTimeUp) return;
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
    case '1':
		isMoving = !isMoving;
        if (isMoving) {
            UpdateWind(0);
			playSound("flag.wav",false);
        }
        break;
    case ' ':
        if (!isArrowActive) {
            isArrowActive = true;
            arrowPosX = playerX;
            arrowPosY = playerY+0.5;
            arrowPosZ = playerZ;
            playSound("arrow-shoot.wav",false);
        }
        break;
	case '2':
		moveQuiver = !moveQuiver;
		if (moveQuiver)
            playSound("arrows-move.wav",false);
		break;
    case '3':
		changeColor = !changeColor;
		if (changeColor)
		    playSound("color-change.wav",false);
		break;
	case '4':
		screenColorChange = !screenColorChange;
		if (screenColorChange)
			playSound("color-change.wav", false);
		break;
    }
	printf("my score is %d\n", score);
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

void SpecialKeys(int key, int x, int y) {
    if (isTimeUp) return;
    switch (key) {
    case GLUT_KEY_LEFT: moveLeft = true; break;
    case GLUT_KEY_RIGHT: moveRight = true; break;
    case GLUT_KEY_UP: moveForward = true; break;
    case GLUT_KEY_DOWN: moveBackward = true; break;
    }
    glutPostRedisplay();
}

void SpecialKeysUp(int key, int x, int y) {
    if (isTimeUp) return;
    switch (key) {
    case GLUT_KEY_LEFT: moveLeft = false; break;
    case GLUT_KEY_RIGHT: moveRight = false; break;
    case GLUT_KEY_UP: moveForward = false; break;
    case GLUT_KEY_DOWN: moveBackward = false; break;
    }
}

void Render2DText(int score, float timeLeft,bool gameWin, bool gameLose) {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
    gluOrtho2D(0.0, 800.0, 0.0, 600.0);
    glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

    if (gameLose) {
        glColor3f(1.0f, 0.0f, 0.0f);
        glRasterPos2f(xCord / 2.0-100, yCord / 2.0);
        char message[50];
        sprintf(message, "Game Lose,\n\n with a score of %d", score);
        for (char* c = message; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
        }
    }
    else if (gameWin) {
        glColor3f(0.1137f, 0.6118f, 0.0980f);
        glRasterPos2f(xCord / 2.0-100, yCord / 2.0);
        char message[50];
        sprintf(message, "Game Win,\n\n with a score of %d", score);
        for (char* c = message; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
        }
    }
    else {
        glColor3f(0.0f, 0.0f, 0.0f);
        glRasterPos2f(50.0f, 530.0f);
        std::string scoreText = "Score: " + std::to_string(score);
        for (char c : scoreText) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
        }
        glColor3f(0.0f, 0.0f, 0.0f);
        glRasterPos2f(50.0f, 500.0f);
        std::string timeText = "Time: " + std::to_string(static_cast<int>(timeLeft));
        for (char c : timeText) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
        }
    }
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}

void Display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (isTimeUp && backgroundSound) {
        backgroundSound->stop();
        backgroundSound->drop();
        backgroundSound = nullptr;
    }
    if (isTimeUp && !winGame && !hasPlayedLoseSound) {
        playSound("lose.wav",true);
        hasPlayedLoseSound = true;
    }
    if (winGame && isTimeUp && !hasPlayedWinSound) {
        playSound("win.wav",true);
        hasPlayedWinSound = true;
    }
    //update the camera freely or to one of the views
    SetCamera();
    if (winGame && isTimeUp) Render2DText(score, 0, true, false);
    else if (isTimeUp && !winGame) Render2DText(score, 0, false, true);
    else {
        glColor3f(0.125f, 0.271f, 0.094f);
        DrawGround(0.0f, 0.0f, 0.0f, groundSize); //ground

        glColor3f(0.3608f, 0.0706f, 0.2510f);
        DrawWall(backWallX, backWallY + wallHeight / 1.82, backWallZ, groundSize, 0.3f, wallThickness + 0.1); //back ceiling

        glPushMatrix();
        glColor3f(0.1686f, 0.0314f, 0.1176f);
        DrawWall(leftWallX, leftWallY + wallHeight / 1.82, leftWallZ, wallThickness, 0.3f, groundSize); //left ceiling
        glRotatef(90, 0.0f, 1.0f, 0.0f);
        glPopMatrix();

        glColor3f(backWallColor[0], backWallColor[1], backWallColor[2]);
        DrawWallWithRings(backWallX, backWallY, backWallZ, groundSize, wallHeight, wallThickness); //back

        glColor3f(leftWallColor[0], leftWallColor[1], leftWallColor[2]);
        DrawWallWithRings(leftWallX, leftWallY, leftWallZ, wallThickness, wallHeight, groundSize, 90.0f); //left

        if (currentView != SIDE_VIEW) {
            glColor3f(rightWallColor[0], rightWallColor[1], rightWallColor[2]);
            DrawWallWithRings(rightWallX, rightWallY, rightWallZ, wallThickness, wallHeight, groundSize, -90.0f); //right
        
            glPushMatrix();
            glColor3f(0.1686f, 0.0314f, 0.1176f);
            DrawWall(rightWallX, rightWallY + wallHeight / 1.82, rightWallZ, wallThickness, 0.3f, groundSize); //right celing
            glRotatef(-90, 0.0f, 1.0f, 0.0f);
            glPopMatrix();

        }
        glScalef(0.5f, 0.5f, 0.5f);
        DrawPlayer(playerX, playerY, playerZ);
        DrawQuiver(3.0f, 0.5f, 3.0f);
        DrawWindsock(3, 2, 0.5);
        DrawTarget(targetPosX, targetPosY, targetPosZ);
        DrawScoreboard(leftWallX - 0.5, 1.5f, backWallZ - 1, 0.2f, 1.6f, 1.0f);
        DrawPodium(-2.6f, 0.5f, 2.0f, 45.0f);
        DrawTargetArrow();
        DrawFence(1.5f,0.0f, 5.0f, 1.0f, 0.05f, 0.03f, 0.05f, 4, 0.5f);
        Render2DText(score, timeRemaining,false,false);
    }
    glFlush();
}

void Anim() {
    static int lastTime = glutGet(GLUT_ELAPSED_TIME);
    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    float deltaTime = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;

    MovePlayer(deltaTime);

    rotAng += 0.01;
    glutPostRedisplay();
}

void main(int argc, char** argv) {
    InitializeGLUT(argc, argv);
    InitializeOpenGL();
    InitializeCallbacks();
    UpdateTarget(0);
    InitializeColors();
    UpdateColors(0);
    Update(0);
	if (!backgroundSound)
        backgroundSound= engine->play2D("bg_sound.wav", false, false, true);
    
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
    if (isTimeUp) return;
    glutIdleFunc(Anim);
    glutKeyboardFunc(Keyboard);
    glutSpecialFunc(SpecialKeys);
    glutSpecialUpFunc(SpecialKeysUp);
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

void playSound(const char* soundFile,bool loop) {
    if (engine) {
        engine->play2D(soundFile, loop, false, true);
    }
}