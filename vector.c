#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "vector.h"

Vec3 add(Vec3 v1, Vec3 v2){
    Vec3 vec_addition = {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
    return vec_addition; 
}

Vec3 subtract(Vec3 v1, Vec3 v2){
    Vec3 vec_substract = {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
    return vec_substract;
}

Vec3 scalarMultiply(float s, Vec3 v){
    Vec3 scalar_Multi = {s * v.x, s * v.y, s * v.z};
    return scalar_Multi;
}

Vec3 scalarDivide(Vec3 v, float d){
    if (d == 0){
        printf("Can't be divided by 0.");
        exit(EXIT_FAILURE);
    }
    Vec3 scalar_div = {v.x / d, v.y / d, v.z / d};
    return scalar_div;
}
float dot(Vec3 v1, Vec3 v2){
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
float length2(Vec3 v){
    return v.x * v.x + v.y * v.y + v.z * v.z;
}
float length(Vec3 v){
    return sqrt(length2(v));
}
float distance2(Vec3 v1, Vec3 v2){
    return length2(subtract(v1, v2));
}
float distance(Vec3 v1, Vec3 v2){
    return length(subtract(v1, v2));
}
Vec3 normalize(Vec3 v){
    float len = length(v);
    // if (len == 0) {
    //     fprintf(stderr, "Error: Cannot normalize a zero-length vector.\n");
    //     exit(EXIT_FAILURE);
    // }
    Vec3 result = {v.x / len, v.y / len, v.z / len};
    return result;
}
