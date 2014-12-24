#pragma once;

struct vec3
{
    float x;
    float y;
    float z;

    vec3(float x, float y, float z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    vec3() {}
};

typedef struct
{
    float x;
    float y;
    float z;
    float w;
} vec4;

typedef struct
{
    unsigned short a;
    unsigned short b;
    unsigned short c;
    unsigned short d;
} usvec4;

typedef struct
{
    unsigned short a;
    unsigned short b;
    unsigned short c;
} usvec3;