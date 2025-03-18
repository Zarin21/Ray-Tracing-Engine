#include "color.h"
#include <math.h>

Vec3 unpackRGB(unsigned int packedRGB){
    Vec3 rgb;
    rgb.z = (float)(packedRGB % 256) / 255.0f;          // Blue component
    rgb.y = (float)((packedRGB / 256) % 256) / 255.0f;   // Green component
    rgb.x = (float)((packedRGB / 256 / 256) % 256) / 255.0f; // Red component
    return rgb;
}
void writeColour(FILE *ppmFile, Vec3 color){
    int r = (int)(color.x * 255.0f);
    int g = (int)(color.y * 255.0f);
    int b = (int)(color.z * 255.0f);
    fprintf(ppmFile, "%d %d %d\n", r, g, b);
}
int compareColor(const void *a, const void *b)
{
    int a1 = 0, b1 = 0;
    for (int i = 0; i < (int)sizeof(int); i++)
    {
        a1 |= (*((unsigned char*)a + i) & 0x0F) << (i * 8);
        b1 |= (*((unsigned char*)b + i) & 0x0F) << (i * 8);
    }
    return (a1 < b1) ? -1 : (b1 < a1) ? 1 : (*((int*)a) < *((int*)b)) ? -1 : (*((int*)a) > *((int*)b)) ? 1 : 0;
}
