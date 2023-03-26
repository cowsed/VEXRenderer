#include "gfx.h"
#include "gfx_math.h"
#include "stdlib.h"
struct RenderTarget
{
    RenderTarget(int width, int height) : width(width), height(height)
    {
        color_buffer = (uint32_t *)malloc(sizeof(uint32_t) * width * height);
        depth_buffer = (float *)malloc(sizeof(float) * width * height);
    }
    ~RenderTarget()
    {
        free(color_buffer);
        free(depth_buffer);
    }
    void Clear(uint32_t col, float depth)
    {

        // Clear all pixels
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                color_buffer[y * width + x] = col;
            }
        }
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                depth_buffer[y * width + x] = depth;
            }
        }
    }
    const int width;
    const int height;
    uint32_t *color_buffer;
    float *depth_buffer;
};