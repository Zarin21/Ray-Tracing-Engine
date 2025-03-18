#include "spheres.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

void worldInit(World *world){
    world->spheres = malloc(sizeof(Sphere *)); // Initial capacity of 1
    if (world->spheres == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(1);
    }
    world->size = 0;
    world->capacity = 1;
}
void freeWorld(World *world){
    for (int i = 0; i < world->size; i++) {
        free(world->spheres[i]);
        
    }
    free(world->spheres);
}
void addSphere(World *world, Sphere *sphere){
    if (world->size == world->capacity) {
        // Double the capacity if needed
        world->capacity *= 2;
        world->spheres = realloc(world->spheres, sizeof(Sphere *) * world->capacity);
        if (world->spheres == NULL) {
            fprintf(stderr, "Memory allocation failed!\n");
            exit(1);
        }
    }
    world->spheres[world->size++] = sphere;
}
Sphere *createSphere(float radius, Vec3 position, Vec3 color){
    Sphere *sphere = malloc(sizeof(Sphere));
    if (sphere == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(1);
    }
    sphere->r = radius;
    sphere->pos = position;
    sphere->color = color;
    return sphere;
}
int doesIntersect(const Sphere *sphere, Vec3 rayPos, Vec3 rayDir, float *t){
    Vec3 V = subtract(rayPos, sphere->pos); // vector from ray origin to sphere center

    // Quadratic Coefficients
    float a = dot(rayDir, rayDir);
    float b = 2.0f * dot(rayDir, V);
    float c = dot(V,V) - sphere->r * sphere->r;

    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0){
        return 0; // no intersection
    }

    // solutions
    float sqrtD = sqrtf(discriminant);
    float t1 = (-b - sqrtD) / (2.0f * a);
    float t2 = (-b + sqrtD) / (2.0f * a);

    if (t1 > 0 && t2 > 0){
       *t = (t1 < t2) ? t1 : t2; // Both infront. taking the lower one
    }
    else if (t1 > 0)
    {
        *t = t1; // t1 infront
    }
    else if (t2 > 0)
    {
        *t = t2; // t2 infront
    }
    else
    {
        return 0; // both behind
    }
    return 1; // intersection found


}
