#pragma once

#include "vec_types.h"

class CGLRenderer
{
public:
    CGLRenderer(void);
    virtual ~CGLRenderer(void);
        
    bool CreateGLContext(CDC* pDC);			// kreira OpenGL Rendering Context
    void PrepareScene(CDC* pDC);			// inicijalizuje scenu,
    void Reshape(CDC* pDC, int w, int h);	// kod koji treba da se izvrsi svaki put kada se promeni velicina prozora ili pogleda i
    void DrawScene(CDC* pDC);				// iscrtava scenu
    void DestroyScene(CDC* pDC);			// dealocira resurse alocirane u drugim funkcijama ove klase,

    bool onKeyDown(UINT nChar);



protected:
    HGLRC	 m_hrc; //OpenGL Rendering Context 

    void DrawBox(double a, double b, double c);
    void DrawTable();
    void DrawWall(double a);
    void DrawWalls();
    void DrawLamp();
    void DrawLampTop();
    void DrawPyramid(double height);
    void DrawRing(double height, double radius, double steps, double topOffset = 0.f);


    
};
