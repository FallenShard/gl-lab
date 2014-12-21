#include "StdAfx.h"
#include "GLRenderer.h"
#include "GL\gl.h"
#include "GL\glu.h"
//#include "GL\glaux.h"
#include "GL\glut.h"
#include "DIB.h"

#include <cmath>

float rad(float degree)
{
    return degree * 3.141593f / 180;
}

float alphaX = 0.f;     // Scene rotation around X axis
float alphaY = 0.f;     // Scene rotation around Y axis

bool polygonMode = true; // Wireframe controller

// 30, 15, -81
float basePitch = 51.f; // Rotation of the base around X axis
float baseYaw = -30.f;  // Rotation of the base around Y axis
float armPitch = -90.f; // Rotation of the arm around X axis
float headYaw = -50.f; // Rotation of the head around Y axis

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

    glRotatef(alphaX, 0.f, 1.f, 0.f);           // Rotate the scene around X, on key-press
    glRotatef(alphaY, 1.f, 0.f, 0.f);           // Rotate the scene around Y, on key-press

	
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

    glColor3f(1.f, 1.f, 1.f);   // Debug purposes only, draw a white point in 0, 0, 0
    glBegin(GL_POINTS);
    glVertex3f(0.f, 0.f, 0.f);
    glEnd();
        
    DrawWalls();    // Draw the walls
    DrawTable();    // Draw the table
    DrawLamp();     // Draw the lamp
    
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

    glEnableClientState(GL_VERTEX_ARRAY);   // Enable vertex stream

    glVertexPointer(sizeof(vec3) / sizeof(float), GL_FLOAT, 0, array);  // Point to the vertex data
    
    glDrawElements(GL_QUADS, 24, GL_UNSIGNED_SHORT, indices);   // Draw the box

    glDisableClientState(GL_VERTEX_ARRAY);  // Disable vertex stream
}

void CGLRenderer::DrawWall(double a)
{
    glBegin(GL_QUADS);                  // Simple stuff, draw a wall which lays horizontally on y = 0 plane
    glVertex3f(+a / 2, 0.f, -a / 2);
    glVertex3f(-a / 2, 0.f, -a / 2);
    glVertex3f(-a / 2, 0.f, +a / 2);
    glVertex3f(+a / 2, 0.f, +a / 2);
    glEnd();
}

void CGLRenderer::DrawWalls()
{
	glDisable(GL_CULL_FACE);
    glPushMatrix();                             // Save the current matrix
        glTranslatef(-20.f, 50.f - 12.4f, 30.f); // Rotate, then translate the wall ("left" wall)
        glRotatef(-90, 0.f, 0.f, 1.f);

        glColor3f(0.6f, 0.6f, 0.6f);            // Colorize and draw
        DrawWall(100);
    glPopMatrix();                              // Restore previous matrix state

    glPushMatrix();
        glTranslatef(30, -12.4f, 30.f);           // This wall is actually the floor

        glColor3f(0.3f, 0.3f, 0.3f);
        DrawWall(100);
    glPopMatrix();

    glPushMatrix();
        glTranslatef(30.f, 50.f - 12.4f, -20.f); // This wall is the "back" wall
        glRotatef(90, 1.f, 0.f, 0.f);

        glColor3f(0.8f, 0.8f, 0.8f);
        DrawWall(100);
    glPopMatrix();

	glEnable(GL_CULL_FACE);
}

void CGLRenderer::DrawTable()
{
    glPushMatrix();                         // Save the current matrix state
    // Top cover
        glColor3f(0.8f, 0.4f, 0.4f);        // Draw the thin top cover (0, 0, 0) point is practically inside this element
        DrawBox(20, 0.2f, 10);
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
    glPushMatrix();                     // This one's for fun, the teapot
        glTranslatef(5.f, 2.f, 0.f); 

        glColor3f(1.f, 0.3f, 0.f);    
        glutSolidTeapot(2.f);        
    glPopMatrix();

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
        DrawBox(0.3, 0.3f, 0.3f);

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

    // Now we want to save the matrix, we'll be drawing to two different sides
    glPushMatrix();
        // Draw the pyramid pointy end (this pyramid has no floor)
        glTranslatef(0.f, -1.5f, 0.f);
        glScalef(1.f, -1.f, 1.f);

        DrawPyramid(0.5f);
    glPopMatrix();

    //  We restored the state up there, so we're back in the center of blue box part
    glPushMatrix();
    // Move to the other end of the blue box part
        glTranslatef(0.f, 1.5f, 0.f);
        // Okay, we gotta save that end

			glPushMatrix();
			glColor3f(1.f, 0.5f, .5f);
			glEnable(GL_CLIP_PLANE0);
			
			
			glTranslatef(0.f, 1.f, 0.f);
			glRotatef(180, 1.f, 0.f, 0.f);
			glColor3f(1.f, 0.f, 0.f);       // Red color

			GLdouble cp[4] = { 0.f, 2.f, 0.f, 0.f };
			glClipPlane(GL_CLIP_PLANE0, cp);

			glutSolidSphere(1.3f, 12, 12);

			glColor3f(1.f, 1.f, 0.f);
			glCullFace(GL_FRONT);    // Face to be culled is back-face
			glutSolidSphere(1.3f, 12, 12);
			glCullFace(GL_BACK);    // Face to be culled is back-face

			glDisable(GL_CLIP_PLANE0);
			glPopMatrix();

            //glPushMatrix();
            //    glTranslatef(0.f, 0.2f, 0.f);   // Draw the first inset ring
            //    glScalef(1.f, -1.f, 1.f);

            //    glColor3f(0.f, 0.f, 1.f);       // Blue color
            //    // Since the next part includes drawing both sides, we need to disable culling
            //    glDisable(GL_CULL_FACE);
            //    DrawRing(0.4, 1, 12, 0.5);

            //glPopMatrix();  // Restore the matrix back to the end of blue box part

            //glPushMatrix(); // Draw the next ring
            //    glTranslatef(0.f, 0.4 + 0.3, 0.f);
            //    glScalef(1.f, -1.f, 1.f);

            //    DrawRing(0.6, 1.2, 12, 0.2);
            //glPopMatrix();

            //glPushMatrix();
            //    glTranslatef(0.f, 1.0f + .3f, 0.f);
            //    glScalef(1.f, -1.f, 1.f);

            //    DrawRing(0.6, 1.2, 12);
            //    glEnable(GL_CULL_FACE);
            //glPopMatrix();
    glPopMatrix();
}

void CGLRenderer::DrawPyramid(double height)
{
    vec3 array[5];
    array[0].x = 0.f; array[0].y = height; array[0].z = 0.f;
    array[1].x = height; array[1].y = 0.f; array[1].z = -height;
    array[2].x = height; array[2].y = 0.f; array[2].z = height;
    array[3].x = -height; array[3].y = 0.f; array[3].z = height;
    array[4].x = -height; array[4].y = 0.f; array[4].z = -height;
    
    
    glDisable(GL_CULL_FACE);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3fv((const GLfloat*)&array[0]);
    glVertex3fv((const GLfloat*)&array[1]);
    glVertex3fv((const GLfloat*)&array[2]);
    glVertex3fv((const GLfloat*)&array[3]);
    glVertex3fv((const GLfloat*)&array[4]);
    glEnd();
    glEnable(GL_CULL_FACE);
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
