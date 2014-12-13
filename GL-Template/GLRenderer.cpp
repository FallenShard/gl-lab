#include "StdAfx.h"
#include "GLRenderer.h"
#include "GL\gl.h"
#include "GL\glu.h"
//#include "GL\glaux.h"
#include "GL\glut.h"
#include "DIB.h"

float rad(float degree)
{
    return degree * 3.141593f / 180;
}

float alphaX = 0.f;
float alphaY = 0.f;

bool polygonMode = true;

// 30, 15, -81
float basePitch = 51.f;
float baseYaw = -30.f;
float armPitch = -90.f;
float headYaw = -50.f;

CGLRenderer::CGLRenderer(void)
{
}

CGLRenderer::~CGLRenderer(void)
{
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

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);


    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthRange(0.0f, 1.f);
    glDepthMask(GL_TRUE);

    glClearColor(0.2f, 0.2f, 0.2f, 1.f);
    glClearDepth(1.f);

    //---------------------------------
    wglMakeCurrent(NULL, NULL);
}

void CGLRenderer::DrawScene(CDC *pDC)
{
    wglMakeCurrent(pDC->m_hDC, m_hrc);
    //---------------------------------
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    gluLookAt(25, 10, 25, 0, 0, 0, 0, 1, 0);

    

    glRotatef(alphaX, 0.f, 1.f, 0.f);
    glRotatef(alphaY, 1.f, 0.f, 0.f);


    if (polygonMode)
    {
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glEnable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
        
    DrawWalls();
    DrawTable();
    DrawLamp();
    
    SwapBuffers(pDC->m_hDC);
    //---------------------------------
    wglMakeCurrent(NULL, NULL);
}

void CGLRenderer::Reshape(CDC *pDC, int w, int h)
{
    wglMakeCurrent(pDC->m_hDC, m_hrc);
    //---------------------------------
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, (double)w / h, 1, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
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

    case 'W':
        basePitch += 3.f;
        break;

    case 'S':
        basePitch -= 3.f;
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

void CGLRenderer::DrawBox(double a, double b, double c)
{
    vec3 array[8];
    array[0].x = +a / 2; array[0].y = +b / 2; array[0].z = +c / 2;
    array[1].x = -a / 2; array[1].y = +b / 2; array[1].z = +c / 2;
    array[2].x = -a / 2; array[2].y = -b / 2; array[2].z = +c / 2;
    array[3].x = +a / 2; array[3].y = -b / 2; array[3].z = +c / 2;
    array[4].x = +a / 2; array[4].y = +b / 2; array[4].z = -c / 2;
    array[5].x = -a / 2; array[5].y = +b / 2; array[5].z = -c / 2;
    array[6].x = -a / 2; array[6].y = -b / 2; array[6].z = -c / 2;
    array[7].x = +a / 2; array[7].y = -b / 2; array[7].z = -c / 2;

    usvec4 indices[6];
    indices[0].a = 0; indices[0].b = 1; indices[0].c = 2; indices[0].d = 3;
    indices[1].a = 4; indices[1].b = 0; indices[1].c = 3; indices[1].d = 7;
    indices[2].a = 5; indices[2].b = 4; indices[2].c = 7; indices[2].d = 6;
    indices[3].a = 1; indices[3].b = 5; indices[3].c = 6; indices[3].d = 2;
    indices[4].a = 4; indices[4].b = 5; indices[4].c = 1; indices[4].d = 0;
    indices[5].a = 3; indices[5].b = 2; indices[5].c = 6; indices[5].d = 7;

    glEnableClientState(GL_VERTEX_ARRAY);

    glVertexPointer(sizeof(vec3) / sizeof(float), GL_FLOAT, 0, array);
    
    glDrawElements(GL_QUADS, 24, GL_UNSIGNED_SHORT, indices);

    glDisableClientState(GL_VERTEX_ARRAY);
}

void CGLRenderer::DrawWall(double a)
{
    glBegin(GL_QUADS);
    glVertex3f(+a / 2, 0.f, -a / 2);
    glVertex3f(-a / 2, 0.f, -a / 2);
    glVertex3f(-a / 2, 0.f, +a / 2);
    glVertex3f(+a / 2, 0.f, +a / 2);
    glEnd();
}

void CGLRenderer::DrawWalls()
{
    glPushMatrix();
    glTranslatef(-20.f, 20.f - 12.4f, 0.f);
    glRotatef(-90, 0.f, 0.f, 1.f);

    glColor3f(0.6f, 0.6f, 0.6f);
    DrawWall(40);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, -12.4f, 0.f);

    glColor3f(0.3f, 0.3f, 0.3f);
    DrawWall(40);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.f, 20.f - 12.4f, -20.f);
    glRotatef(90, 1.f, 0.f, 0.f);

    glColor3f(0.8f, 0.8f, 0.8f);
    DrawWall(40);
    glPopMatrix();
}

void CGLRenderer::DrawTable()
{
    glPushMatrix();
    // Top cover
    glColor3f(0.8f, 0.4f, 0.4f);
    DrawBox(20, 0.2f, 10);
    glPopMatrix();

    // Bulk of the table
    glPushMatrix();
    glTranslatef(0.f, -0.1f - 1.f, 0.f);

    glColor3f(0.4f, 0.2f, 0.2f);
    DrawBox(18, 2.f, 9);
    glPopMatrix();

    // Front-right leg
    glPushMatrix();
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
    glPushMatrix();

    
    glColor3f(1.f, 0.f, 0.f);
    glTranslatef(0.f, 1.f, 0.f);

    // DRAW BASE HERE


    glPopMatrix();


    glPushMatrix();

    // First red arm
    glTranslatef(-4.f, 0.f, 0.f);
    glRotatef(baseYaw, 0.f, 1.f, 0.f);
    glRotatef(basePitch, 1.f, 0.f, 0.f);
    glTranslatef(0.f, 3.f, 0.f);
    
    DrawBox(0.2f, 6.f, 0.2f);

    // Blue thing
    glColor3f(0.f, 0.f, 1.f);
    glTranslatef(0.f, 3.f, 0.f);

    DrawBox(0.3, 0.3f, 0.3f);

    // Second red arm
    glColor3f(1.f, 0.f, 0.f);

    glRotatef(armPitch, 1.f, 0.f, 0.f);
    glTranslatef(0.f, 3.f, 0.f);

    DrawBox(0.2f, 6.f, 0.2f);

    glTranslatef(0.f, 3.f, 0.f);
    glRotatef(-90.f, 0.f, 0.f, 1.f);
    glRotatef(headYaw, 1.f, 0.f, 0.f);
    glColor3f(0.f, 0.f, 1.f);

    glDisable(GL_CULL_FACE);
    DrawLampTop();
    glEnable(GL_CULL_FACE);

    glPopMatrix();
}

void CGLRenderer::DrawLampTop()
{
    DrawBox(1.f, 3.f, 1.f);

    glPushMatrix();

    glScalef(1.f, -1.f, 1.f);
    glTranslatef(0.f, 1.5f, 0.f);
    DrawPyramid(0.5f);

    glPopMatrix();

    glPushMatrix();

    glTranslatef(0.f, 1.5f, 0.f);

        glPushMatrix();
            glTranslatef(0.f, 0.2f, 0.f);
            glScalef(1.f, -1.f, 1.f);

            glColor3f(0.f, 1.f, 1.f);
            DrawRing(0.4, 1, 12, 0.5);

        glPopMatrix();

        glPushMatrix();

            glTranslatef(0.f, 0.4 + 0.3, 0.f);
            glScalef(1.f, -1.f, 1.f);
            DrawRing(0.6, 1.2, 12, 0.2);

        glPopMatrix();

        glPushMatrix();

            glTranslatef(0.f, 1.0f + .3f, 0.f);
            glScalef(1.f, -1.f, 1.f);
            DrawRing(0.6, 1.2, 12);

        glPopMatrix();
    
    glPopMatrix();
}

void CGLRenderer::DrawPyramid(double height)
{
    vec3 array[8];
    array[0].x = 0.f; array[0].y = height; array[0].z = 0.f;
    array[1].x = height; array[1].y = 0.f; array[1].z = height;
    array[2].x = -height; array[2].y = 0.f; array[2].z = height;
    array[3].x = -height; array[3].y = 0.f; array[3].z = -height;
    array[4].x = height; array[4].y = 0.f; array[4].z = -height;
    
    glBegin(GL_TRIANGLE_FAN);
    glVertex3fv((const GLfloat*)&array[0]);
    glVertex3fv((const GLfloat*)&array[1]);
    glVertex3fv((const GLfloat*)&array[2]);
    glVertex3fv((const GLfloat*)&array[3]);
    glVertex3fv((const GLfloat*)&array[4]);
    glEnd();
}

void CGLRenderer::DrawRing(double height, double radius, double steps, double topOffset)
{
    float angleStep = 360 / steps;

    glBegin(GL_QUAD_STRIP);


    float dX = radius;
    float dZ = 0;
    float dXt = radius - topOffset;
    float dZt = 0;

    glVertex3f(dXt, +height / 2, dZt);
    glVertex3f(dX, -height / 2, dZ);


    for (int i = 1; i <= steps; i++)
    {
        double c = cos(rad(i * angleStep));
        double s = sin(rad(i * angleStep));
        dX = radius * c;
        dZ = radius * s;
        dXt = (radius - topOffset) * c;
        dZt = (radius - topOffset) * s;
        
        glVertex3f(dXt, +height / 2, dZt);
        glVertex3f(dX, -height / 2, dZ);
    }

    glEnd();
}
