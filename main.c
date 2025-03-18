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


Camera camera;
Viewport viewport;
Light light;
Vec3 backgroundColor;
Intersection sphereIntersect(Ray ray, Sphere *sphere);
Intersection findClosestIntersection(Ray ray, World *world);

float imageAspectRatio(int imageWidth, int imageHeight){
    return (float)imageWidth / (float)imageHeight; 
}

void initCameraAndViewport(int imageWidth, int imageHeight, float viewportHeight, float focalLength){
    camera.focalLength = focalLength;

    viewport.height = viewportHeight;
    viewport.width = viewportHeight * imageAspectRatio(imageWidth, imageHeight);
    viewport.z = -focalLength;
}
Ray generateRayMS2(int x, int y, int imageWidth, int imageHeight){
    Ray ray;
    ray.origin = cameraPosition; 

    float pixelX = (float)x + 0.5f;
    float pixelY = (float)y + 0.5f;

    // Pixel coordinates to world coordinates
    float worldX = (pixelX / imageWidth - 0.5f) * viewport.width;
    float worldY = (0.5f - pixelY / imageHeight) * viewport.height;

    Vec3 pixelPosition = {worldX, worldY, viewport.z};
    ray.direction = normalize(subtract(pixelPosition, ray.origin));
    return ray;
}


Ray generateRayFS(int x, int y, int imageWidth, int imageHeight, int sampleX, int sampleY) {
    Ray ray;
    ray.origin = cameraPosition;

    // Calculate the pixel center (original method)
    float pixelX = (float)x + 0.5f;
    float pixelY = (float)y + 0.5f;

    // Offsets for anti-aliasing
    float offsetX = (sampleX - 1) / 3.0f;
    float offsetY = (sampleY - 1) / 3.0f;

    // Pixel coordinates to world coordinates with offset
    float worldX = ((pixelX + offsetX) / imageWidth - 0.5f) * viewport.width;
    float worldY = (0.5f - (pixelY + offsetY) / imageHeight) * viewport.height;

    Vec3 pixelPosition = {worldX, worldY, viewport.z};
    ray.direction = normalize(subtract(pixelPosition, ray.origin));
    return ray;
}


void initLightAndBackgroundColor(Vec3 lightPosition, float lightBrightness, Vec3 bgColor) {
    light.position = lightPosition;
    light.brightness = lightBrightness;
    backgroundColor = bgColor;
}

Vec3 hexToRgb(unsigned int hex) {
    Vec3 rgb;
    rgb.x = ((hex >> 16) & 0xFF) / 255.0f; // Extract red component
    rgb.y = ((hex >> 8) & 0xFF) / 255.0f;  // Extract green component
    rgb.z = (hex & 0xFF) / 255.0f;        // Extract blue component
    return rgb;
}

Vec3 calculateSurfaceNormal(Sphere *sphere, Vec3 intersectionPoint) {
    return normalize(subtract(intersectionPoint, sphere->pos));
}

int isPointInShadow(World *world, Vec3 intersectionPoint, Vec3 lightPos) {
    Vec3 lightDirection = normalize(subtract(lightPos, intersectionPoint));
    Vec3 shadowRayOrigin = add(intersectionPoint, scalarMultiply(0.01f, lightDirection)); // Avoid precision issues

    for (int i = 0; i < world->size; i++) {
        float t;
        if (doesIntersect(world->spheres[i], shadowRayOrigin, lightDirection, &t) && t > 0.01f) {
            Vec3 intersectionToLight = subtract(lightPos, shadowRayOrigin);
            // Ensures the intersection point is between the light and the object casting the shadow (edge case)
            if (t < length(intersectionToLight)) {
                return 1; // Shadow detected
            }
        }
    }
    return 0; // No shadow
}

// Phong shading model
Vec3 calculatePixelColorMS2(Intersection hit, World *world, Vec3 lightPos, float lightBrightness) {
    // If no intersection, return background color
    if (!hit.hit) {
        return backgroundColor;
    }

    Sphere *sphere = hit.sphere;
    Vec3 intersectionPoint = hit.point;
    Vec3 surfaceNormal = calculateSurfaceNormal(sphere, intersectionPoint);
    
    // Light direction (normalized)
    Vec3 lightDirection = normalize(subtract(lightPos, intersectionPoint));
    
    // Intensity based on dot product between normal and light direction
    float distanceToLight = distance(lightPos, intersectionPoint);
    float intensity = fminf(1.0f, 
        lightBrightness * fmaxf(dot(lightDirection, surfaceNormal), 0.0f) / (distanceToLight * distanceToLight)
    );
    // // Distance to light source and falloff
    // float distanceToLight = distance(lightPos, intersectionPoint);
    // intensity *= lightBrightness / (distanceToLight * distanceToLight);

    // Shadow factor
    float shadowFactor = 1.0f;
    if (isPointInShadow(world, intersectionPoint, lightPos)) {
        shadowFactor = 0.1f; // Apply shadow effect
    }

    // Final color calculation with lighting and shadow effect
    Vec3 surfaceLightingColor = scalarMultiply(intensity * shadowFactor, sphere->color);
    return surfaceLightingColor;
}

Vec3 calculatePixelColorFS(Intersection hit, World *world, Vec3 lightPos, float lightBrightness) {
    // If no intersection, return background color
    if (!hit.hit) {
        return backgroundColor;
    }

    Sphere *sphere = hit.sphere;
    Vec3 intersectionPoint = hit.point;
    Vec3 surfaceNormal = calculateSurfaceNormal(sphere, intersectionPoint);
    
    // Light direction (normalized)
    Vec3 lightDirection = normalize(subtract(lightPos, intersectionPoint)); // ensures consistent scaling during computations.
    
    // Intensity based on dot product between normal and light direction
    float distanceToLight = distance(lightPos, intersectionPoint);
    // cosine of the angle between the surfaceNormal and lightDirection
    float intensity = fminf(1.0f, 
        lightBrightness * fmaxf(dot(lightDirection, surfaceNormal), 0.0f) / (distanceToLight * distanceToLight) 
    );

    // Shadow factor
    float shadowFactor = 1.0f;
    if (isPointInShadow(world, intersectionPoint, lightPos)) {
        shadowFactor = 0.1f; // Apply shadow effect
    }

    // Final color calculation with lighting and shadow effect
    Vec3 surfaceLightingColor = scalarMultiply(intensity * shadowFactor, sphere->color);
    return surfaceLightingColor;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    FILE *inputFile = fopen(argv[1], "r");
    if (inputFile == NULL) {
        fprintf(stderr, "Error opening input file.\n");
        return 1;
    }

    int imageWidth, imageHeight;
    float viewportHeight, focalLength;
    float lightBrightness;
    int numColors, numSpheres;
    unsigned int *colors;
    int bgColorIndex; 
    World world;

    fscanf(inputFile, "%d %d", &imageWidth, &imageHeight);

    fscanf(inputFile, "%f", &viewportHeight);
    fscanf(inputFile, "%f", &focalLength);

    initCameraAndViewport(imageWidth, imageHeight, viewportHeight, focalLength); 

    fscanf(inputFile, "%f %f %f", &light.position.x, &light.position.y, &light.position.z);  
    fscanf(inputFile, "%f", &lightBrightness);

    fscanf(inputFile, "%d", &numColors);
    colors = malloc(sizeof(unsigned int) * numColors);
    if (colors == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(1);
    }
    for (int i = 0; i < numColors; i++) {
        fscanf(inputFile, "%x", &colors[i]); 
    }
    // Sort the colors array using qsort and compareColor function
    #ifdef FS
    qsort(colors, numColors, sizeof(unsigned int), compareColor);
    #endif

    fscanf(inputFile, "%d", &bgColorIndex);
    // Set the background color using hexToRgb function
    #ifndef FS
    backgroundColor = (Vec3){0.0f, 0.0f, 0.0f};
    #endif
    #ifdef FS
    backgroundColor = hexToRgb(colors[bgColorIndex]);
    #endif
    

    // Initialize the world
    worldInit(&world);

    // Read number of spheres and their properties
    fscanf(inputFile, "%d", &numSpheres);
    for (int i = 0; i < numSpheres; i++) {
        Vec3 spherePos;
        float sphereRadius;
        int sphereColorIndex;
        fscanf(inputFile, "%f %f %f", &spherePos.x, &spherePos.y, &spherePos.z);
        fscanf(inputFile, "%f", &sphereRadius);
        fscanf(inputFile, "%d", &sphereColorIndex);

        // Set sphere color using hexToRgb function
        
        #ifndef FS
        Vec3 sphereColor = (Vec3){1.0f, 1.0f, 1.0f};
        #endif
        #ifdef FS
        Vec3 sphereColor = hexToRgb(colors[sphereColorIndex]);
        #endif
        Sphere *sphere = createSphere(sphereRadius, spherePos, sphereColor);
        addSphere(&world, sphere);
    }

    fclose(inputFile);

    #ifdef MS1
        // Output for Milestone 1 (txt file)
        FILE *outputFile = fopen(argv[2], "w");
        if (outputFile == NULL) {
            fprintf(stderr, "Error opening output file.\n");
            return 1;
        }

        // Test vector operations
        Vec3 addResult = add(backgroundColor, light.position);
        Vec3 subResult = subtract(backgroundColor, light.position);
        Vec3 scalarMulResult = scalarMultiply(viewport.width, light.position);
        Vec3 normalizeResult = normalize(light.position);

        fprintf(outputFile, "(%.1f, %.1f, %.1f) + (%.1f, %.1f, %.1f) = (%.1f, %.1f, %.1f)\n", 
                backgroundColor.x, backgroundColor.y, backgroundColor.z, 
                light.position.x, light.position.y, light.position.z, 
                addResult.x, addResult.y, addResult.z);
        fprintf(outputFile, "(%.1f, %.1f, %.1f) - (%.1f, %.1f, %.1f) = (%.1f, %.1f, %.1f)\n", 
                backgroundColor.x, backgroundColor.y, backgroundColor.z, 
                light.position.x, light.position.y, light.position.z, 
                subResult.x, subResult.y, subResult.z);
        fprintf(outputFile, "%.1f * (%.1f, %.1f, %.1f) = (%.1f, %.1f, %.1f)\n", 
                viewport.width, light.position.x, light.position.y, light.position.z, 
                scalarMulResult.x, scalarMulResult.y, scalarMulResult.z);
        fprintf(outputFile, "normalize(%.1f, %.1f, %.1f) = (%.1f, %.1f, %.1f)\n", 
                light.position.x, light.position.y, light.position.z, 
                normalizeResult.x, normalizeResult.y, normalizeResult.z);

        // Test sphere operations
        for (int i = 0; i < world.size; i++) {
            Sphere *sphere = world.spheres[i];
            Vec3 scalarDivResult = scalarDivide(sphere->color, sphere->r);
            float dotResult = dot(light.position, sphere->pos);
            float distanceResult = distance(light.position, sphere->pos);
            float lengthResult = length(sphere->pos);

            fprintf(outputFile, "\n(%.1f, %.1f, %.1f) / %.1f = (%.1f, %.1f, %.1f)\n", 
                    sphere->color.x, sphere->color.y, sphere->color.z, sphere->r, 
                    scalarDivResult.x, scalarDivResult.y, scalarDivResult.z);
            fprintf(outputFile, "dot((%.1f, %.1f, %.1f), (%.1f, %.1f, %.1f)) = %.1f\n", 
                    light.position.x, light.position.y, light.position.z, 
                    sphere->pos.x, sphere->pos.y, sphere->pos.z, 
                    dotResult);
            fprintf(outputFile, "distance((%.1f, %.1f, %.1f), (%.1f, %.1f, %.1f)) = %.1f\n", 
                    light.position.x, light.position.y, light.position.z, 
                    sphere->pos.x, sphere->pos.y, sphere->pos.z, 
                    distanceResult);
            fprintf(outputFile, "length(%.1f, %.1f, %.1f) = %.1f\n", 
                    sphere->pos.x, sphere->pos.y, sphere->pos.z, 
                    lengthResult);
        }
        fclose(outputFile);
    #endif

    #ifdef MS2
    FILE *outputFile = fopen(argv[2], "w");
    if (outputFile == NULL) {
        fprintf(stderr, "Error opening output file.\n");
        return 1;
    }

    // PPM header
    fprintf(outputFile, "P3\n%d %d\n255\n", imageWidth, imageHeight);

    // Render scene
    for (int y = 0; y < imageHeight; y++) {
        for (int x = 0; x < imageWidth; x++) {
            Ray ray = generateRayMS2(x, y, imageWidth, imageHeight);
            
            // Find closest intersection
            Intersection hit = {0};
            float closestDistance = INFINITY;
            for (int i = 0; i < world.size; i++) {
                float t;
                if (doesIntersect(world.spheres[i], ray.origin, ray.direction, &t) && t > 0.01f) {
                    if (t < closestDistance) {
                        closestDistance = t;
                        hit.hit = 1;
                        hit.point = add(ray.origin, scalarMultiply(t, ray.direction));
                        hit.sphere = world.spheres[i];
                    }
                }
            }

            // Calculate pixel color
            Vec3 pixelColor = calculatePixelColorMS2(hit, &world, light.position, lightBrightness);

            // Output pixel color
            writeColour(outputFile, pixelColor);
        }
        fprintf(outputFile, "\n");
    }

    fclose(outputFile);

    #endif


    #ifdef FS
    FILE *outputFile = fopen(argv[2], "w");
    if (outputFile == NULL) {
        fprintf(stderr, "Error opening output file.\n");
        return 1;
    }

    // PPM header
    fprintf(outputFile, "P3\n%d %d\n255\n", imageWidth, imageHeight);

    for (int y = 0; y < imageHeight; y++) {
    for (int x = 0; x < imageWidth; x++) {
        // Variables to accumulate the color
        Vec3 pixelColor = {0, 0, 0};

        // Sample the pixel 9 times (3x3 grid)
        for (int sampleY = 0; sampleY < 3; sampleY++) {
            for (int sampleX = 0; sampleX < 3; sampleX++) {
                Ray ray = generateRayFS(x, y, imageWidth, imageHeight, sampleX, sampleY);

                // Find closest intersection
                Intersection hit = {0};
                float closestDistance = INFINITY;
                for (int i = 0; i < world.size; i++) {
                    float t;
                    if (doesIntersect(world.spheres[i], ray.origin, ray.direction, &t) && t > 0.01f) {
                        if (t < closestDistance) {
                            closestDistance = t;
                            hit.hit = 1;
                            hit.point = add(ray.origin, scalarMultiply(t, ray.direction));
                            hit.sphere = world.spheres[i];
                        }
                    }
                }
                // Assuming colors is an array of unsigned int representing color values

                // Calculate pixel color for this sample
                Vec3 sampleColor = calculatePixelColorFS(hit, &world, light.position, lightBrightness);
                pixelColor = add(pixelColor, sampleColor);
            }
        }

        // Average the pixel color from all 9 samples
        pixelColor = scalarMultiply(1.0f / 9.0f, pixelColor);

        // Output the averaged pixel color
        writeColour(outputFile, pixelColor);
    }
    fprintf(outputFile, "\n");
    }

    fclose(outputFile);
    
    #endif


    // Cleanup
    free(colors);
    freeWorld(&world);

    return 0;
}
