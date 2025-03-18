#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "vector.h"
#include "color.h"
#include "spheres.h"

Vec3 cameraPosition = {0, 0, 0};

typedef struct {
    float focalLength;
} Camera;

typedef struct {
    float width;
    float height;
    float z;
} Viewport;

typedef struct {
    Vec3 origin;
    Vec3 direction;
} Ray;

typedef struct {
    Vec3 position;
    float brightness;
} Light;

typedef struct {
    int hit;
    Vec3 point;
    float distance;
    Sphere *sphere;  
} Intersection;
