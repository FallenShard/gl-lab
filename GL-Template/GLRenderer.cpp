#include "StdAfx.h"
#include "GLRenderer.h"
#include "GL\gl.h"
#include "GL\glu.h"
//#include "GL\glaux.h"
#include "GL\glut.h"
#include "DIB.h"

#include <cmath>

#include <vector>

#include "DImage.h"

namespace
{
    float rad(float degree)
    {
        return degree * 3.141593f / 180;
    }

    float alphaX = 60.f;     // Scene rotation around X axis
    float alphaY = 0.f;     // Scene rotation around Y axis

    bool polygonMode = true; // Wireframe controller
    bool useLightBulb = false;

    // 30, 15, -81
    float basePitch = 51.f; // Rotation of the base around X axis
    float baseYaw = -30.f;  // Rotation of the base around Y axis
    float armPitch = -90.f; // Rotation of the arm around X axis
    float headYaw = -50.f; // Rotation of the head around Y axis

    const int tessFactor = 200;
    vec3 wall[tessFactor + 1][tessFactor + 1];


    bool freeLook = false;

    const float lookSpeed = 100.f;

    float texelDensity = 8.f;


    GLuint texId[3];

    enum TexId {
        Wood,
        Wall,
        Floor
    };

    DImage* g_textures = nullptr;
}


CGLRenderer::CGLRenderer(void)
{
    g_textures = new DImage[3];

    g_textures[Wood].Load(CString("ASHSEN512.bmp"));
    g_textures[Wall].Load(CString("Wall512.bmp"));
    g_textures[Floor].Load(CString("PAT39.bmp"));
}

CGLRenderer::~CGLRenderer(void)
{
    delete[] g_textures;
}

bool CGLRenderer::CreateGLContext(CDC* pDC)
{
    PIXELFORMATDESCRIPTOR pfd ;
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize  = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion   = 1; 
    pfd.dwFlags    = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;   
    pfd.iPixelType = PFD_TYPE_RGBA; 
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24; 
    pfd.iLayerType = PFD_MAIN_PLANE;
    
    int nPixelFormat = ChoosePixelFormat(pDC->m_hDC, &pfd);
    
    if (nPixelFormat == 0) return false; 

    BOOL bResult = SetPixelFormat (pDC->m_hDC, nPixelFormat, &pfd);
    
    if (!bResult) return false; 

    m_hrc = wglCreateContext(pDC->m_hDC); 

    if (!m_hrc) return false; 

    return true;	
}

void CGLRenderer::PrepareScene(CDC *pDC)
{
    wglMakeCurrent(pDC->m_hDC, m_hrc);
    //---------------------------------

    // Enable back face culling
    glEnable(GL_CULL_FACE); // Enable face culling
    glCullFace(GL_BACK);    // Face to be culled is back-face
    glFrontFace(GL_CCW);    // Front faces are wound in CCW manner

    glEnable(GL_DEPTH_TEST);    // Enable depth drawing
    glDepthFunc(GL_LEQUAL);     // Depth function is <=, that is draw over the buffer if incoming pixel (fragment) is less than or equal to the buffer
    glDepthRange(0.0f, 1.f);    // Normalized depth range
    glDepthMask(GL_TRUE);       // Enables writing into the depth buffer

    glClearColor(0.2f, 0.2f, 0.2f, 1.f);    // Default window color, dark gray
    glClearDepth(1.f);  // Default depth - 1.f (highest)
    glPointSize(3.f);   // Debug purposes only

    CreateWall(100);

    PrepareTextures();

    SetLightModel();
    SetBulbLight();

    //---------------------------------
    wglMakeCurrent(NULL, NULL);
}

void CGLRenderer::DrawScene(CDC *pDC)
{
    wglMakeCurrent(pDC->m_hDC, m_hrc);
    //---------------------------------
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color (wipe the screen) and depth buffer (reset depth)

    glLoadIdentity();                           // Load identity into MODELVIEW matrix
    gluLookAt(25, 10, 25, 0, 0, 0, 0, 1, 0);    // Set up viewer, target and up

    glRotatef(alphaY, 1.f, 0.f, 0.f);           // Rotate the scene around X, on key-press
    glRotatef(alphaX, 0.f, 1.f, 0.f);           // Rotate the scene around Y, on key-press

    if (useLightBulb)
        glEnable(GL_LIGHT1);
    else
        glDisable(GL_LIGHT1);

    
    
    if (polygonMode)            // Debug purposes only, activate/deactivate on space
    {
        glDisable(GL_CULL_FACE);                    // We want everything drawn, disable culling
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // Draw everything as lines
    }
    else
    {
        glEnable(GL_CULL_FACE); // Normal drawing mode, use culling
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // Draw everything normally, as filled polygons
    }
        
    DrawWalls();    // Draw the walls
    DrawTable();    // Draw the table
    DrawLamp();     // Draw the lamp

    //glBindTexture(GL_TEXTURE_2D, texId[Wood]);
    //glBegin(GL_QUADS);
    //glColor4f(1.f, 1.f, 1.f, 1.f);

    //glTexCoord2i(0.f, 0.f);
    //glVertex3f(-0.5, -0.5f, -5.f);

    //glTexCoord2i(1.f, 0.f);
    //glVertex3f(+0.5, -0.5f, -5.f);

    //glTexCoord2i(1.f, 1.f);
    //glVertex3f(+0.5, +0.5f, -5.f);

    //glTexCoord2i(0.f, 1.f);
    //glVertex3f(-0.5, +0.5f, -5.f);
    //glEnd();
    
    SwapBuffers(pDC->m_hDC);    // Swap the back buffer onto the screen
    //---------------------------------
    wglMakeCurrent(NULL, NULL);
}

void CGLRenderer::Reshape(CDC *pDC, int w, int h)
{
    wglMakeCurrent(pDC->m_hDC, m_hrc);
    //---------------------------------
    glViewport(0, 0, w, h);         // If reshaped, we want to draw to the whole screen again, set viewport
    glMatrixMode(GL_PROJECTION);    // Alter the projection matrix with new width/height
    glLoadIdentity();               // Load identity first
    gluPerspective(45, (double)w / h, 0.1, 150);  // Set the perspective matrix
    glMatrixMode(GL_MODELVIEW);     // Go to MODELVIEW mode and alter it during drawing..
    //---------------------------------
    wglMakeCurrent(NULL, NULL);
}

void CGLRenderer::DestroyScene(CDC *pDC)
{
    wglMakeCurrent(pDC->m_hDC, m_hrc);
    // ... 
    wglMakeCurrent(NULL,NULL); 
    if(m_hrc) 
    {
        wglDeleteContext(m_hrc);
        m_hrc = NULL;
    }
}

bool CGLRenderer::onKeyDown(UINT nChar)
{
    bool handled = false;

    switch (nChar)
    {
    case VK_LEFT:
        alphaX -= 10.f;
        break;

    case VK_RIGHT:
        alphaX += 10.f;
        break;

    case VK_UP:
        alphaY += 10.f;
        break;

    case VK_DOWN:
        alphaY -= 10.f;
        break;

    case VK_SPACE:
        polygonMode = !polygonMode;
        break;
        
    case VK_RETURN:
        useLightBulb = !useLightBulb;
        break;

    case 'W':
        basePitch += 3.f;
        if (basePitch > 80.f) basePitch = 80.f;
        break;

    case 'S':
        basePitch -= 3.f;
        if (basePitch < -80.f) basePitch = -80.f;
        break;

    case 'A':
        baseYaw += 3.f;
        break;

    case 'D':
        baseYaw -= 3.f;
        break;

    case 'Q':
        armPitch += 3.f;
        break;

    case 'E':
        armPitch -= 3.f;
        break;

    case 'C':
        headYaw += 3.f;
        break;

    case 'F':
        headYaw -= 3.f;
        break;
    };


    handled = true;

    return handled;
}


// Draws a box using indexed quad list
void CGLRenderer::DrawBox(double a, double b, double c)
{
    vec3 array[24];
    //Front 
    array[0].x = +a / 2; array[0].y = +b / 2; array[0].z = +c / 2;
    array[1].x = -a / 2; array[1].y = +b / 2; array[1].z = +c / 2;
    array[2].x = -a / 2; array[2].y = -b / 2; array[2].z = +c / 2;
    array[3].x = +a / 2; array[3].y = -b / 2; array[3].z = +c / 2;

    //Right
    array[4].x = +a / 2; array[4].y = +b / 2; array[4].z = -c / 2;
    array[5].x = +a / 2; array[5].y = +b / 2; array[5].z = +c / 2;
    array[6].x = +a / 2; array[6].y = -b / 2; array[6].z = +c / 2;
    array[7].x = +a / 2; array[7].y = -b / 2; array[7].z = -c / 2;

    //Back
    array[8].x  = -a / 2; array[8].y  = +b / 2; array[8].z  = -c / 2;
    array[9].x  = +a / 2; array[9].y  = +b / 2; array[9].z  = -c / 2;
    array[10].x = +a / 2; array[10].y = -b / 2; array[10].z = -c / 2;
    array[11].x = -a / 2; array[11].y = -b / 2; array[11].z = -c / 2;

    //Left
    array[12].x = -a / 2; array[12].y = +b / 2; array[12].z = +c / 2;
    array[13].x = -a / 2; array[13].y = +b / 2; array[13].z = -c / 2;
    array[14].x = -a / 2; array[14].y = -b / 2; array[14].z = -c / 2;
    array[15].x = -a / 2; array[15].y = -b / 2; array[15].z = +c / 2;

    //Top
    array[16].x = +a / 2; array[16].y = +b / 2; array[16].z = -c / 2;
    array[17].x = -a / 2; array[17].y = +b / 2; array[17].z = -c / 2;
    array[18].x = -a / 2; array[18].y = +b / 2; array[18].z = +c / 2;
    array[19].x = +a / 2; array[19].y = +b / 2; array[19].z = +c / 2;

    //Bottom
    array[20].x = +a / 2; array[20].y = -b / 2; array[20].z = +c / 2;
    array[21].x = -a / 2; array[21].y = -b / 2; array[21].z = +c / 2;
    array[22].x = -a / 2; array[22].y = -b / 2; array[22].z = -c / 2;
    array[23].x = +a / 2; array[23].y = -b / 2; array[23].z = -c / 2;

    vec3 normals[24];
    //Front
    normals[0].x = 0.f; normals[0].y = 0.f; normals[0].z = +1.f;
    normals[1].x = 0.f; normals[1].y = 0.f; normals[1].z = +1.f;
    normals[2].x = 0.f; normals[2].y = 0.f; normals[2].z = +1.f;
    normals[3].x = 0.f; normals[3].y = 0.f; normals[3].z = +1.f;

    //Right
    normals[4].x = 1.f; normals[4].y = 0.f; normals[4].z = 0.f;
    normals[5].x = 1.f; normals[5].y = 0.f; normals[5].z = 0.f;
    normals[6].x = 1.f; normals[6].y = 0.f; normals[6].z = 0.f;
    normals[7].x = 1.f; normals[7].y = 0.f; normals[7].z = 0.f;

    //Back
    normals[8].x  = 0.f; normals[8].y  = 0.f; normals[8].z  = -1.f;
    normals[9].x  = 0.f; normals[9].y  = 0.f; normals[9].z  = -1.f;
    normals[10].x = 0.f; normals[10].y = 0.f; normals[10].z = -1.f;
    normals[11].x = 0.f; normals[11].y = 0.f; normals[11].z = -1.f;

    //Left
    normals[12].x = -1.f; normals[12].y = 0.f; normals[12].z = 0.f;
    normals[13].x = -1.f; normals[13].y = 0.f; normals[13].z = 0.f;
    normals[14].x = -1.f; normals[14].y = 0.f; normals[14].z = 0.f;
    normals[15].x = -1.f; normals[15].y = 0.f; normals[15].z = 0.f;

    //Top
    normals[16].x = 0.f; normals[16].y = 1.f; normals[16].z = 0.f;
    normals[17].x = 0.f; normals[17].y = 1.f; normals[17].z = 0.f;
    normals[18].x = 0.f; normals[18].y = 1.f; normals[18].z = 0.f;
    normals[19].x = 0.f; normals[19].y = 1.f; normals[19].z = 0.f;

    //Bottom
    normals[20].x = 0.f; normals[20].y = -1.f; normals[20].z = 0.f;
    normals[21].x = 0.f; normals[21].y = -1.f; normals[21].z = 0.f;
    normals[22].x = 0.f; normals[22].y = -1.f; normals[22].z = 0.f;
    normals[23].x = 0.f; normals[23].y = -1.f; normals[23].z = 0.f;

    usvec4 indices[6];

    for (int i = 0; i < 6; i++)
    {
        indices[i].a = i * 4 + 0;
        indices[i].b = i * 4 + 1;
        indices[i].c = i * 4 + 2;
        indices[i].d = i * 4 + 3;
    }

    
    glBegin(GL_QUADS);
    for (int i = 0; i < 6; i++)
    {
        glNormal3fv((const GLfloat*)&normals[indices[i].a]);
        glVertex3fv((const GLfloat*)&array[indices[i].a]);
        glNormal3fv((const GLfloat*)&normals[indices[i].b]);
        glVertex3fv((const GLfloat*)&array[indices[i].b]);
        glNormal3fv((const GLfloat*)&normals[indices[i].c]);
        glVertex3fv((const GLfloat*)&array[indices[i].c]);
        glNormal3fv((const GLfloat*)&normals[indices[i].d]);
        glVertex3fv((const GLfloat*)&array[indices[i].d]);
    }
    glEnd();
}

void CGLRenderer::DrawTessQuad(double a, double b, int factor)
{
    double aStep = a / factor;
    double bStep = b / factor;
    double aHalf = a / 2.f;
    double bHalf = b / 2.f;

    std::vector<vec3> quad;

    for (int i = 0; i < factor + 1; i++)
    {
        for (int j = 0; j < factor + 1; j++)
        {
            quad.push_back(vec3(j * aStep - aHalf, 0.f, i * bStep - bHalf));
        }
    }
        
    vec3 normal(0.f, 1.f, 0.f);
    glBegin(GL_QUADS);
    for (int i = 0; i < factor; i++)
        for (int j = 0; j < factor; j++)
        {
            float texS = float(j + 1) / (factor + 1);
            float texT = 1.f - float(i) / (factor + 1);
            glTexCoord2f(texS, texT);
            glNormal3fv((const GLfloat*)&normal);
            glVertex3fv((const GLfloat*)&quad[i * (factor + 1) + j + 1]);

            texS = float(j) / (factor + 1);
            texT = 1.f - float(i) / (factor + 1);
            glTexCoord2f(texS, texT);
            glNormal3fv((const GLfloat*)&normal);
            glVertex3fv((const GLfloat*)&quad[i * (factor + 1) + j]);

            texS = float(j) / (factor + 1);
            texT = 1.f - float(i + 1) / (factor + 1);
            glTexCoord2f(texS, texT);
            glNormal3fv((const GLfloat*)&normal);
            glVertex3fv((const GLfloat*)&quad[(i + 1) * (factor + 1) + j]);

            texS = float(j + 1) / (factor + 1);
            texT = 1.f - float(i + 1) / (factor + 1);
            glTexCoord2f(texS, texT);
            glNormal3fv((const GLfloat*)&normal);
            glVertex3fv((const GLfloat*)&quad[(i + 1) * (factor + 1) + j + 1]);
        }
    glEnd();
}

void CGLRenderer::CreateWall(double a)
{
    double step = a / tessFactor;

    for (int i = 0; i < tessFactor + 1; i++)
        for (int j = 0; j < tessFactor + 1; j++)
        {
            wall[i][j].x = j * step - a / 2.0;
            wall[i][j].y = 0.f;
            wall[i][j].z = i * step - a / 2.0;
        }
}

void CGLRenderer::DrawWall(double a)
{
    const GLfloat normal[3] = { 0.f, 1.f, 0.f };
    glBegin(GL_QUADS);
    for (int i = 0; i < tessFactor; i++)
        for (int j = 0; j < tessFactor; j++)
        {
            float texS = float(j + 1) / tessFactor   * texelDensity;
            float texT = 1.f - float(i) / tessFactor * texelDensity;
            glTexCoord2f(texS, texT);
            glNormal3fv(normal);
            glVertex3fv((const GLfloat*)&wall[i][j + 1]);

            texS = float(j) / tessFactor       * texelDensity;
            texT = 1.f - float(i) / tessFactor * texelDensity;
            glTexCoord2f(texS, texT);
            glNormal3fv(normal);
            glVertex3fv((const GLfloat*)&wall[i][j]);

            texS = float(j) / tessFactor          * texelDensity;
            texT = 1.f - float(i + 1) / tessFactor * texelDensity;
            glTexCoord2f(texS, texT);
            glNormal3fv(normal);
            glVertex3fv((const GLfloat*)&wall[i + 1][j]);

            texS = float(j + 1) / tessFactor       * texelDensity;
            texT = 1.f - float(i + 1) / tessFactor * texelDensity;
            glTexCoord2f(texS, texT);
            glNormal3fv(normal);
            glVertex3fv((const GLfloat*)&wall[i + 1][j + 1]);
        }
    glEnd();
}

void CGLRenderer::DrawWalls()
{
    SetWallMaterial();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texId[Wall]);
    glDisable(GL_CULL_FACE);
    glPushMatrix();                             // Save the current matrix
        glTranslatef(-20.f, 50.f - 12.4f, 30.f); // Rotate, then translate the wall ("left" wall)
        glRotatef(-90, 0.f, 0.f, 1.f);

        glColor3f(0.6f, 0.6f, 0.6f);            // Colorize and draw
        DrawWall(100);
    glPopMatrix();                              // Restore previous matrix state

    glBindTexture(GL_TEXTURE_2D, texId[Floor]);
    texelDensity = 24.f;
    glPushMatrix();
        glTranslatef(30, -12.4f, 30.f);         // This wall is actually the floor

        glColor3f(0.3f, 0.3f, 0.3f);
        DrawWall(100);
    glPopMatrix();
    texelDensity = 8.f;

    glBindTexture(GL_TEXTURE_2D, texId[Wall]);
    glPushMatrix();
        glTranslatef(30.f, 50.f - 12.4f, -20.f); // This wall is the "back" wall
        glRotatef(90, 1.f, 0.f, 0.f);

        glColor3f(0.8f, 0.8f, 0.8f);
        DrawWall(100);
    glPopMatrix();
    glEnable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
}

void CGLRenderer::DrawTable()
{
    SetWoodMaterial();
    glPushMatrix();                         // Save the current matrix state
    // Top cover
        glColor3f(0.8f, 0.4f, 0.4f);        // Draw the thin top cover (0, 0, 0) point is practically inside this element
        DrawBox(20, 0.2f, 10);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texId[Wood]);
        glTranslatef(0.f, 0.11f, 0.f);
        DrawTessQuad(20, 10, 50);
        glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    // Bulk of the table
    glPushMatrix();                         // Draw the fat part of the table
        glTranslatef(0.f, -0.1f - 1.f, 0.f);

        glColor3f(0.4f, 0.2f, 0.2f);
        DrawBox(18, 2.f, 9);
    glPopMatrix();

    // Front-right leg
    glPushMatrix();                         // Draw the legs
        glTranslatef(9.2f, -6.2f, 4.7f);
    
        glColor3f(0.f, 1.f, 0.f);
        DrawBox(0.4f, 12.f, 0.4f);
    glPopMatrix();

    // Back-right leg
    glPushMatrix();
        glTranslatef(9.2f, -6.2f, -4.7f);

        glColor3f(0.f, 1.f, 0.f);
        DrawBox(0.4f, 12.f, 0.4f);
    glPopMatrix();

    // Back-left leg
    glPushMatrix();
        glTranslatef(-9.2f, -6.2f, -4.7f);

        glColor3f(0.f, 1.f, 0.f);
        DrawBox(0.4f, 12.f, 0.4f);
    glPopMatrix();

    // Front-left leg
    glPushMatrix();
        glTranslatef(-9.2f, -6.2f, 4.7f);

        glColor3f(0.f, 1.f, 0.f);
        DrawBox(0.4f, 12.f, 0.4f);
    glPopMatrix();
}

void CGLRenderer::DrawLamp()
{
    SetPorcelainMaterial();
    glPushMatrix();                     // This one's for fun, the teapot
        glTranslatef(5.f, 2.f, 0.f); 

        glColor3f(1.f, 0.3f, 0.f);    
        glFrontFace(GL_CW);
        glutSolidTeapot(2.f);   
        glFrontFace(GL_CCW);
    glPopMatrix();

    SetRedMaterial();
    glPushMatrix();                     // Save the state
        glTranslatef(-4.f, 0.f, 0.f);   // Position the spherical base

        glEnable(GL_CLIP_PLANE0);
        GLdouble cp[4] = { 0.f, 1.f, 0.f, 0.f };
        glClipPlane(GL_CLIP_PLANE0, cp);

        glColor3f(1.f, 0.f, 0.f);       // Red color
        glutSolidSphere(1.f, 12, 12);

        glDisable(GL_CLIP_PLANE0);

        // First red arm
        glRotatef(baseYaw, 0.f, 1.f, 0.f);      // Rotate around Y
        glRotatef(basePitch, 1.f, 0.f, 0.f);    // Rotate around X
        glTranslatef(0.f, 3.f, 0.f);            // Translate the object by height first
    
        DrawBox(0.2f, 6.f, 0.2f);

        // Now we do not restore matrix state, we want to continue building on this one
        // Blue joint thing
        glTranslatef(0.f, 3.f, 0.f);

        glColor3f(0.f, 0.f, 1.f);
        glutSolidSphere(0.2, 16, 16);
        //DrawBox(0.3, 0.3f, 0.3f);

        // Second red arm, continue onto the joint translation, no matrix restoration
        glRotatef(armPitch, 1.f, 0.f, 0.f);     // Rotate around X axis to set the pitch
        glTranslatef(0.f, 3.f, 0.f);            // Translate the composite matrix

        glColor3f(1.f, 0.f, 0.f);
        DrawBox(0.2f, 6.f, 0.2f);

        // Continuing on, we'll draw the lamp head
        glTranslatef(0.f, 3.f, 0.f);            // Position correctly along Y
        glRotatef(-90.f, 0.f, 0.f, 1.f);        // Rotate by -90 around Z
        glRotatef(headYaw, 1.f, 0.f, 0.f);      // Rotate by yaw around X
        
        glColor3f(0.f, 0.f, 1.f);               // Colorize into blue
        DrawLampTop();
    glPopMatrix(); // This restores the matrix that was pushed when we started drawing the base
}

void CGLRenderer::DrawLampTop()
{
    // We know the matrix state, it's set up, so just draw the blue box part of the top
    DrawBox(1.f, 3.f, 1.f);

    //  We restored the state up there, so we're back in the center of blue box part
    glPushMatrix();
    // Move to the other end of the blue box part
        glTranslatef(0.f, 1.5f, 0.f);
        // Okay, we gotta save that end
            glPushMatrix();
            glColor3f(1.f, 0.5f, .5f);
            
            glTranslatef(0.f, 1.f, 0.f);
            glRotatef(180, 1.f, 0.f, 0.f);

            GLdouble cp[4] = { 0.f, 0.4f, 0.f, 0.25f };
            glClipPlane(GL_CLIP_PLANE1, cp);
            glDisable(GL_CULL_FACE);
            glEnable(GL_CLIP_PLANE1);
            glutSolidSphere(1.3f, 32, 32);
            glDisable(GL_CLIP_PLANE1);
            glEnable(GL_CULL_FACE);

            SetBulbMaterial();
            glutSolidSphere(0.5f, 16, 16);
            
            SetBulbLight();

            glPopMatrix();
    glPopMatrix();
}

void CGLRenderer::SetLightModel()
{
    const GLfloat ambient[4] =  { 0.2f, 0.2f, 0.2f, 1.f };
    const GLfloat diffuse[4] = { 1.f, 1.f, 1.f, 1.f };
    const GLfloat specular[4] = { 1.f, 1.f, 1.f, 1.f };
    const GLfloat position[4] = { 0.5f, 1.f, 0.f, 0.f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.02);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.002);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.001);
    glEnable(GL_LIGHT0);
    
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

    glEnable(GL_LIGHTING);
}

void CGLRenderer::SetBulbLight()
{
    GLfloat pos[] = { 0.f, 0.f, 0.f, 1.f };
    const GLfloat ambient[4] = { 0.2f, 0.2f, 0.2f, 1.f };
    const GLfloat diffuse[4] = { 1.f, 1.f, 0.7f, 1.f };
    const GLfloat specular[4] = { 1.f, 1.f, 0.7f, 1.f };
    GLfloat dir[] = { 0.f, -1.f, 0.f };
    glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT1, GL_POSITION, pos);
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.2);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.02);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.002);
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 45.0);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, dir);
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 1.0);
}

void CGLRenderer::SetRedMaterial()
{
    GLfloat ambient[] = { 1.0f, 0.f, 0.f, 1.f };
    GLfloat diffuse[] = { 1.f, 0.f, 0.f, 1.f };
    GLfloat specular[] = { 1.f, 0.7f, 0.7f, 1.f };
    GLfloat default[] = { 0.f, 0.f, 0.f, 1.f };
    GLfloat shininess = 64;
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT, GL_EMISSION, default);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}

void CGLRenderer::SetWallMaterial()
{
    GLfloat ambient[] = { 0.4f, 0.35f, 0.3f, 1.f };
    GLfloat diffuse[] = { 0.2f, 0.1f, 0.0f, 1.f };
    GLfloat default[] = { 0.f, 0.f, 0.f, 1.f };
    GLfloat shininess = 0;
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, default);
    glMaterialfv(GL_FRONT, GL_EMISSION, default);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}
void CGLRenderer::SetBulbMaterial()
{
    GLfloat ambient[] = { 1.0f, 1.f, 0, 1.f };
    GLfloat diffuse[] = { 1.f, 1.f, 0, 1.f };
    GLfloat specular[] = { 1.f, 1.f, 0, 1.f };
    GLfloat emissive[] = { 1.f, 1.f, 0.7, 1.f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT, GL_EMISSION, emissive);
    glMaterialf(GL_FRONT, GL_SHININESS, 0);
}
void CGLRenderer::SetWoodMaterial()
{
    GLfloat ambient[] = { 0.4f, 0.2f, 0.f, 1.f };
    GLfloat diffuse[] = { 0.4f, 0.2f, 0.f, 1.f };
    GLfloat specular[] = { 0.2f, 0.2f, 0.2f, 1.f };
    GLfloat emissive[] = { 0.f, 0.f, 0.f, 1.f };
    GLfloat shininess = 16;
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT, GL_EMISSION, emissive);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}

void CGLRenderer::SetPorcelainMaterial()
{
    GLfloat ambient[] = { 1.0f, 0.6f, 0.6f, 1.f };
    GLfloat diffuse[] = { 1.f, 0.8f, 0.8f, 1.f };
    GLfloat specular[] = { 1.f, 1.f, 1.f, 1.f };
    GLfloat default[] = { 0.f, 0.f, 0.f, 1.f };
    GLfloat shininess = 128;
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT, GL_EMISSION, default);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}

void CGLRenderer::PrepareTextures()
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glGenTextures(3, texId);

    GLenum envMode = GL_MODULATE;

    glBindTexture(GL_TEXTURE_2D, texId[Wall]);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, envMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, g_textures[Wall].Width(), g_textures[Wall].Height(),
        0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, g_textures[Wall].GetDIBBits());

    glBindTexture(GL_TEXTURE_2D, texId[Wood]);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, envMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, g_textures[Wood].Width(), g_textures[Wood].Height(),
        0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, g_textures[Wood].GetDIBBits());

    glBindTexture(GL_TEXTURE_2D, texId[Floor]);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, envMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, g_textures[Floor].Width(), g_textures[Floor].Height(),
        0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, g_textures[Floor].GetDIBBits());

    glBindTexture(GL_TEXTURE_2D, 0);
}