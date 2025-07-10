#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <fstream>
#include <windows.h>
#include <iostream>
#ifndef M_PI
#define M_PI 3.14159265359
#endif

#include "texture_loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


// Variabel kamera FPS
float camPosX = 0.0f, camPosY = 5.0f, camPosZ = 25.0f;
float camYaw = 0.0f, camPitch = 0.0f;
float camSpeed = 1.0f;
bool keyStates[256] = {0};
bool specialKeyStates[256] = {0};
bool isDragging = false;
int lastMouseX = 0;
int lastMouseY = 0;
// Update posisi kamera FPS
void updateCameraFPS() {
    float radYaw = camYaw * M_PI / 180.0f;
    float radPitch = camPitch * M_PI / 180.0f;
    float moveSpeed = camSpeed;
    // Forward vector (ikut yaw & pitch)
    float forwardX = cos(radPitch) * sin(radYaw);
    float forwardY = sin(radPitch);
    float forwardZ = -cos(radPitch) * cos(radYaw);
    // Right vector (hanya yaw)
    float rightX = cos(radYaw);
    float rightZ = sin(radYaw);

    // WASD: maju/mundur/kanan/kiri mengikuti arah kamera (termasuk pitch)
    if (keyStates['w'] || keyStates['W']) {
        camPosX += forwardX * moveSpeed;
        camPosY += forwardY * moveSpeed;
        camPosZ += forwardZ * moveSpeed;
    }
    if (keyStates['s'] || keyStates['S']) {
        camPosX -= forwardX * moveSpeed;
        camPosY -= forwardY * moveSpeed;
        camPosZ -= forwardZ * moveSpeed;
    }
    if (keyStates['a'] || keyStates['A']) {
        camPosX -= rightX * moveSpeed;
        camPosZ -= rightZ * moveSpeed;
    }
    if (keyStates['d'] || keyStates['D']) {
        camPosX += rightX * moveSpeed;
        camPosZ += rightZ * moveSpeed;
    }
    // Shift/Ctrl tetap naik/turun sumbu Y
    if (keyStates[16]) { // Shift
        camPosY += moveSpeed;
    }
    if (keyStates[17]) { // Ctrl
        camPosY -= moveSpeed;
    }
}

//Varibel warna
GLuint pillarTexture;
GLuint logoTexture;


//light variable
bool lampLightOn = true;
GLuint lampLightID = GL_LIGHT1;

// Day/Night cycle variables
bool isNightTime = false;
float dayNightTransition = 0.0f; // 0.0 = full day, 1.0 = full night
float transitionSpeed = 0.05f;
bool isTransitioning = false;

// Enhanced lighting variables
GLfloat dayAmbient[] = {0.3f, 0.3f, 0.3f, 1.0f};
GLfloat nightAmbient[] = {0.08f, 0.08f, 0.1f, 1.0f};
GLfloat dayDiffuse[] = {0.7f, 0.7f, 0.6f, 1.0f};
GLfloat nightDiffuse[] = {0.2f, 0.2f, 0.4f, 1.0f};
GLfloat daySkyColor[] = {0.6f, 0.8f, 1.0f};
GLfloat nightSkyColor[] = {0.05f, 0.05f, 0.2f};



//function buat gambar
void setupWorkingDirectory() {
    const char* path = "C:\\Users\\ASUS\\Documents\\C programs\\TR_ASDOS_GRAFKOM\\bin\\Debug";
    if (!SetCurrentDirectoryA(path)) {
        std::cerr << "Failed to changed Directory: " << path << std::endl;
    }
}

// Debug isi folder
void debugFolder() {
    char cwd[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, cwd);
    std::cout << "Current working directory: " << cwd << std::endl;
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile("texture\\.", &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        std::cout << "Folder 'texture' was not found." << std::endl;
    } else {
        std::cout << "Folder 'texture' filled:" << std::endl;
        do {
            if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                std::cout << " - " << findFileData.cFileName << std::endl;
            }
        } while (FindNextFile(hFind, &findFileData));
        FindClose(hFind);
    }
}

void setMaterial(float r, float g, float b, float shininess = 10.0f, float specStrength = 0.3f) {
    float nightDimming = 0.8f + 0.2f * (1.0f - dayNightTransition);

    GLfloat ambient[] = {r * 0.3f * nightDimming, g * 0.3f * nightDimming, b * 0.3f * nightDimming, 1.0f};
    GLfloat diffuse[] = {r * nightDimming, g * nightDimming, b * nightDimming, 1.0f};
    GLfloat specular[] = {
        specStrength * (0.9f + 0.1f * dayNightTransition),
        specStrength * (0.9f + 0.1f * dayNightTransition),
        specStrength * (0.95f + 0.05f * dayNightTransition),
        1.0f
    };

    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}



//supporter function
void drawTaperedCylinder(float bottomRadius, float topRadius, float height, int segments = 16) {
    float angleStep = 2.0f * M_PI / segments;

    glBegin(GL_TRIANGLE_STRIP);

    for (int i = 0; i <= segments; i++) {
        float angle = i * angleStep;

        // Bottom ring
        float x1 = bottomRadius * cos(angle);
        float z1 = bottomRadius * sin(angle);

        // Top ring
        float x2 = topRadius * cos(angle);
        float z2 = topRadius * sin(angle);

        // Calculate normals for smooth lighting
        float nx = cos(angle);
        float nz = sin(angle);

        glNormal3f(nx, 0, nz);
        glVertex3f(x1, 0, z1);

        glNormal3f(nx, 0, nz);
        glVertex3f(x2, height, z2);
    }

    glEnd();

    // Draw bottom cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, -1, 0);
    glVertex3f(0, 0, 0);
    for (int i = 0; i <= segments; i++) {
        float angle = i * angleStep;
        float x = bottomRadius * cos(angle);
        float z = bottomRadius * sin(angle);
        glVertex3f(x, 0, z);
    }
    glEnd();

    // Draw top cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 1, 0);
    glVertex3f(0, height, 0);
    for (int i = 0; i <= segments; i++) {
        float angle = i * angleStep;
        float x = topRadius * cos(angle);
        float z = topRadius * sin(angle);
        glVertex3f(x, height, z);
    }
    glEnd();
}

//draw sphere with custom color
void drawColoredSphere(float radius, int slices = 16, int stacks = 16) {
    glutSolidSphere(radius, slices, stacks);
}
void drawTwistedCylinder(float baseRadius, float topRadius, float height, int segments = 16) {
    float angleStep = 2.0f * M_PI / segments;
    float heightStep = height / 10; // Divide height into steps for twisting

    glBegin(GL_TRIANGLE_STRIP);

    for (int h = 0; h <= 10; h++) {
        float currentHeight = h * heightStep;
        float nextHeight = (h + 1) * heightStep;

        // Twist factor - more twist as we go up
        float twist = (currentHeight / height) * 0.5f; // Half rotation over height
        float nextTwist = (nextHeight / height) * 0.5f;

        // Radius interpolation with slight variation for bark texture
        float currentRadius = baseRadius + (topRadius - baseRadius) * (currentHeight / height);
        float nextRadius = baseRadius + (topRadius - baseRadius) * (nextHeight / height);

        for (int i = 0; i <= segments; i++) {
            float angle = i * angleStep;

            // Add bark texture variation
            float barkVariation = 0.9f + 0.1f * sin(angle * 3 + currentHeight * 0.1f);
            float nextBarkVariation = 0.9f + 0.1f * sin(angle * 3 + nextHeight * 0.1f);

            // Current ring
            float x1 = (currentRadius * barkVariation) * cos(angle + twist);
            float z1 = (currentRadius * barkVariation) * sin(angle + twist);

            // Next ring
            float x2 = (nextRadius * nextBarkVariation) * cos(angle + nextTwist);
            float z2 = (nextRadius * nextBarkVariation) * sin(angle + nextTwist);

            // Calculate normals for lighting
            float nx1 = cos(angle + twist);
            float nz1 = sin(angle + twist);
            float nx2 = cos(angle + nextTwist);
            float nz2 = sin(angle + nextTwist);

            glNormal3f(nx1, 0, nz1);
            glVertex3f(x1, currentHeight, z1);

            if (h < 10) {
                glNormal3f(nx2, 0, nz2);
                glVertex3f(x2, nextHeight, z2);
            }
        }
    }
    glEnd();
}

// Helper function to draw a branch (tapered cylinder)
void drawBranch(float length, float baseRadius, float topRadius, float bendAngle = 0.0f) {
    glPushMatrix();

    // Apply bend by rotating slightly
    if (bendAngle != 0.0f) {
        glRotatef(bendAngle, 0, 0, 1);
    }

    // Draw the branch cylinder
    drawTwistedCylinder(baseRadius, topRadius, length, 12);

    glPopMatrix();
}

void updateLighting() {
    // Interpolate between day and night lighting
    GLfloat currentAmbient[4], currentDiffuse[4], currentSky[3];

    for (int i = 0; i < 3; i++) {
        currentAmbient[i] = dayAmbient[i] * (1.0f - dayNightTransition) + nightAmbient[i] * dayNightTransition;
        currentDiffuse[i] = dayDiffuse[i] * (1.0f - dayNightTransition) + nightDiffuse[i] * dayNightTransition;
        currentSky[i] = daySkyColor[i] * (1.0f - dayNightTransition) + nightSkyColor[i] * dayNightTransition;
    }
    currentAmbient[3] = 1.0f;
    currentDiffuse[3] = 1.0f;

    // Update sky color
    glClearColor(currentSky[0], currentSky[1], currentSky[2], 1.0f);

    // Update main light (sun/moon)
    GLfloat lightPos[] = {
        5.0f + 10.0f * dayNightTransition,
        15.0f - 5.0f * dayNightTransition,
        10.0f,
        1.0f
    };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, currentAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, currentDiffuse);

    // Add slight blue tint for night specular
    GLfloat specular[] = {
        0.5f + 0.2f * dayNightTransition,
        0.5f + 0.2f * dayNightTransition,
        0.7f + 0.3f * dayNightTransition,
        1.0f
    };
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
}


void setRealisticMaterial(float r, float g, float b, float shininess = 1.0f, float specStrength = 0.3f) {
    // Adjust material properties based on day/night
    float nightDimming = 0.7f + 0.3f * (1.0f - dayNightTransition);

    GLfloat ambient[] = {
        r * 0.2f * nightDimming,
        g * 0.2f * nightDimming,
        b * 0.2f * nightDimming,
        1.0f
    };
    GLfloat diffuse[] = {
        r * nightDimming,
        g * nightDimming,
        b * nightDimming,
        1.0f
    };
    GLfloat specular[] = {
        specStrength * (0.8f + 0.2f * dayNightTransition),
        specStrength * (0.8f + 0.2f * dayNightTransition),
        specStrength * (0.9f + 0.1f * dayNightTransition),
        1.0f
    };

    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}




// ========== UTILITAS ==========
void drawBox(float x, float y, float z, float w, float h, float d) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(w, h, d);
    glutSolidCube(1.0f);
    glPopMatrix();
}

void drawFlutedCylinder(float baseRadius, float topRadius, float height, int numFlutes) {
    float angleStep = 2.0f * M_PI / numFlutes;
    float fluteDepth = 0.05f; // Depth of the flute relative to the main radius

    glBegin(GL_QUADS);
    for (int i = 0; i < numFlutes; ++i) {
        float angle1 = i * angleStep;
        float angle2 = (i + 1) * angleStep;
        float midAngle = (angle1 + angle2) / 2.0f;

        // Outer points (the "arris" or sharp edge)
        float x1_outer_bot = baseRadius * cos(angle1);
        float z1_outer_bot = baseRadius * sin(angle1);
        float x1_outer_top = topRadius * cos(angle1);
        float z1_outer_top = topRadius * sin(angle1);

        // Inner point (the bottom of the flute)
        float innerRadius_bot = baseRadius - fluteDepth;
        float innerRadius_top = topRadius - fluteDepth;
        float x_inner_bot = innerRadius_bot * cos(midAngle);
        float z_inner_bot = innerRadius_bot * sin(midAngle);
        float x_inner_top = innerRadius_top * cos(midAngle);
        float z_inner_top = innerRadius_top * sin(midAngle);

        // --- First half of the flute ---
        // Normal vector for the first face
        float nx1 = cos((angle1 + midAngle) / 2.0f);
        float nz1 = sin((angle1 + midAngle) / 2.0f);
        glNormal3f(nx1, 0, nz1);
        glVertex3f(x1_outer_bot, 0, z1_outer_bot);
        glVertex3f(x_inner_bot, 0, z_inner_bot);
        glVertex3f(x_inner_top, height, z_inner_top);
        glVertex3f(x1_outer_top, height, z1_outer_top);

        // --- Second half of the flute ---
        // Normal vector for the second face
        float nx2 = cos((midAngle + angle2) / 2.0f);
        float nz2 = sin((midAngle + angle2) / 2.0f);
        glNormal3f(nx2, 0, nz2);
        glVertex3f(x_inner_bot, 0, z_inner_bot);
        glVertex3f(baseRadius * cos(angle2), 0, baseRadius * sin(angle2));
        glVertex3f(topRadius * cos(angle2), height, topRadius * sin(angle2));
        glVertex3f(x_inner_top, height, z_inner_top);
    }
    glEnd();

    // Draw bottom cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, -1, 0);
    glVertex3f(0, 0, 0);
    for (int i = 0; i <= numFlutes; i++) {
        float angle = i * angleStep;
        glVertex3f(baseRadius * cos(angle), 0, baseRadius * sin(angle));
    }
    glEnd();

    // Draw top cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 1, 0);
    glVertex3f(0, height, 0);
    for (int i = 0; i <= numFlutes; i++) {
        float angle = i * angleStep;
        glVertex3f(topRadius * cos(angle), height, topRadius * sin(angle));
    }
    glEnd();
}

// Fungsi untuk menggambar silinder dengan radius dan posisi yang bisa diatur
void drawCustomCylinder(float x, float y, float z, float radius, float height, int segments = 32) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(-90, 1, 0, 0); // Agar sumbu Y menjadi sumbu utama silinder
    GLUquadric* quad = gluNewQuadric();
    gluCylinder(quad, radius, radius, height, segments, 1);
    // Tutup bawah
    glPushMatrix();
    glRotatef(180, 1, 0, 0);
    gluDisk(quad, 0, radius, segments, 1);
    glPopMatrix();
    // Tutup atas
    glPushMatrix();
    glTranslatef(0, 0, height);
    gluDisk(quad, 0, radius, segments, 1);
    glPopMatrix();
    gluDeleteQuadric(quad);
    glPopMatrix();
}

void drawTexturedCircle(float radius, GLuint textureID) {
    // Save current lighting state
    GLboolean lightingEnabled = glIsEnabled(GL_LIGHTING);
    GLboolean textureEnabled = glIsEnabled(GL_TEXTURE_2D);

    // Enable texturing
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture environment mode to replace for pure texture display
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    // Temporarily disable lighting for pure texture display
    glDisable(GL_LIGHTING);

    glBegin(GL_TRIANGLE_FAN);
    glTexCoord2f(0.5f, 0.5f); // Center of texture
    glVertex3f(0.0f, 0.0f, 0.0f); // Center of circle

    for (int angle = 0; angle <= 360; angle += 10) {
        float rad = angle * M_PI / 180.0f;
        float x = radius * cos(rad);
        float y = radius * sin(rad);

        // Proper texture coordinates (0 to 1 range)
        float u = (cos(rad) + 1.0f) * 0.5f;
        float v = (sin(rad) + 1.0f) * 0.5f;

        glTexCoord2f(u, v);
        glVertex3f(x, y, 0.0f);
    }
    glEnd();

    // Restore previous states
    glBindTexture(GL_TEXTURE_2D, 0);
    if (!textureEnabled) glDisable(GL_TEXTURE_2D);
    if (lightingEnabled) glEnable(GL_LIGHTING);
}

// Alternative: Simple textured quad (easier to debug)
void drawTexturedQuad(float size, GLuint textureID) {
    // Save current lighting state
    GLboolean lightingEnabled = glIsEnabled(GL_LIGHTING);
    GLboolean textureEnabled = glIsEnabled(GL_TEXTURE_2D);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glDisable(GL_LIGHTING);

    float half = size * 0.5f;

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-half, -half, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( half, -half, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( half,  half, 0.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-half,  half, 0.0f);
    glEnd();

    // Restore previous states
    glBindTexture(GL_TEXTURE_2D, 0);
    if (!textureEnabled) glDisable(GL_TEXTURE_2D);
    if (lightingEnabled) glEnable(GL_LIGHTING);
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
void drawLogo(float x, float y, float z, float size, float rotX = 0, float rotY = 0, float rotZ = 0, bool useCircle = false) {

    glPushMatrix();
    glTranslatef(x, y, z);

    // Apply rotations in order: X, Y, Z
    if (rotX != 0) glRotatef(rotX, 1, 0, 0);
    if (rotY != 0) glRotatef(rotY, 0, 1, 0);
    if (rotZ != 0) glRotatef(rotZ, 0, 0, 1);

    if (useCircle) {
        drawTexturedCircle(size * 0.5f, logoTexture);  // radius is half the size
    } else {
        drawTexturedQuad(size, logoTexture);
    }

    glPopMatrix();
}

void renderStrokeTextAtBold(const char* text, float x, float y, float z,
                            float rotX = 0.0f, float rotY = 0.0f, float rotZ = 0.0f,
                            float scale = 0.01f, int weight = 2) {
    for (int i = 0; i < weight; ++i) {
        for (int j = 0; j < weight; ++j) {
            glPushMatrix();

            glTranslatef(x + i * 0.001f, y + j * 0.001f, z);  // Slight offset
            glRotatef(rotX, 1, 0, 0);
            glRotatef(rotY, 0, 1, 0);
            glRotatef(rotZ, 0, 0, 1);
            glScalef(scale, scale, scale);

            for (const char* c = text; *c != '\0'; ++c) {
                glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
            }

            glPopMatrix();
        }
    }
}



// ========== KOMPONEN UTAMA ==========
/**
 * @brief Menggambar gedung bank dua lantai seperti pada gambar referensi.
 * @param x Posisi X dunia (pusat bangunan).
 * @param y Posisi Y dunia (dasar bangunan).
 * @param z Posisi Z dunia (pusat bangunan).
 */
/**
 * @brief Menggambar gedung bank dua lantai dengan 4 mesin ATM.
 * @param x Posisi X dunia (pusat bangunan).
 * @param y Posisi Y dunia (dasar bangunan).
 * @param z Posisi Z dunia (pusat bangunan).
 */
/**
 * @brief Menggambar gedung bank dengan 4 ATM dan parameter rotasi.
 * @param x Posisi X dunia (pusat bangunan).
 * @param y Posisi Y dunia (dasar bangunan).
 * @param z Posisi Z dunia (pusat bangunan).
 * @param angleY Sudut rotasi pada sumbu Y (dalam derajat).
 */
void drawBankBuilding(float x, float y, float z, float angleY) {
    glPushMatrix();
    glTranslatef(x, y, z);
    // Tambahkan rotasi pada sumbu Y
    glRotatef(angleY, 0.0f, 1.0f, 0.0f);

    // --- Lantai 1 (Area Bawah & Pilar) ---
    setRealisticMaterial(0.8f, 0.8f, 0.8f, 5.0f, 0.1f); // Abu-abu terang untuk pilar
    drawBox(-7.0f, 2.5f, 0.0f, 2.0f, 5.0f, 8.0f); // Pilar kiri
    drawBox(7.0f, 2.5f, 0.0f, 2.0f, 5.0f, 8.0f);  // Pilar kanan

    // Dinding Kaca (disimulasikan dengan box gelap)
    setRealisticMaterial(0.1f, 0.15f, 0.2f, 0.0f, 0.8f); // Kaca gelap reflektif
    drawBox(0.0f, 2.5f, -0.5f, 12.0f, 4.8f, 7.0f);

    // --- Area 4 Mesin ATM ---
    setRealisticMaterial(0.35f, 0.4f, 0.45f, 0.0f, 0.1f); // Abu-abu netral
    drawBox(0.0f, 2.4f, 3.4f, 11.5f, 4.8f, 0.4f);

    float startX = -4.5f;
    float atmWidth = 2.0f;
    float atmSpacing = 1.0f;

    for (int i = 0; i < 4; ++i) {
        float currentX = startX + i * (atmWidth + atmSpacing);
        glPushMatrix();
        glTranslatef(currentX, 0.0f, 0.0f);
        setRealisticMaterial(0.1f, 0.4f, 0.8f, 0.0f, 0.3f);
        drawBox(0.0f, 2.0f, 3.7f, atmWidth, 4.0f, 0.5f);
        setRealisticMaterial(0.1f, 0.8f, 1.0f, 0.0f, 0.9f);
        drawBox(0.0f, 3.0f, 4.0f, 1.4f, 1.2f, 0.1f);
        setRealisticMaterial(0.1f, 0.1f, 0.1f, 1.0f, 0.1f);
        drawBox(0.0f, 1.5f, 4.0f, 1.0f, 0.3f, 0.1f);
        drawBox(0.0f, 0.8f, 4.0f, 1.4f, 0.3f, 0.1f);
        glPopMatrix();
    }

    // --- Lantai 2 (Dinding Putih Atas) ---
    setRealisticMaterial(0.95f, 0.95f, 0.93f, 5.0f, 0.1f); // Putih bersih
    drawBox(0.0f, 7.5f, 0.0f, 16.0f, 5.0f, 8.5f);

    // Jendela Lantai 2
    setRealisticMaterial(0.1f, 0.15f, 0.2f, 20.0f, 0.8f);
    drawBox(0.0f, 8.0f, 4.3f, 14.0f, 3.0f, 0.2f);

    // // --- Atap ---
    // setRealisticMaterial(0.6f, 0.25f, 0.2f, 2.0f, 0.1f); // Warna genteng

    // // Sisi Kanan Atap
    // glPushMatrix();
    // glTranslatef(4.5f, 11.5f, 0.0f);
    // glRotatef(45.0f, 0.0f, 0.0f, 1.0f);
    // drawBox(0.0f, 0.0f, 0.0f, 9.0f, 1.0f, 9.5f);
    // glPopMatrix();

    // // Sisi Kiri Atap
    // glPushMatrix();
    // glTranslatef(-4.5f, 11.5f, 0.0f);
    // glRotatef(-45.0f, 0.0f, 0.0f, 1.0f);
    // drawBox(0.0f, 0.0f, 0.0f, 9.0f, 1.0f, 9.5f);
    // glPopMatrix();

    // // Dinding Segitiga Penutup Atap
    // setRealisticMaterial(0.95f, 0.95f, 0.93f, 5.0f, 0.1f);
    // glBegin(GL_TRIANGLES);
    //     // Depan
    //     glNormal3f(0.0, 0.0, 1.0);
    //     glVertex3f(-8.0f, 10.0f, 4.25f);
    //     glVertex3f(8.0f, 10.0f, 4.25f);
    //     glVertex3f(0.0f, 15.0f, 4.25f);
    //     // Belakang
    //     glNormal3f(0.0, 0.0, -1.0);
    //     glVertex3f(-8.0f, 10.0f, -4.25f);
    //     glVertex3f(8.0f, 10.0f, -4.25f);
    //     glVertex3f(0.0f, 15.0f, -4.25f);
    // glEnd();

    glPopMatrix();
}

void drawRoadMarkings() {
    setRealisticMaterial(0.9f, 0.9f, 0.9f, 0.0f, 0.05f);
    float lineLength = 4.0f; // Panjang setiap garis putih
    float gapLength = 5.0f;  // Jarak antar garis
    float step = lineLength + gapLength;

    // Posisi Y sedikit di atas jalan raya untuk menghindari Z-fighting (glitch)
    float y_pos = 0.21f;

    // Loop untuk menggambar garis putus-putus di sepanjang sumbu X
    for (float x = -50.0f; x <= 50.0f; x += step) {
        // drawBox(posisi_x, posisi_y, posisi_z, panjang, ketebalan, lebar_garis)
        drawBox(x, y_pos, 30.0f, lineLength, 0.02f, 0.3f);
    }
}

void drawGround() {
    // Lapisan 1: Rumput sebagai dasar
    setRealisticMaterial(0.2f, 0.5f, 0.2f, 0.0f, 0.1f);
    drawBox(0.0f, -0.15f, 0.0f, 100.0f, 0.1f, 100.0f);

    // Lapisan 2: Trotoar di depan gerbang
    setRealisticMaterial(0.7f, 0.7f, 0.65f, 0.0f, 0.1f);
    drawBox(0.0f, 0.0f, 4.0f, 100.0f, 0.2f, 20.0f);

    // Lapisan 3: Jalan raya

    setRealisticMaterial(0.2f, 0.2f, 0.22f, 0.0f, 0.2f);
    drawBox(0.0f, -0.1f, 20.0f, 100.0f, 0.1f, 50.0f);

    //jalan kedalam
    setRealisticMaterial(0.2f, 0.2f, 0.22f, 0.0f, 0.2f);
    drawBox(0.0f, -0.1f, -20.0f, 25.0f, 0.1f, 58.0f);

    //trotoar dalam sebelah kiri
    setRealisticMaterial(0.2f, 0.2f, 0.22f, 0.0f, 0.2f);
    drawBox(-14.5f, 0.1f, -26.0f, 5.0f, 0.2f, -40.0f);

    //trotoar dalam sebelah kanan
    setRealisticMaterial(0.2f, 0.2f, 0.22f, 0.0f, 0.2f);
    drawBox(14.5f, 0.1f, -26.0f, 5.0f, 0.2f, -40.0f);


}
void drawGreekPillar(float x, float y, float z) {
    glPushMatrix();
    // Move the entire pillar to the desired position
    glTranslatef(x, y, z);

    // Set material to a white marble/stone color
    setRealisticMaterial(0.9f, 0.88f, 0.85f, 5.0f, 0.2f);

    // --- 1. Pillar Base ---
    // A square plinth at the very bottom
    drawBox(0, 0.25f, 0, 2.0f, 0.5f, 2.0f);
    // A cylindrical base on top of the plinth
    glPushMatrix();
    glTranslatef(0, 0.5f, 0);
    drawTaperedCylinder(0.9f, 1.0f, 0.3f, 32);
    glPopMatrix();

    // --- 2. Pillar Shaft ---
    glPushMatrix();
    glTranslatef(0, 0.8f, 0); // Position shaft on top of the base
    drawFlutedCylinder(0.8f, 0.7f, 10.0f, 20); // baseRadius, topRadius, height, numFlutes
    glPopMatrix();

    // --- 3. Pillar Capital (Top) ---
    // Echinus (the flared, cushion-like part)
    glPushMatrix();
    glTranslatef(0, 10.8f, 0); // Position capital on top of the shaft
    drawTaperedCylinder(0.7f, 1.1f, 0.5f, 32);
    glPopMatrix();
    // Abacus (the square slab on top)
    drawBox(0, 11.3f, 0, 2.2f, 0.4f, 2.2f);

    glPopMatrix();
}

void drawMainPillar(float x, float angle) {
    glPushMatrix();

    // Pindahkan poros rotasi ke posisi pilar (misalnya bagian bawah pilar)
    glTranslatef(x, 1.0f, -4.0f);
    glRotatef(angle, 0.0f, 1.0f, 0.0f);  // Rotasi terhadap sumbu Y
    glTranslatef(-x, -1.0f, 4.0f);       // Kembalikan posisi ke asal

    // Bagian bawah (Base)
    setRealisticMaterial(0.75f, 0.60f, 0.45f, 0.1f);
    drawBox(x, 1.0f, -4.0f, 2.5f, 2.0f, 6.0f);

    // Pilar utama
    drawBox(x, 6.0f, -4.0f, 2.2f, 10.0f, 5.0f);

    // Bagian atas (Top)
    drawBox(x - 1.5f, 11.5f, -4.0f, 5.1f, 1.0f, 5.0f);

    glPopMatrix();
}


void drawCenterPillar() {

    //pilar utama
    setRealisticMaterial(0.75f, 0.60f, 0.45f, 0.0f,0.3f);
    drawBox(0, 8.0f, -4.0f, 7.0f, 16.0f, 5.5f);

    setRealisticMaterial(0.75f, 0.60f, 0.45f, 0.0f,0.3f);
    drawBox(0, 16.5f, -3.0f, 2.8f, 1.0f, 3.5f);
    //tempat logo
    setRealisticMaterial(0.9f, 0.88f, 0.85f, 0.0f,0.3f);
    drawBox(0, 14.3f, -1.1f, 3.5f, 3.0f, 0.4f);

    // Parameters: x, y, z, size, rotX, rotY, rotZ, useCircle
    drawLogo(0.0f, 14.3f, -0.89f, 3.0f, 180, 0, 0, true);  // Front face

    // --- Tulisan: putih saat siang, kuning menyala saat malam & lampu ON, gelap saat malam & lampu OFF ---
    float textR, textG, textB;
    if (dayNightTransition <= 0.3f) {
        // Siang: putih
        textR = 1.0f; textG = 1.0f; textB = 1.0f;
    } else if (lampLightOn && (isNightTime || dayNightTransition > 0.3f)) {
        // Malam & lampu ON: kuning menyala
        textR = 1.0f; textG = 0.85f; textB = 0.2f;
    } else {
        // Malam & lampu OFF: gelap
        textR = 0.18f; textG = 0.18f; textB = 0.18f;
    }
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING); // Pakai warna flat biar "menyala"
    glColor3f(textR, textG, textB);
    renderStrokeTextAtBold("UNIVERSITAS KRISTEN",
                   -2.0f, 12.1f, -1.1f, //posisi x,y,z
                   0, 0, 0,    // Rotasi X, Y, Z
                   0.003f,20);              // Skala teks
    renderStrokeTextAtBold("SATYA WACANA",
                   -2.0f, 11.5f, -1.1f, //posisi x,y,z
                   0, 0, 0,    // Rotasi X, Y, Z
                   0.004f,20);              // Skala teks

                   renderStrokeTextAtBold("KAMPUS UKSW",
                   -1.5f, 6.9f, -1.1f, //posisi x,y,z
                   0, 0, 0,    // Rotasi X, Y, Z
                   0.003f,20);              // Skala teks
    renderStrokeTextAtBold("1NDONESIA",
                   -2.6f, 6.0f, -1.1f, //posisi x,y,z
                   0, 0, 0,    // Rotasi X, Y, Z
                   0.008f,20);              // Skala teks

    renderStrokeTextAtBold("Mini",
                   0.5f, 5.5f, -1.1f, //posisi x,y,z
                   0, 0, 0,    // Rotasi X, Y, Z
                   0.004f,20);              // Skala teks
    glPopAttrib();

}

void drawSideWall(float x, float y, float z) {
    setRealisticMaterial(0.75f, 0.60f, 0.45f, 1.0f,0.3f);

    drawBox(x, y, z, 5.6f, 16.0f, 4.0f); // Ukuran default, bisa kamu ubah sesuai desain
}

void drawSideWallAbove(float x, float y, float z) {
    setRealisticMaterial(0.75f, 0.60f, 0.45f, 1.0f,0.3f);

    drawBox(x, y, z, 4.0f, 7.0f, 4.0f); // Ukuran default, bisa kamu ubah sesuai desain
}



void drawConnectingBeamAt(float x, float y, float z) {
    setRealisticMaterial(0.85f, 0.85f, 0.85f, 0.0f,0.3f);

    // Main beam (long horizontal)
    drawBox(x, y, z, 22.0f, 0.6f, 0.5f);

    // Left connector
    drawBox(x - 9.5f, y, z, 1.0f, 0.6f, 1.0f);

    // Right connector
    drawBox(x + 9.5f, y, z, 1.0f, 0.6f, 1.0f);
}

// Main function to render the 3D cartoon tree
void renderCartoonTree3D(float x, float y, float z, float scale = 1.0f) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(scale, scale, scale);

    // --- TRUNK ---
    setRealisticMaterial(0.65f, 0.32f, 0.15f, 0.5f,0.3f);

    // Main trunk (twisted) - batang diperkecil
    glPushMatrix();
    drawTwistedCylinder(1.5f, 1.2f, 15.0f, 20); // batang lebih kecil
    glPopMatrix();

    // --- MAIN BRANCHES ---
    setRealisticMaterial(0.35f, 0.18f, 0.08f, 16.0f); // Slightly darker brown for branches


    // --- FOLIAGE CANOPY ---
    // Create multiple overlapping spheres for fluffy appearance

    // Back layer spheres - Dark green
    setRealisticMaterial(0.1f, 0.4f, 0.1f, 64.0f);

    glPushMatrix();
    glTranslatef(-3.5f, 14.0f, -2.0f);
    drawColoredSphere(3.2f, 12, 10);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(3.0f, 14.5f, -1.5f);
    drawColoredSphere(3.0f, 12, 10);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, 15.0f, -3.0f);
    drawColoredSphere(2.8f, 12, 10);
    glPopMatrix();

    // Middle layer spheres - Medium green
    setRealisticMaterial(0.2f, 0.6f, 0.2f, 64.0f);

    glPushMatrix();
    glTranslatef(-2.5f, 15.0f, -0.5f);
    drawColoredSphere(3.5f, 14, 12);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(2.8f, 15.5f, 0.2f);
    drawColoredSphere(3.3f, 14, 12);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.5f, 16.0f, -1.0f);
    drawColoredSphere(3.0f, 14, 12);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-1.0f, 14.5f, 1.5f);
    drawColoredSphere(2.7f, 14, 12);
    glPopMatrix();

    // Front layer spheres - Bright green
    setRealisticMaterial(0.3f, 0.8f, 0.3f, 64.0f);

    glPushMatrix();
    glTranslatef(-1.5f, 15.5f, 1.0f);
    drawColoredSphere(3.8f, 16, 14);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(2.0f, 16.0f, 1.5f);
    drawColoredSphere(3.6f, 16, 14);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, 17.0f, 0.5f);
    drawColoredSphere(3.2f, 16, 14);
    glPopMatrix();

    // Top highlight spheres - Very bright green
    setRealisticMaterial(0.5f, 1.0f, 0.5f, 128.0f);

    glPushMatrix();
    glTranslatef(-0.5f, 16.5f, 2.0f);
    drawColoredSphere(2.5f, 16, 14);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(1.5f, 17.0f, 2.2f);
    drawColoredSphere(2.3f, 16, 14);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.8f, 18.0f, 1.0f);
    drawColoredSphere(2.0f, 16, 14);
    glPopMatrix();

    // Small detail spheres for extra fluffiness
    setRealisticMaterial(0.4f, 0.9f, 0.4f, 128.0f);

    glPushMatrix();
    glTranslatef(-2.0f, 17.0f, 2.5f);
    drawColoredSphere(1.8f, 12, 10);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(2.5f, 17.5f, 2.8f);
    drawColoredSphere(1.5f, 12, 10);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, 18.5f, 2.0f);
    drawColoredSphere(1.2f, 12, 10);
    glPopMatrix();

    glPopMatrix();
}


void drawLamppost(float x, float y, float z, float scale = 1.0f, float rotX = 0.0f, float rotY = 0.0f, float rotZ = 0.0f) {
    glPushMatrix();
    glTranslatef(x, y, z);

    if (rotX != 0.0f) glRotatef(rotX, 1, 0, 0);
    if (rotY != 0.0f) glRotatef(rotY, 0, 1, 0);
    if (rotZ != 0.0f) glRotatef(rotZ, 0, 0, 1);

    glScalef(scale, scale, scale);

    // Base platform
    setRealisticMaterial(0.3f, 0.3f, 0.3f, 10.0f, 0.2f);
    glPushMatrix();
    glTranslatef(0, 0.2f, 0);
    drawTaperedCylinder(1.2f, 1.0f, 0.4f, 20);
    glPopMatrix();

    // Main pole - metallic
    setRealisticMaterial(0.2f, 0.2f, 0.2f, 10.0f, 0.8f);
    glPushMatrix();
    glTranslatef(0, 0.6f, 0);
    drawTaperedCylinder(0.15f, 0.12f, 8.0f, 16);
    glPopMatrix();

    // Decorative rings - shinier metal
    setRealisticMaterial(0.4f, 0.4f, 0.4f, 10.0f, 0.9f);
    glPushMatrix();
    glTranslatef(0, 1.5f, 0);
    drawTaperedCylinder(0.18f, 0.16f, 0.2f, 16);
    glPopMatrix();

    // Lamp arm
    setRealisticMaterial(0.25f, 0.25f, 0.25f, 10.0f, 0.6f);
    glPushMatrix();
    glTranslatef(0.8f, 8.2f, 0);
    glRotatef(90, 0, 0, 1);
    drawTaperedCylinder(0.08f, 0.06f, 1.6f, 12);
    glPopMatrix();

    // Lamp shade
    setRealisticMaterial(0.15f, 0.15f, 0.15f, 10.0f, 0.3f);
    glPushMatrix();
    glTranslatef(0.0f, 8.89f, 0);
    drawTaperedCylinder(0.6f, 0.4f, 0.8f, 20);
    glPopMatrix();

    // Enhanced lamp lighting logic
    bool shouldLampBeOn = lampLightOn && (isNightTime || dayNightTransition > 0.3f);

    if (shouldLampBeOn) {
        // Brighter at night, dimmer during day
        float lampIntensity = 0.3f + 0.7f * dayNightTransition;
        setRealisticMaterial(1.0f * lampIntensity, 0.9f * lampIntensity, 0.7f * lampIntensity, 300.0f, 0.5f);

        // Lamp position in local coordinates (relative to lamppost origin)
        GLfloat lampPos[] = {1.6f, 8.5f, 0.0f, 1.0f};
        GLfloat lampAmbient[] = {0.3f * lampIntensity, 0.25f * lampIntensity, 0.15f * lampIntensity, 1.0f};
        GLfloat lampDiffuse[] = {1.0f * lampIntensity, 0.8f * lampIntensity, 0.6f * lampIntensity, 1.0f};
        GLfloat lampSpecular[] = {1.0f, 0.9f, 0.8f, 1.0f};

        glEnable(lampLightID);
        // Set lamp light position after all transforms (so it follows the lamppost in world space)
        glLightfv(lampLightID, GL_POSITION, lampPos);
        glLightfv(lampLightID, GL_AMBIENT, lampAmbient);
        glLightfv(lampLightID, GL_DIFFUSE, lampDiffuse);
        glLightfv(lampLightID, GL_SPECULAR, lampSpecular);

        // Enhanced attenuation for night effect
        glLightf(lampLightID, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(lampLightID, GL_LINEAR_ATTENUATION, 0.02f);
        glLightf(lampLightID, GL_QUADRATIC_ATTENUATION, 0.005f);
    } else {
        setRealisticMaterial(0.3f, 0.3f, 0.3f, 0.0f, 0.1f);
        glDisable(lampLightID);
    }

    // Draw the bulb
    glPushMatrix();
    glTranslatef(0.0f, 8.8f, 0);
    drawColoredSphere(0.35f, 16, 12);
    glPopMatrix();

    glPopMatrix();
}
void drawTamanOval(float centerX, float centerZ, float radiusX, float radiusZ) {
    const int segments = 100;

    // Rumput oval
    glColor3f(0.3f, 0.7f, 0.3f); // hijau rumput
    glBegin(GL_POLYGON);
    for (int i = 0; i < segments; ++i) {
        float theta = 2.0f * 3.1415926f * float(i) / float(segments);
        float x = radiusX * cosf(theta);
        float z = radiusZ * sinf(theta);
        glVertex3f(centerX + x, 0.01f, centerZ + z);
    }
    glEnd();

    // Batu pembatas oval (hitam-putih)
    for (int i = 0; i < segments; ++i) {
        float theta1 = 2.0f * 3.1415926f * float(i) / float(segments);
        float theta2 = 2.0f * 3.1415926f * float(i + 1) / float(segments);
        float x1 = radiusX * cosf(theta1);
        float z1 = radiusZ * sinf(theta1);
        float x2 = radiusX * cosf(theta2);
        float z2 = radiusZ * sinf(theta2);

        if (i % 2 == 0)
            glColor3f(1.0f, 1.0f, 1.0f); // putih
        else
            glColor3f(0.0f, 0.0f, 0.0f); // hitam

        glBegin(GL_QUADS);
        glVertex3f(centerX + x1, 0.01f, centerZ + z1);
        glVertex3f(centerX + x2, 0.01f, centerZ + z2);
        glVertex3f(centerX + x2, 0.3f, centerZ + z2);
        glVertex3f(centerX + x1, 0.3f, centerZ + z1);
        glEnd();
    }
}
void drawTiangBendera(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);  // 🔁 Rotasi agar silinder tegak (Z -> Y)

    // Tiang
    glColor3f(0.8f, 0.8f, 0.8f);
    GLUquadric *quad = gluNewQuadric();
    gluCylinder(quad, 0.05, 0.05, 6.0, 8, 8); // tinggi 6 satuan

    // Bola di atas tiang
    glTranslatef(0.0f, 0.0f, 6.0f); // pindah ke atas ujung silinder
    glColor3f(1.0f, 1.0f, 0.0f); // kuning
    glutSolidSphere(0.1, 12, 12);

    glPopMatrix();
}
// ========== KOMPONEN TAMBAHAN (SESUAI GAMBAR REFERENSI) ==========

/**
 * @brief Menggambar kolam air mancur sesuai gambar referensi.
 * @param x Posisi X dunia (pusat kolam).
 * @param y Posisi Y dunia (dasar kolam).
 * @param z Posisi Z dunia (pusat kolam).
 */
void drawFountainPool(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);

    // --- 1. Basin Kolam Utama ---
    setRealisticMaterial(0.7f, 0.7f, 0.65f, 5.0f, 0.1f); // Warna batu/beton
    // Dinding luar
    drawCustomCylinder(0.0f, 0.0f, 0.0f, 8.0f, 1.0f, 64);
    // Dinding dalam
    drawCustomCylinder(0.0f, 0.0f, 0.0f, 7.5f, 1.0f, 64);

    // Lantai kolam (di bawah air)
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    gluDisk(gluNewQuadric(), 0, 7.5, 64, 1);
    glPopMatrix();


    // --- 2. Air Kolam (Dengan Transparansi) ---
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Material untuk air: hijau kebiruan, 70% opacity
    GLfloat waterAmbient[] = {0.1f, 0.3f, 0.25f, 0.7f};
    GLfloat waterDiffuse[] = {0.2f, 0.6f, 0.5f, 0.7f};
    GLfloat waterSpecular[] = {0.8f, 0.9f, 0.85f, 0.7f};
    glMaterialfv(GL_FRONT, GL_AMBIENT, waterAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, waterDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, waterSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, 80.0f);

    glPushMatrix();
    glTranslatef(0, 0.8f, 0); // Posisikan permukaan air sedikit di bawah bibir kolam
    glRotatef(-90, 1, 0, 0);
    gluDisk(gluNewQuadric(), 0, 7.5, 64, 1);
    glPopMatrix();

    glDisable(GL_BLEND); // Matikan blending setelah selesai


    // --- 3. Struktur Air Mancur Tengah ---
    setRealisticMaterial(0.6f, 0.6f, 0.6f, 5.0f, 0.2f); // Abu-abu batu
    // Base lebar
    drawCustomCylinder(0.0f, 0.0f, 0.0f, 2.5f, 0.5f, 16);
    // Tingkat kedua
    drawCustomCylinder(0.0f, 0.5f, 0.0f, 2.0f, 1.5f, 16);
    // Bagian tengah dengan logo
    drawBox(0.0f, 2.5f, 0.0f, 1.5f, 2.0f, 1.5f);
    // Logo di sisi depan
    drawLogo(0.0f, 2.5f, 0.76f, 1.4f, 0, 0, 0, true);
    // Tingkat atas
    drawCustomCylinder(0.0f, 3.5f, 0.0f, 1.8f, 0.5f, 16);

    // --- 4. Simulasi Pancaran Air ---
    setRealisticMaterial(0.8f, 0.9f, 1.0f, 100.0f, 0.9f); // Warna air mancur putih kebiruan
    // Pancaran utama
    drawTaperedCylinder(0.1f, 0.02f, 4.0f, 8);
    // Pancaran kecil di sekitar
    for (int i = 0; i < 8; ++i) {
        glPushMatrix();
        glRotatef(i * 45.0f, 0, 1, 0); // Putar di sekitar sumbu Y
        glTranslatef(1.5f, 1.8f, 0.0f); // Pindahkan ke pinggir
        glRotatef(-20, 0, 0, 1); // Arahkan sedikit keluar
        drawTaperedCylinder(0.05f, 0.01f, 2.0f, 6);
        glPopMatrix();
    }

    glPopMatrix();
}

void drawGardenBush(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);

    // --- Batang Kayu ---
    // Menggunakan silinder meruncing yang pendek dan berwarna coklat tua.
    setRealisticMaterial(0.4f, 0.25f, 0.15f, 2.0f, 0.1f);
    drawTaperedCylinder(0.3f, 0.2f, 1.0f, 8); // radiusBawah, radiusAtas, tinggi, segment

    // --- Dedaunan ---
    // Dibuat dari beberapa bola hijau yang tumpang tindih untuk tampilan lebat.
    glPushMatrix();
    glTranslatef(0.0f, 1.0f, 0.0f); // Posisikan dedaunan di atas batang

    // Lapisan daun utama (hijau sedang)
    setRealisticMaterial(0.2f, 0.6f, 0.2f, 10.0f, 0.2f);
    drawColoredSphere(1.0f, 12, 10); // radius, slices, stacks

    // Lapisan daun tambahan untuk variasi bentuk dan warna
    setRealisticMaterial(0.3f, 0.7f, 0.3f, 15.0f, 0.2f); // Warna lebih terang

    // Bola tambahan di kanan atas
    glPushMatrix();
    glTranslatef(0.4f, 0.3f, 0.3f);
    glScalef(0.8f, 0.8f, 0.8f); // Buat ukurannya sedikit lebih kecil
    drawColoredSphere(0.8f, 10, 8);
    glPopMatrix();

    // Bola tambahan di kiri bawah
    glPushMatrix();
    glTranslatef(-0.3f, 0.2f, -0.5f);
    glScalef(0.7f, 0.7f, 0.7f);
    drawColoredSphere(0.9f, 10, 8);
    glPopMatrix();

    glPopMatrix(); // Kembali dari translasi dedaunan

    glPopMatrix(); // Selesai menggambar semak
}

/**
 * @brief Menggambar tulisan "UKSW" 3D besar di belakang kolam.
 * @param x Posisi X dunia (pusat tulisan).
 * @param y Posisi Y dunia (dasar tulisan).
 * @param z Posisi Z dunia (pusat tulisan).
 */
/**
 * @brief Menggambar tulisan "UKSW" 3D besar di belakang kolam. (VERSI DIPERBAIKI)
 * @param x Posisi X dunia (pusat tulisan).
 * @param y Posisi Y dunia (dasar tulisan).
 * @param z Posisi Z dunia (pusat tulisan).
 */
void drawUKSWText(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    // Ukuran diperkecil dari 1.5f menjadi 1.0f untuk tampilan yang lebih pas
    glScalef(1.0f, 1.0f, 1.0f);

    // Material biru cerah untuk tulisan
    setRealisticMaterial(0.3f, 0.7f, 1.0f, 0.8f, 0.5f);

    // Variabel untuk kontrol ukuran huruf agar mudah diubah
    float letterHeight = 3.5f;
    float letterWidth  = 2.5f;
    float thickness    = 0.7f;
    float spacing      = 4.5f; // Jarak antar huruf

    // --- Huruf U ---
    glPushMatrix();
    glTranslatef(-spacing * 1.5f, 0.0f, 0.0f); // Posisi huruf U
    drawBox(0, thickness / 2, 0, letterWidth, thickness, thickness); // Bagian bawah
    drawBox(-(letterWidth-thickness)/2, letterHeight/2, 0, thickness, letterHeight - thickness, thickness); // Kaki kiri
    drawBox((letterWidth-thickness)/2, letterHeight/2, 0, thickness, letterHeight - thickness, thickness); // Kaki kanan
    glPopMatrix();

    // --- Huruf K ---
    glPushMatrix();
    glTranslatef(-spacing * 0.5f, 0.0f, 0.0f); // Posisi huruf K
    drawBox(-(letterWidth/2) + thickness, letterHeight/2, 0, thickness, letterHeight, thickness); // Tiang vertikal
    // Diagonal atas
    glPushMatrix();
    glTranslatef(0.1f, letterHeight/2 + 0.5f, 0); // Pivot rotasi di tengah
    glRotatef(45, 0, 0, 1);
    drawBox(0, 0, 0, letterHeight/1.5f, thickness, thickness);
    glPopMatrix();
    // Diagonal bawah
    glPushMatrix();
    glTranslatef(0.1f, letterHeight/2 - 0.5f, 0); // Pivot rotasi di tengah
    glRotatef(-45, 0, 0, 1);
    drawBox(0, 0, 0, letterHeight/1.5f, thickness, thickness);
    glPopMatrix();
    glPopMatrix();

    // --- Huruf S ---
    glPushMatrix();
    glTranslatef(spacing * 0.5f, 0.0f, 0.0f); // Posisi huruf S
    drawBox(0, letterHeight - thickness/2, 0, letterWidth, thickness, thickness); // Atas
    drawBox(-(letterWidth-thickness)/2, letterHeight - thickness*1.5, 0, thickness, thickness, thickness); // Kiri atas
    drawBox(0, letterHeight/2, 0, letterWidth, thickness, thickness); // Tengah
    drawBox((letterWidth-thickness)/2, thickness*1.5, 0, thickness, thickness, thickness); // Kanan bawah
    drawBox(0, thickness/2, 0, letterWidth, thickness, thickness); // Bawah
    glPopMatrix();

    // --- Huruf W ---
    glPushMatrix();
    glTranslatef(spacing * 1.5f, 0.0f, 0.0f); // Posisi huruf W
    float angleW = 12.0f;
    // Kaki kiri luar
    glPushMatrix();
    glTranslatef(-1.2f, letterHeight/2, 0);
    glRotatef(angleW, 0, 0, 1);
    drawBox(0, 0, 0, thickness, letterHeight, thickness);
    glPopMatrix();
    // Kaki kiri dalam
    glPushMatrix();
    glTranslatef(-0.4f, (letterHeight-thickness)/4, 0);
    glRotatef(-angleW, 0, 0, 1);
    drawBox(0, 0, 0, thickness, (letterHeight-thickness)/2, thickness);
    glPopMatrix();
    // Kaki kanan dalam
    glPushMatrix();
    glTranslatef(0.4f, (letterHeight-thickness)/4, 0);
    glRotatef(angleW, 0, 0, 1);
    drawBox(0, 0, 0, thickness, (letterHeight-thickness)/2, thickness);
    glPopMatrix();
    // Kaki kanan luar
    glPushMatrix();
    glTranslatef(1.2f, letterHeight/2, 0);
    glRotatef(-angleW, 0, 0, 1);
    drawBox(0, 0, 0, thickness, letterHeight, thickness);
    glPopMatrix();
    glPopMatrix();

    glPopMatrix();
}
// Fungsi untuk menggambar satu pot bunga beserta tanamannya
void drawPlantPot(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0.1f, z); // Posisikan pot di atas trotoar

    // Gambar Pot menggunakan silinder meruncing
    // Bagian utama pot berwarna putih keabu-abuan
    setRealisticMaterial(0.9f, 0.9f, 0.9f, 1.0f, 0.2f);
    drawTaperedCylinder(0.4f, 0.6f, 0.8f, 16);

    // Tambahkan list biru di bagian atas pot
    glPushMatrix();
    glTranslatef(0, 0.7f, 0);
    setRealisticMaterial(0.2f, 0.3f, 0.8f, 1.0f, 0.3f);
    drawTaperedCylinder(0.61f, 0.61f, 0.1f, 16); // Sedikit lebih lebar dari pot
    glPopMatrix();

    // Gambar tanah di dalam pot
    glPushMatrix();
    glTranslatef(0, 0.8f, 0);
    setRealisticMaterial(0.4f, 0.25f, 0.15f, 1.0f, 0.1f);
    drawTaperedCylinder(0.55f, 0.55f, 0.05f, 16); // Disk tipis untuk tanah
    glPopMatrix();

    // Gambar Tanaman (beberapa bola hijau yang tumpang tindih)
    setRealisticMaterial(0.1f, 0.5f, 0.15f, 1.0f, 0.2f); // Warna daun
    glPushMatrix();
    glTranslatef(0.1f, 1.2f, 0.1f);
    drawColoredSphere(0.4f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.1f, 1.1f, -0.2f);
    drawColoredSphere(0.35f);
    glPopMatrix();

    setRealisticMaterial(0.2f, 0.7f, 0.25f, 1.0f, 0.2f); // Warna daun lebih terang
    glPushMatrix();
    glTranslatef(0.0f, 1.4f, 0.0f);
    drawColoredSphere(0.45f);
    glPopMatrix();

    glPopMatrix();
}

void drawPlanterBox(float x, float y, float z, float width, float height, float depth) {
    glPushMatrix();
    glTranslatef(x, y, z);

    // --- 1. Gambar Pot Persegi Panjang ---
    // Menggunakan material putih bersih seperti pada gambar referensi
    setRealisticMaterial(0.95f, 0.95f, 0.95f, 10.0f, 0.1f);
    drawBox(0.0f, height / 2.0f, 0.0f, width, height, depth);

    // --- 2. Gambar Tanah di Dalam Pot ---
    setRealisticMaterial(0.3f, 0.18f, 0.1f, 1.0f, 0.0f); // Warna tanah gelap
    glPushMatrix();
    // Posisi tanah sedikit di bawah bibir atas pot
    glTranslatef(0.0f, height - 0.1f, 0.0f);
    drawBox(0.0f, 0.0f, 0.0f, width * 0.95f, 0.2f, depth * 0.95f);
    glPopMatrix();

    // --- 3. Gambar Tanaman Lebat ---
    int numPlants = int(width * depth * 0.5f); // Jumlah tanaman berdasarkan luas pot
    srand(x * 100 + z); // Seed acak berdasarkan posisi agar setiap pot unik

    for (int i = 0; i < numPlants; ++i) {
        // Tentukan posisi acak untuk setiap tanaman di dalam pot
        float plantX = ((rand() % 100) / 100.0f - 0.5f) * width * 0.9f;
        float plantZ = ((rand() % 100) / 100.0f - 0.5f) * depth * 0.9f;

        glPushMatrix();
        glTranslatef(plantX, height, plantZ);

        // Gambar Batang (silinder tipis dan gelap)
        setRealisticMaterial(0.1f, 0.15f, 0.1f, 1.0f, 0.0f);
        float plantHeight = 1.5f + (rand() % 100) / 100.0f * 1.5f; // Tinggi acak
        drawTaperedCylinder(0.05f, 0.03f, plantHeight, 6);

        // Gambar Dedaunan (kumpulan bola hijau di atas batang)
        glTranslatef(0.0f, plantHeight, 0.0f);
        int numLeaves = 5 + (rand() % 5);
        for (int j = 0; j < numLeaves; ++j) {
            // Variasi warna hijau daun
            float r = 0.1f + (rand() % 10) / 50.0f;
            float g = 0.4f + (rand() % 20) / 100.0f;
            float b = 0.1f + (rand() % 10) / 50.0f;
            setRealisticMaterial(r, g, b, 5.0f, 0.1f);

            // Posisi acak untuk setiap gumpalan daun
            float leafX = ((rand() % 100) - 50) / 150.0f;
            float leafY = ((rand() % 100) / 100.0f) * 0.8f;
            float leafZ = ((rand() % 100) - 50) / 150.0f;

            glPushMatrix();
            glTranslatef(leafX, leafY, leafZ);
            drawColoredSphere(0.3f + (rand() % 10) / 50.0f, 6, 5);
            glPopMatrix();
        }
        glPopMatrix();
    }

    glPopMatrix();
}

// ... (setelah fungsi drawPlanterBox atau fungsi gambar lainnya)

/**
 * @brief Menggambar gumpalan rumput yang lebat.
 */
void drawGrassLump(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);

    int numBlades = 20; // Jumlah helai rumput per gumpalan
    for (int i = 0; i < numBlades; ++i) {
        glPushMatrix();

        // Warna hijau rumput dengan sedikit variasi
        setRealisticMaterial(0.1f + (rand() % 10) / 80.0f, 0.4f + (rand() % 20) / 100.0f, 0.1f, 2.0f, 0.05f);

        // Rotasi acak agar rumput tidak seragam
        glRotatef(((rand() % 40) - 20), 1.0f, 0.0f, 0.0f);
        glRotatef(((rand() % 40) - 20), 0.0f, 0.0f, 1.0f);

        float height = 0.8f + ((rand() % 100) / 100.0f) * 0.5f;
        drawTaperedCylinder(0.02f, 0.005f, height, 4); // Gambar helai rumput sebagai silinder tipis

        glPopMatrix();
    }
    glPopMatrix();
}
// ==========================================================
// ===== IMPLEMENTASI FUNGSI UNTUK TERMINAL BUS ==========
// ==========================================================

/**
 * @brief Menggambar pondasi, tangga, dan ramp untuk terminal.
 */
void drawTerminalBase(float width, float depth) {
    glPushMatrix();

    // 1. Lantai Platform Utama (abu-abu muda)
    setRealisticMaterial(0.7f, 0.7f, 0.7f, 1.0f, 0.1f);
    drawBox(0.0f, 0.8f, 0.0f, width, 0.4f, depth);

    // 2. Pondasi Batu Kasar (abu-abu gelap)
    setRealisticMaterial(0.3f, 0.3f, 0.32f, 1.0f, 0.1f);
    drawBox(0.0f, 0.2f, 0.0f, width + 0.2f, 0.6f, depth + 0.2f);

    // 3. Tangga (3 undakan)
    setRealisticMaterial(0.6f, 0.6f, 0.6f, 2.0f, 0.1f);
    for (int i = 0; i < 3; ++i) {
        drawBox(0.0f, 0.1f + i * 0.2f, (depth/2) + 0.3f + i * 0.3f, width * 0.6f, 0.2f, 0.3f);
    }

    // 4. Ramp / Jalan Landai di sisi kiri
    glPushMatrix();
    glTranslatef(-(width/2) + 0.75f, 0.4f, (depth/2) + 0.75f);
    glRotatef(20, 0, 1, 0); // Putar sedikit
    glRotatef(-15, 1, 0, 0); // Miringkan
    drawBox(0.0f, 0.0f, 0.0f, 1.5f, 0.1f, 2.5f);
    glPopMatrix();

    // 5. Pinggiran jalan (curb) hitam-putih
    for (float z = -depth/2; z < depth/2; z += 1.0f) {
        // Sisi kanan
        setRealisticMaterial(1.0f, 1.0f, 1.0f, 1.0f, 0.0f); // Putih
        drawBox(width/2 + 0.2f, 0.1f, z, 0.2f, 0.2f, 0.5f);
        setRealisticMaterial(0.1f, 0.1f, 0.1f, 1.0f, 0.0f); // Hitam
        drawBox(width/2 + 0.2f, 0.1f, z + 0.5f, 0.2f, 0.2f, 0.5f);
    }

    glPopMatrix();
}

/**
 * @brief Menggambar pilar-pilar penyangga atap.
 */
void drawTerminalPillars(float width, float depth, float height) {
    glPushMatrix();
    setRealisticMaterial(0.55f, 0.55f, 0.6f, 5.0f, 0.2f); // Warna batu pilar

    float pillar_w = 0.8f;
    float pillar_d = 0.8f;
    float offset_x = width/2 - pillar_w/2;
    float offset_z = depth/2 - pillar_d/2;

    // Pilar depan kiri
    drawBox(-offset_x, height/2 + 1.0f, -offset_z, pillar_w, height, pillar_d);
    // Pilar depan kanan
    drawBox(offset_x, height/2 + 1.0f, -offset_z, pillar_w, height, pillar_d);
    // Pilar belakang kiri
    drawBox(-offset_x, height/2 + 1.0f, offset_z, pillar_w, height, pillar_d);
    // Pilar belakang kanan
    drawBox(offset_x, height/2 + 1.0f, offset_z, pillar_w, height, pillar_d);

    glPopMatrix();
}

/**
 * @brief Menggambar atap terminal.
 */
void drawTerminalRoof(float width, float depth, float height) {
    glPushMatrix();

    // Posisi Y atap
    float roof_y = height + 1.0f; // 1.0f adalah tinggi platform

    // 1. Atap utama (putih/abu-abu sangat terang)
    setRealisticMaterial(0.9f, 0.9f, 0.9f, 10.0f, 0.1f);
    glPushMatrix();
    glTranslatef(0, roof_y, 0);
    glRotatef(3.0f, 1, 0, 0); // Miringkan sedikit ke belakang
    drawBox(0.0f, 0.0f, 0.0f, width + 2.0f, 0.5f, depth + 2.0f); // Atap lebih besar dari pilar
    glPopMatrix();

    // 2. Lis kayu/coklat di depan
    setRealisticMaterial(0.4f, 0.25f, 0.18f, 2.0f, 0.1f);
    drawBox(0.0f, roof_y + 0.05f, -(depth/2) - 1.0f, width + 2.0f, 0.6f, 0.2f);

    glPopMatrix();
}

/**
 * @brief Menggambar detail seperti meja, bangku, dan pagar.
 */
void drawTerminalDetails(float width, float depth) {
    glPushMatrix();

    // 1. Meja Jaga
    // Bagian bawah (batu gelap)
    setRealisticMaterial(0.2f, 0.18f, 0.18f, 50.0f, 0.6f);
    drawBox(width/4, 1.4f, depth/4, 2.5f, 0.8f, 1.5f);
    // Permukaan meja (abu-abu)
    setRealisticMaterial(0.5f, 0.5f, 0.5f, 10.0f, 0.2f);
    drawBox(width/4, 1.85f, depth/4, 2.7f, 0.1f, 1.7f);

    // 2. Dinding/Bangku Samping
    setRealisticMaterial(0.3f, 0.3f, 0.32f, 1.0f, 0.1f); // Batu gelap
    // Sisi kiri
    drawBox(-width/2 + 1.25f, 1.4f, 0.0f, 2.5f, 0.8f, depth);
    // Sisi kanan
    drawBox(width/2 - 0.5f, 1.4f, 0.0f, 1.0f, 0.8f, depth);
    // Bagian atas bangku (untuk duduk)
    setRealisticMaterial(0.7f, 0.7f, 0.7f, 1.0f, 0.1f); // Abu-abu muda
    drawBox(-width/2 + 1.25f, 1.85f, 0.0f, 2.5f, 0.1f, depth);
    drawBox(width/2 - 0.5f, 1.85f, 0.0f, 1.0f, 0.1f, depth);

    // 3. Pagar di Ramp
    setRealisticMaterial(0.2f, 0.2f, 0.2f, 80.0f, 0.8f); // Besi gelap mengkilap
    glPushMatrix();
    glTranslatef(-(width/2) - 0.2f, 1.0f, (depth/2) + 0.75f);
    glRotatef(20, 0, 1, 0);
    // Tiang-tiang
    drawCustomCylinder(0, 0, 0, 0.05f, 1.5f);
    drawCustomCylinder(0, 0, -1.0f, 0.05f, 1.3f);
    // Railing atas
    glPushMatrix();
    glTranslatef(0, 1.4f, -0.5f);
    glRotatef(90, 0, 1, 0);
    glRotatef(-15, 1, 0, 0);
    drawCustomCylinder(0, 0, 0, 0.05f, 1.2f);
    glPopMatrix();
    glPopMatrix();


    glPopMatrix();
}


/**
 * @brief Fungsi utama untuk menggambar seluruh struktur terminal bus.
 */
void drawBusTerminal(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);

    // Definisikan dimensi utama struktur
    float term_width = 12.0f;
    float term_depth = 8.0f;
    float term_height = 5.0f; // Tinggi pilar

    // Gambar setiap komponen
    drawTerminalBase(term_width, term_depth);
    drawTerminalPillars(term_width, term_depth, term_height);
    drawTerminalRoof(term_width, term_depth, term_height);
    drawTerminalDetails(term_width, term_depth);

   
    glPopMatrix();
}
/**
 * @brief Menggambar tanaman berdaun.
 */
void drawLeafyPlant(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);

    // Batang utama
    setRealisticMaterial(0.2f, 0.3f, 0.15f, 1.0f, 0.1f);
    float height = 1.2f + ((rand() % 100) / 100.0f) * 0.8f;
    drawTaperedCylinder(0.08f, 0.05f, height, 6);

    // Dedaunan (menggunakan quad)
    setRealisticMaterial(0.2f, 0.6f, 0.2f, 5.0f, 0.1f);
    int numLeaves = 6;
    for (int i = 0; i < numLeaves; ++i) {
        glPushMatrix();
        // Sebar daun di sepanjang batang
        glTranslatef(0.0f, height * (0.4f + (float)i / (numLeaves * 1.5f)), 0.0f);
        glRotatef(float(rand() % 360), 0.0f, 1.0f, 0.0f); // Rotasi Y acak
        glRotatef(30 + (rand() % 40), 1.0f, 0.0f, 1.0f); // Kemiringan daun

        // Gambar daun sebagai quad
        float leafSize = 0.4f;
        glBegin(GL_QUADS);
            glNormal3f(0.0, 1.0, 0.0);
            glVertex3f(0, 0, 0);
            glVertex3f(leafSize, leafSize * 0.3f, 0);
            glVertex3f(0, leafSize * 1.2f, 0);
            glVertex3f(-leafSize, leafSize * 0.3f, 0);
        glEnd();

        glPopMatrix();
    }
    glPopMatrix();
}


/**
 * @brief Menggambar pot persegi panjang abu-abu dengan bibir putih, diisi tanaman bervariasi.
 * @param x Posisi X dunia.
 * @param y Posisi Y dunia (dasar pot).
 * @param z Posisi Z dunia.
 * @param width Lebar pot (sepanjang sumbu X).
 * @param height Tinggi pot.
 * @param depth Kedalaman pot (sepanjang sumbu Z).
 */
void drawLushPlanterBox(float x, float y, float z, float width, float height, float depth) {
    glPushMatrix();
    glTranslatef(x, y, z);

    // --- 1. Gambar Pot ---
    // Badan pot abu-abu
    setRealisticMaterial(0.4f, 0.4f, 0.42f, 5.0f, 0.1f);
    drawBox(0.0f, (height - height * 0.1f) / 2.0f, 0.0f, width, height - height * 0.1f, depth);

    // Bibir pot putih di atas
    setRealisticMaterial(0.9f, 0.9f, 0.9f, 10.0f, 0.1f);
    drawBox(0.0f, height - (height * 0.1f / 2.0f), 0.0f, width, height * 0.1f, depth);

    // --- 2. Gambar Tanah di Dalam Pot ---
    setRealisticMaterial(0.3f, 0.18f, 0.1f, 1.0f, 0.0f);
    drawBox(0.0f, height * 0.9f, 0.0f, width * 0.95f, 0.1f, depth * 0.95f);

    // --- 3. Gambar Tanaman (Campuran Rumput dan Tanaman Berdaun) ---
    int numPlants = int(width * depth * 0.8f); // Kepadatan tanaman
    srand(x * 100 + z); // Seed acak agar setiap pot unik

    for (int i = 0; i < numPlants; ++i) {
        float plantX = ((rand() % 100) / 100.0f - 0.5f) * width * 0.9f;
        float plantZ = ((rand() % 100) / 100.0f - 0.5f) * depth * 0.9f;

        // Pilih jenis tanaman secara acak
        int plantType = rand() % 5; // Probabilitas 3:2 untuk rumput vs tanaman daun
        if (plantType < 3) {
            drawGrassLump(plantX, height, plantZ);
        } else {
            drawLeafyPlant(plantX, height, plantZ);
        }
    }

    glPopMatrix();
}

// Fungsi untuk menggambar satu bagian pagar
void drawFenceSection(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0.1f, z);

    // Tiang utama pagar yang besar
    setRealisticMaterial(0.8f, 0.8f, 0.8f, 0.0f, 0.2f);
    drawBox(0.0f, 0.75f, 0.0f, 0.8f, 3.5f, 0.8f);

    // Bagian railing (disederhanakan menjadi sebuah balok)
    setRealisticMaterial(0.5f, 0.5f, 0.5f, 0.0f, 0.5f);
    drawBox(2.5f, 0.8f, 0.0f, 4.2f, 3.0f, 0.3f);

    glPopMatrix();
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

void timer(int value) {
    if (isTransitioning) {
        glutPostRedisplay();
    }
    glutTimerFunc(50, timer, 0); // 20 FPS for smooth transition
}
void drawSunAndMoon() {
    glPushMatrix();

    // Posisi interpolasi dari day-night transition
    float sunY = 15.0f - 10.0f * dayNightTransition;  // Turun saat malam
    float sunX = -15.0f + 30.0f * dayNightTransition; // Geser ke kanan saat malam
    float sunZ = 20.0f;

    glTranslatef(sunX, sunY, sunZ);

    // Glow Material
    GLfloat glowAmbient[] = {1.0f, 1.0f, 0.6f, 1.0f};
    GLfloat glowDiffuse[] = {1.0f, 1.0f, 0.6f, 1.0f};
    if (dayNightTransition > 0.5f) {
        // Moon Mode
        glowAmbient[0] = 0.6f; glowAmbient[1] = 0.6f; glowAmbient[2] = 0.9f;
        glowDiffuse[0] = 0.6f; glowDiffuse[1] = 0.6f; glowDiffuse[2] = 0.9f;
    }

    glMaterialfv(GL_FRONT, GL_AMBIENT, glowAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, glowDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, glowAmbient);
    glMaterialf(GL_FRONT, GL_SHININESS, 100.0f);

    // Gambar sphere untuk sun/moon
    glutSolidSphere(1.5f, 20, 16);

    glPopMatrix();
}


void updateDayNightTransition() {
    if (isTransitioning) {
        if (isNightTime) {
            dayNightTransition += transitionSpeed;
            if (dayNightTransition >= 1.0f) {
                dayNightTransition = 1.0f;
                isTransitioning = false;
                std::cout << "Night transition complete" << std::endl;
            }
        } else {
            dayNightTransition -= transitionSpeed;
            if (dayNightTransition <= 0.0f) {
                dayNightTransition = 0.0f;
                isTransitioning = false;
                std::cout << "Day transition complete" << std::endl;
            }
        }
    }
}

// ========== DISPLAY ==========
void display() {

    float item_z_pos = 1.0f;


    updateDayNightTransition();
    updateLighting();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    updateCameraFPS();
    float radYaw = camYaw * M_PI / 180.0f;
    float radPitch = camPitch * M_PI / 180.0f;
    float lookX = camPosX + cos(radPitch) * sin(radYaw);
    float lookY = camPosY + sin(radPitch);
    float lookZ = camPosZ - cos(radPitch) * cos(radYaw);
    gluLookAt(camPosX, camPosY, camPosZ, lookX, lookY, lookZ, 0, 1, 0);

    drawGround();
    drawRoadMarkings();
    //palang trotoar kiri
    drawCustomCylinder(-20.0f, 0.0f, 8.0f-3,0.3f , 2.0f , 32);
    drawCustomCylinder(-20.0f, 0.0f, 10.0f-3,0.3f , 2.0f , 32);
    drawCustomCylinder(-20.0f, 0.0f, 12.0f-3,0.3f , 2.0f , 32);

    //palang trotoar kanan
    drawCustomCylinder(20.0f, 0.0f, 8.0f-3, 0.3f , 2.0f , 32);
    drawCustomCylinder(20.0f, 0.0f, 10.0f-3, 0.3f , 2.0f , 32);
    drawCustomCylinder(20.0f, 0.0f, 12.0f-3, 0.3f , 2.0f , 32);
    //float x, float y, float z, float radius, float height, int segments = 32

    // Pagar di sisi kiri
    for(float x = -50.0f; x <= -18.0f; x += 5.0f) {
        glPushMatrix();
        glTranslatef(x, 0.1f, item_z_pos);
        glRotatef(180.0f, 1, 0, 0); // rotasi 180 derajat di sumbu Y
        glRotatef(180.0f, 0, 0, 1); // rotasi 180 derajat di sumbu Z
        glTranslatef(-x, -0.1f, -item_z_pos);
        drawFenceSection(x, item_z_pos);
        glPopMatrix();
    }
    // Pagar di sisi kanan (diperpanjang)
    // Loop berjalan dari 18.0f hingga 48.0f
    for(float x = 18.0f; x <= 50.0f; x += 5.0f) {
        drawFenceSection(x, item_z_pos);
    }

        // Pot di sisi kiri (di depan pagar)
    drawPlantPot(-22.0f, item_z_pos + 1.5f);
    drawPlantPot(-18.0f, item_z_pos + 1.5f);
    drawPlantPot(-26.0f,item_z_pos + 1.5f);

    // Pot di sisi kanan (di depan pagar)
    drawPlantPot(18.0f, item_z_pos + 1.5f);
    drawPlantPot(22.0f, item_z_pos + 1.5f);
    drawPlantPot(26.0f,item_z_pos + 1.5f);

        drawGardenBush(4.0f, 0.1f, -22.0f);
    drawGardenBush(-5.0f, 0.1f, -23.0f);
    drawGardenBush(2.0f, 0.1f, -24.0f);
    drawGardenBush(4.0f, 0.1f, -20.0f);




    drawMainPillar(-13.0f,180.0f);
    drawMainPillar(13.0f,0.0f);
    drawCenterPillar();
    drawConnectingBeamAt(0.0f, 11.5f, -2.5f); // same default position
    drawConnectingBeamAt(.0f, 11.5f, -3.5f); // same default position
    drawConnectingBeamAt(.0f, 11.5f, -4.5f); // same default position
    drawConnectingBeamAt(.0f, 11.5f, -5.5f); // same default position

    drawBusTerminal(0.0f, 0.0f, -5.0f);

    drawSideWall(20.0f, 4.0f, -4.0f);
    drawSideWallAbove(15.7f, 8.5f, -4.0f);
    drawSideWall(-20.0f, 4.0f, -4.0f);
    drawSideWallAbove(-15.7f, 8.5f, -4.0f);

    //taman belakang
    drawTamanOval(0.0f, -25.0f, 7.0f, 15.0f);
     drawTiangBendera(-0.8f, -12.0f); // Kiri
    drawTiangBendera( 0.0f, -12.0f); // Tengah
    drawTiangBendera( 0.8f, -12.0f); // Kanan
     drawPlantPot(3.0f,item_z_pos -15.5f);
     drawPlantPot(4.0f,item_z_pos -16.5f);
     drawPlantPot(4.5f,item_z_pos -17.5f);

     drawPlantPot(-3.0f,item_z_pos -15.5f);
     drawPlantPot(-4.0f,item_z_pos -16.5f);
     drawPlantPot(-4.5f,item_z_pos -17.5f);

     drawFountainPool(0.0f, 0.1f, -34.0f);  // (x, y, z) -> (pusat, dasar, depan)
    drawUKSWText(0.0f, 0.5f, -43.0f);      // (x, y, z) -> (pusat, dasar, di belakang kolam)

    //greek pillar
    drawGreekPillar(-24.5f, 0.0f, -1.0f); // A pillar on the left
    drawGreekPillar(-22.0f, 0.0f, -1.0f);  // A pillar on the right
    drawGreekPillar(-19.0f, 0.0f, -1.0f);
    drawGreekPillar(19.0f, 0.0f, -1.0f);
    drawGreekPillar(22.0f, 0.0f, -1.0f);
    drawGreekPillar(24.0f, 0.0f, -1.0f);

     drawBankBuilding(-25.0f, 0.0f, -22.0f, 90.0f);

    //pohon gede kiri
    renderCartoonTree3D(-22, 0, -10, 1.0f);
    //pohon gede kanan
    renderCartoonTree3D(22, 0, -10, 1.0f);



    //pot plant
    drawPlanterBox(-40.0f, 0.0f, -4.0f, 8.0f, 2.0f, 2.5f); // x, y, z, lebar, tinggi, kedalaman
    // Pot di sisi kanan gerbang
    drawPlanterBox(40.0f, 0.0f, -4.0f, 8.0f, 2.0f, 2.5f);
    drawPlanterBox(30.0f, 0.0f, -4.0f, 8.0f, 2.0f, 2.5f);
    drawPlanterBox(-30.0f, 0.0f, -4.0f, 8.0f, 2.0f, 2.5f);

    // drawLushPlanterBox(-12.0f, 0.0f, 8.0f, 8.0f, 1.5f, 2.5f); // Pot di kiri
    // drawLushPlanterBox(12.0f, 0.0f, 8.0f, 8.0f, 1.5f, 2.5f);


    //param (x,y,z,scale,RotX,RotY,RotZ)
    drawLamppost(-25.0f, 0.0f, 12.2f, 1.0f, 0.0f, 180.0f, 0.0f);     // Left far (angled)
    drawLamppost(25.0f, 0.0f, 12.2f, 1.0f, 0.0f, 45.0f, 0.0f);     // Right far (angled)
    drawLamppost(12.5f, 0.0f, 12.2f, 1.0f, 0.0f, 180.0f, 0.0f);
    drawLamppost(-12.5f, 0.0f, 12.2f, 1.0f, 0.0f, 180.0f, 0.0f);
    drawLamppost(-36.5f, 0.0f, 12.2f, 1.0f, 0.0f, 180.0f, 0.0f);
    drawLamppost(36.5f, 0.0f, 12.2f, 1.0f, 0.0f, 180.0f, 0.0f);


    drawDecorations();
    glutSwapBuffers();
}

// ========== KONTROL ==========
void keyboard(unsigned char key, int x, int y) {
    keyStates[key] = true;
    switch (key) {
        case 'g': case 'G':
            lampLightOn = !lampLightOn;
            std::cout << "Lamp light " << (lampLightOn ? "ON" : "OFF") << std::endl;
            break;
        case 'n': case 'N':
            if (!isTransitioning) {
                isNightTime = !isNightTime;
                isTransitioning = true;
                std::cout << "Switching to " << (isNightTime ? "NIGHT" : "DAY") << " mode" << std::endl;
            }
            break;
        case 't': case 'T':
            dayNightTransition = isNightTime ? 1.0f : 0.0f;
            isTransitioning = false;
            break;
        case 27: exit(0); // ESC
    }
    glutPostRedisplay();
}

void keyboardUp(unsigned char key, int x, int y) {
    keyStates[key] = false;
}

void specialKey(int key, int x, int y) {
    specialKeyStates[key] = true;
}
void specialKeyUp(int key, int x, int y) {
    specialKeyStates[key] = false;
}



// Mouse FPS style
void mouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        isDragging = true;
        lastMouseX = x;
        lastMouseY = y;
    } else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        isDragging = false;
    }
}

void mouseMotion(int x, int y) {
    if (isDragging) {
        int dx = x - lastMouseX;
        int dy = y - lastMouseY;
        camYaw += dx * 0.3f;
        camPitch -= dy * 0.2f;
        if (camPitch > 89.0f) camPitch = 89.0f;
        if (camPitch < -89.0f) camPitch = -89.0f;
        lastMouseX = x;
        lastMouseY = y;
        glutPostRedisplay();
    }
}




// ========== SETUP ==========
void setup() {
    glClearColor(0.6f, 0.8f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Load texture and check for errors
    logoTexture = loadTexture("texture/logo_uksw.png");
    if (logoTexture == 0) {
        std::cerr << "Failed to load logo texture!" << std::endl;
    } else {
        std::cout << "\n\nLogo texture loaded successfully with ID: " << logoTexture << std::endl;
    }
    pillarTexture = loadTexture("texture/pillar_texture.jpg");
    if(pillarTexture == 0){
        printf("Failed to load pillar texure");
    }
    else{
        printf("\n\nLogo Texture loaded successfully with ID: %d",pillarTexture);
    }

    GLfloat lightPos[] = { 5, 15, 10, 1 };
    GLfloat ambient[] = { 0.4, 0.4, 0.4, 1 };
    GLfloat diffuse[] = { 0.8, 0.8, 0.8, 1 };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

    glMatrixMode(GL_PROJECTION);
    gluPerspective(45.0, 1.333, 1.0, 500.0);
    glMatrixMode(GL_MODELVIEW);
}

// ========== MAIN ==========
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 750);
    glutCreateWindow("BENTENG TAKSESHI UKSW 3D - Final Revisi");

    setupWorkingDirectory();
    debugFolder();

    setup();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKey);
    glutSpecialUpFunc(specialKeyUp);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    glutTimerFunc(16, timer, 0); // ~60 FPS
    glutMainLoop();
    return 0;
}
