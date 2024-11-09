#include <glut.h>

float rotAng;
//screen size
int xCord = 1000, yCord = 700;

//ground and wall dimensions
const float groundSize = 5.0f;
const float wallHeight = 3.0f;
const float wallThickness = 0.1f;

void DrawWall(float x, float y, float z, float width, float height, float thickness) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(width, height, thickness);
    glutSolidCube(1);
    glPopMatrix();
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

    
    glColor3f(0.6f, 0.6f, 0.6f);
	DrawGround(0.0f, -wallHeight / 2, 0.0f, groundSize);

    
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

    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)xCord / (float)yCord, 0.1f, 300.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0f, 2.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    glutMainLoop();
}