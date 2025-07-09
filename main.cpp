
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
    setRealisticMaterial(0.75f, 0.60f, 0.45f, 0.0f,0.3f);
    drawBox(0, 8.0f, -3.0f, 5.0f, 16.0f, 3.5f);

    setRealisticMaterial(0.75f, 0.60f, 0.45f, 0.0f,0.3f);
    drawBox(0, 10.0f, -2.3f, 2.0f, 2.0f, 0.3f);

    setRealisticMaterial(0.75f, 0.60f, 0.45f, 0.0f,0.3f);
    drawBox(0, 16.5f, -3.0f, 2.8f, 1.0f, 3.5f);
    //tempat logo
    setRealisticMaterial(0.9f, 0.88f, 0.85f, 0.0f,0.3f);
    drawBox(0, 14.3f, -1.1f, 3.5f, 3.0f, 0.4f);

    // Parameters: x, y, z, size, rotX, rotY, rotZ, useCircle
     drawLogo(0.0f, 14.3f, -0.89f, 3.0f, 180, 0, 0, true);  // Front face
     renderStrokeTextAtBold("UNIVERSITAS KRISTEN",
                   -2.0f, 12.1f, -1.1f, //posisi x,y,z
                   0, 0, 0,    // Rotasi X, Y, Z
                   0.003f,20);              // Skala teks

     renderStrokeTextAtBold("SATYA WACANA",
                   -2.0f, 11.5f, -1.1f, //posisi x,y,z
                   0, 0, 0,    // Rotasi X, Y, Z
                   0.004f,20);              // Skala teks

}

void drawSideWall(float x, float y, float z) {
    setRealisticMaterial(0.75f, 0.60f, 0.45f, 32.0f,0.3f);

    drawBox(x, y, z, 5.6f, 16.0f, 4.0f); // Ukuran default, bisa kamu ubah sesuai desain
}

void drawSideWallAbove(float x, float y, float z) {
    setRealisticMaterial(0.75f, 0.60f, 0.45f, 32.0f,0.3f);

    drawBox(x, y, z, 4.0f, 7.0f, 4.0f); // Ukuran default, bisa kamu ubah sesuai desain
}


void drawSmallPillar(float x, float z) {
    setRealisticMaterial(0.95f, 0.95f, 0.95f, 32.0f,0.3f);
    drawCylinder(x, 0.0f, z, 0.2f, 11.5f); // lebih tinggi

    glColor3f(0.90f, 0.90f, 0.90f);
    drawBox(x, 11.7f, z, 0.5f, 0.5f, 0.5f);
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

    // Main trunk (twisted)
    glPushMatrix();
    drawTwistedCylinder(4.0f, 2.0f, 15.0f, 20);
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

        // Enhanced light source setup
        glPushMatrix();
        glLoadIdentity();
        glTranslatef(x, y, z);
        if (rotX != 0.0f) glRotatef(rotX, 1, 0, 0);
        if (rotY != 0.0f) glRotatef(rotY, 0, 1, 0);
        if (rotZ != 0.0f) glRotatef(rotZ, 0, 0, 1);
        glScalef(scale, scale, scale);

        GLfloat lampPos[] = {1.6f, 8.5f, 0.0f, 1.0f};
        GLfloat lampAmbient[] = {0.3f * lampIntensity, 0.25f * lampIntensity, 0.15f * lampIntensity, 1.0f};
        GLfloat lampDiffuse[] = {1.0f * lampIntensity, 0.8f * lampIntensity, 0.6f * lampIntensity, 1.0f};
        GLfloat lampSpecular[] = {1.0f, 0.9f, 0.8f, 1.0f};

        glEnable(lampLightID);
        glLightfv(lampLightID, GL_POSITION, lampPos);
        glLightfv(lampLightID, GL_AMBIENT, lampAmbient);
        glLightfv(lampLightID, GL_DIFFUSE, lampDiffuse);
        glLightfv(lampLightID, GL_SPECULAR, lampSpecular);

        // Enhanced attenuation for night effect
        glLightf(lampLightID, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(lampLightID, GL_LINEAR_ATTENUATION, 0.02f);
        glLightf(lampLightID, GL_QUADRATIC_ATTENUATION, 0.005f);

        glPopMatrix();
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
    drawMainPillar(-13.0f,180.0f);
    drawMainPillar(13.0f,0.0f);
    drawCenterPillar();
    drawConnectingBeamAt(0.0f, 11.5f, -2.5f); // same default position
    drawConnectingBeamAt(.0f, 11.5f, -3.5f); // same default position
    drawConnectingBeamAt(.0f, 11.5f, -4.5f); // same default position
    drawConnectingBeamAt(.0f, 11.5f, -5.5f); // same default position


    drawSideWall(20.0f, 4.0f, -4.0f);
    drawSideWallAbove(15.7f, 8.5f, -4.0f);
    drawSideWall(-20.0f, 4.0f, -4.0f);
    drawSideWallAbove(-15.7f, 8.5f, -4.0f);



    // Pilar putih kiri-kanan (3 tiap sisi)
    drawSmallPillar(-22.5f, -1.0f);
    drawSmallPillar(-20.0f, -1.0f);
    drawSmallPillar(-17.5f, -1.0f);
    drawSmallPillar(17.5f, -1.0);
    drawSmallPillar(20.0f, -1.0f);
    drawSmallPillar(22.5f, -1.0f);

    //pohon gede kiri
    renderCartoonTree3D(-22, 0, -10, 1.0f);
    //pohon gede kanan
    renderCartoonTree3D(22, 0, -10, 1.0f);

    //param (x,y,z,scale,RotX,RotY,RotZ)
    drawLamppost(-25.0f, 0.0f, 12.2f, 1.0f, 0.0f, 45.0f, 0.0f);     // Left far (angled)
    drawLamppost(25.0f, 0.0f, 12.2f, 1.0f, 0.0f, -45.0f, 0.0f);     // Right far (angled)
    drawLamppost(12.5f, 0.0f, 12.2f, 1.0f, 0.0f, 180.0f, 0.0f);
    drawLamppost(-12.5f, 0.0f, 12.2f, 1.0f, 0.0f, 180.0f, 0.0f);


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
