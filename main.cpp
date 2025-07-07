#include <GL/glut.h>
#include <cmath>

// Variabel kamera
float cameraAngle = 0.0f;
float cameraDistance = 25.0f;
float cameraHeight = 5.0f;

// ========== UTILITAS ==========
void drawBox(float x, float y, float z, float w, float h, float d) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(w, h, d);
    glutSolidCube(1.0f);
    glPopMatrix();
}

void drawCylinder(float x, float y, float z, float radius, float height) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(-90, 1, 0, 0);
    GLUquadric* quad = gluNewQuadric();
    gluCylinder(quad, radius, radius, height, 16, 16);
    gluDeleteQuadric(quad);
    glPopMatrix();
}

// ========== KOMPONEN UTAMA ==========

void drawGround() {
    glColor3f(0.5f, 0.5f, 0.5f); // Jalan
    drawBox(0, -0.5f, 0, 60, 1, 40);

    glColor3f(0.8f, 0.8f, 0.8f); // Trotoar
    drawBox(0, -0.4f, -10, 50, 0.2f, 8);
    drawBox(0, -0.4f, 10, 50, 0.2f, 8);
}

void drawMainPillar(float x) {
    glColor3f(0.75f, 0.60f, 0.45f); // Base
    drawBox(x, 1.0f, -4.0f, 2.5f, 2.0f, 6.0f);

    glColor3f(0.78f, 0.63f, 0.48f); // Pilar utama
    drawBox(x, 6.0f, -4.0f, 2.2f, 10.0f, 5.0f);

    glColor3f(0.80f, 0.65f, 0.50f); // Top
    drawBox(x, 11.5f, -4.0f, 2.6f, 1.0f, 5.2f);
}

void drawCenterPillar() {
    glColor3f(0.78f, 0.63f, 0.48f);
    drawBox(0, 8.0f, -3.0f, 5.0f, 16.0f, 3.5f); // lebih lebar dan dalam

    glColor3f(0.85f, 0.70f, 0.55f); // Area logo
    drawBox(0, 10.0f, -2.3f, 2.0f, 2.0f, 0.3f);

    glColor3f(0.80f, 0.65f, 0.50f); // Top
    drawBox(0, 16.5f, -3.0f, 2.8f, 1.0f, 3.5f);
}

void drawSideWall(float x) {
    glColor3f(0.75f, 0.60f, 0.45f);
    drawBox(x - 1.2f, 3.0f, -4.0f, 1.6f, 16.0f, 4.0f); // dimundurkan dan dipendekkan
drawBox(x + 1.2f, 3.0f, -4.0f, 1.6f, 16.0f, 4.0f); // dimundurkan dan dipendekkan
}

void drawSideWall2(float x) {
     glColor3f(0.75f, 0.60f, 0.45f);
    drawBox(x - 1.2f, 3.0f, -4.0f, 2.0f, 16.0f, 4.0f); // dimundurkan dan dipendekkan
drawBox(x + 1.2f, 3.0f, -4.0f, 2.0f, 16.0f, 4.0f); // dimundurkan dan dipendekkan
}

void drawSmallPillar(float x, float z) {
    glColor3f(0.95f, 0.95f, 0.95f);
    drawCylinder(x, 0.0f, z, 0.2f, 11.5f); // lebih tinggi

    glColor3f(0.90f, 0.90f, 0.90f);
    drawBox(x, 11.7f, z, 0.5f, 0.5f, 0.5f);
}

void drawConnectingBeam() {
    glColor3f(0.85f, 0.85f, 0.85f);
    drawBox(0, 12.0f, -2.5f, 22.0f, 0.6f, 1.0f);
    drawBox(-9.5f, 12.0f, -2.5f, 1.0f, 0.6f, 1.0f);
    drawBox(9.5f, 12.0f, -2.5f, 1.0f, 0.6f, 1.0f);
}

void drawDecorations() {
    // Ornamen kecil di pilar tengah
    glColor3f(0.9f, 0.8f, 0.6f);
    drawBox(0, 14.0f, -2.3f, 0.8f, 1.2f, 0.3f);

    // Papan informasi di belakang pilar tengah
    glColor3f(0.4f, 0.4f, 0.4f);
    drawBox(0, 2.0f, -10.0f, 4.0f, 2.0f, 0.5f); // <== mundur ke z = -10

    // Dekorasi vertikal kiri-kanan papan
    glColor3f(0.85f, 0.85f, 0.85f); // ganti warna biar beda dari tiang
    drawBox(-2.0f, 1.2f, -10.0f, 0.1f, 2.0f, 0.1f); // lebih seperti tiang hias
    drawBox(2.0f, 1.2f, -10.0f, 0.1f, 2.0f, 0.1f);

    // Bola dekoratif di atas
    glPushMatrix();
    glTranslatef(-2.0f, 2.5f, -10.0f);
    glutSolidSphere(0.2, 12, 12);
    glTranslatef(4.0f, 0.0f, 0.0f);
    glutSolidSphere(0.2, 12, 12);
    glPopMatrix();
}

void drawBackground() {
    glColor3f(0.3f, 0.5f, 0.2f);
    for (int i = -2; i <= 2; i++)
        drawBox(i * 8.0f, 6.0f, -25.0f, 2.0f, 8.0f, 2.0f);

    glColor3f(0.7f, 0.8f, 0.9f);
    drawBox(0.0f, 15.0f, -40.0f, 100.0f, 40.0f, 1.0f);
}

// ========== DISPLAY ==========
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    float camX = cameraDistance * sin(cameraAngle * M_PI / 180.0f);
    float camZ = cameraDistance * cos(cameraAngle * M_PI / 180.0f);
    gluLookAt(camX, cameraHeight, camZ, 0, 5, -3, 0, 1, 0);

    drawBackground();
    drawGround();

    drawMainPillar(-9.0f);
    drawMainPillar(9.0f);
    drawCenterPillar();
    drawConnectingBeam();

    drawSideWall(-11.0f);
    drawSideWall(11.0f);
     drawSideWall2(11.0f);
     drawSideWall2(11.0f);

    // Pilar putih kiri-kanan (3 tiap sisi)
    drawSmallPillar(-11.5f, -1.5f);
drawSmallPillar(-10.0f, -1.0f);
drawSmallPillar(-8.5f, -1.0f);
drawSmallPillar(8.5f, -1.0);
drawSmallPillar(10.0f, -1.0f);
drawSmallPillar(11.5f, -1.0f);

    drawDecorations();
    glutSwapBuffers();
}

// ========== KONTROL ==========
void keyboard(unsigned char key, int, int) {
    switch (key) {
        case 'a': cameraAngle -= 5; break;
        case 'd': cameraAngle += 5; break;
        case 'w': cameraDistance -= 2; break;
        case 's': cameraDistance += 2; break;
        case 'q': cameraHeight += 1; break;
        case 'e': cameraHeight -= 1; break;
        case 'r': cameraAngle = 0; cameraDistance = 25; cameraHeight = 5; break;
        case 27: exit(0); // ESC
    }
    glutPostRedisplay();
}

// ========== SETUP ==========
void setup() {
    glClearColor(0.6f, 0.8f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    GLfloat lightPos[] = { 5, 15, 10, 1 };
    GLfloat ambient[] = { 0.4, 0.4, 0.4, 1 };
    GLfloat diffuse[] = { 0.8, 0.8, 0.8, 1 };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

    glMatrixMode(GL_PROJECTION);
    gluPerspective(45.0, 1.333, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

// ========== MAIN ==========
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 750);
    glutCreateWindow("Gerbang UKSW 3D - Final Revisi");

    setup();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}
