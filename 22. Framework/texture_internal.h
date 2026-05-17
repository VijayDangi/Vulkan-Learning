#ifndef __TEXTURE_INTERNAL_H__
#define __TEXTURE_INTERNAL_H__

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <math.h>

#include <vector>

#include <glm/glm.hpp>

#define CUBEMAP_NUM_FACE 6

#define CUBEMAP_INDEX_POS_X 0
#define CUBEMAP_INDEX_NEG_X 1
#define CUBEMAP_INDEX_POS_Y 2
#define CUBEMAP_INDEX_NEG_Y 3
#define CUBEMAP_INDEX_POS_Z 4
#define CUBEMAP_INDEX_NEG_Z 5

#define __MIN(a, b) ((a) < (b)) ? (a) : (b)
#define __MAX(a, b) ((a) > (b)) ? (a) : (b)

#define CLAMP(x, minVal, maxVal) __MIN( __MAX(x, minVal), maxVal)

static glm::vec3 FaceCoordsToXYZ(unsigned int x, unsigned int y, unsigned int faceId, unsigned int faceSize)
{
    // code
    float A = 2.0f * float(x) / faceSize;
    float B = 2.0f * float(y) / faceSize;

    glm::vec3 ret;

    switch(faceId)
    {
        case CUBEMAP_INDEX_POS_X:
            ret = glm::vec3(A - 1.0f, 1.0f, 1.0f - B);
        break;
        case CUBEMAP_INDEX_NEG_X:
            ret = glm::vec3(1.0f - A, -1.0f, 1.0f - B);
        break;
        case CUBEMAP_INDEX_POS_Y:
            ret = glm::vec3(1.0f - B, A - 1.0f, 1.0f);
        break;
        case CUBEMAP_INDEX_NEG_Y:
            ret = glm::vec3(B - 1.0f, A - 1.0f, - 1.0f);
        break;
        case CUBEMAP_INDEX_POS_Z:
            ret = glm::vec3(- 1.0f, A - 1.0f, 1.0f - B);
        break;
        case CUBEMAP_INDEX_NEG_Z:
            ret = glm::vec3(1.0f, 1.0f - A, 1.0f - B);
        break;

        default: assert(0);
    }

    return ret;
}

static unsigned int ConvertEquirectangularImageToCubemap(const unsigned char *pixels, unsigned int width, unsigned int height, std::vector< std::vector<unsigned char>>& outCubePixels)
{
    // code
    unsigned int faceSize = width / 4;

    outCubePixels.resize(CUBEMAP_NUM_FACE);

    unsigned int maxW = width - 1;
    unsigned int maxH = height - 1;

    for(unsigned int face = 0; face < CUBEMAP_NUM_FACE; ++face)
    {
        outCubePixels[face].resize(faceSize * faceSize * 4);

        for(unsigned int y = 0; y < faceSize; ++y)
        {
            for(unsigned int x = 0; x < faceSize; ++x)
            {
                glm::vec3 p = FaceCoordsToXYZ(x, y, face, faceSize);
                float r = sqrtf(p.x * p.x + p.y * p.y);
                float phi = atan2f(p.y, p.x);
                float theta = atan2f(p.z, r);

                // Calculate texture coordinates
                float u = (float)((phi + M_PI) / (2.0f * M_PI));
                float v = (float)((M_PI / 2.0f - theta) / M_PI);

                // Scale texture coordinates by image size
                float U = u * width;
                float V = v * height;

                // 4-samples for bilinear interpolation
                int U1 = CLAMP(int(floor(U)), 0, maxW);
                int V1 = CLAMP(int(floor(V)), 0, maxH);

                int U2 = CLAMP(U1 + 1, 0, maxW);
                int V2 = CLAMP(V1 + 1, 0, maxH);

                // Calculate the fractional part
                float s = U - U1;
                float t = V - V1;

                // Fetch 4-samples
                glm::vec4 bottomLeft  = glm::vec4(pixels[4 * (V1 * width + U1) + 0], pixels[4 * (V1 * width + U1) + 1], pixels[4 * (V1 * width + U1) + 2], pixels[4 * (V1 * width + U1) + 3]);
                glm::vec4 bottomRight = glm::vec4(pixels[4 * (V1 * width + U2) + 0], pixels[4 * (V1 * width + U2) + 1], pixels[4 * (V1 * width + U2) + 2], pixels[4 * (V1 * width + U2) + 3]);
                glm::vec4 topLeft     = glm::vec4(pixels[4 * (V2 * width + U1) + 0], pixels[4 * (V2 * width + U1) + 1], pixels[4 * (V2 * width + U1) + 2], pixels[4 * (V2 * width + U1) + 3]);
                glm::vec4 topRight    = glm::vec4(pixels[4 * (V2 * width + U2) + 0], pixels[4 * (V2 * width + U2) + 1], pixels[4 * (V2 * width + U2) + 2], pixels[4 * (V2 * width + U2) + 3]);

                // Bilinear interpolation
                glm::vec4 color = bottomLeft  * (1 - s) * (1 - t) +
                                  bottomRight *      s  * (1 - t) +
                                  topLeft     * (1 - s) *      t  +
                                  topRight    *      s  *      t;

                outCubePixels[face][4 * (y * faceSize + x) + 0] = color.x;
                outCubePixels[face][4 * (y * faceSize + x) + 1] = color.y;
                outCubePixels[face][4 * (y * faceSize + x) + 2] = color.z;
                outCubePixels[face][4 * (y * faceSize + x) + 3] = color.w;
            }
        }
    }

    return faceSize;
}

#endif