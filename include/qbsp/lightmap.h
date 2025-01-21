#pragma once

#include "entity_solid.h"

namespace qformats::qbsp
{
    const int LM_MAX_WIDTH = 1024;
    const int LM_MAX_HEIGHT = 1024;
    const int LM_BLOCK_WIDTH = 256;
    const int LM_BLOCK_HEIGHT = 256;
    const int MAX_SANITY_LIGHTMAPS = (1u << 20);

    struct Color
    {
        union
        {
            struct
            {
                uint8_t r, g, b, a;
            };
            uint8_t rgba[4];
        };
        void Set(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
        {
            this->r = r, this->g = g, this->b = b, this->a = a;
        };
    };

    struct LightmapChart
    {
        bool reverse;
        int x;
        int width;
        int height;
        int *allocated;
    };

    class Lightmap
    {
    public:
        Lightmap(uint8_t *data, size_t sz)
        {
            this->data = data;
            size = sz;
            chart = LightmapChart{0};
        };
        void PackLitSurfaces(std::vector<SolidEntityPtr> ent);

        const int Width() const { return m_lm_width; }
        const int Height() const { return m_lm_height; }
        const vector<Color> &RGBA() const { return lightmap_data; }

    private:
        void initChart(LightmapChart *chart, int width, int height);
        bool addChart(LightmapChart *chart, int w, int h, short *outx, short *outy);
        int allocateBlock(int w, int h, short *x, short *y);
        void fillSurfaceLightmap(SurfacePtr surf);

        LightmapChart chart;
        std::vector<vec2i_t> lm_offsets;
        std::vector<SurfacePtr> lit_surfs;

        uint8_t *data;
        vector<Color> lightmap_data;
        uint32_t *lightmap_data2;
        size_t size;
        int m_count;
        int m_sample_count;
        int m_last_allocated;

        int m_lm_width;
        int m_lm_height;
    };
}