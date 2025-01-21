#include <cmath>
#include <qbsp/lightmap.h>
#include <string.h>

namespace qformats::qbsp {
void
Lightmap::PackLitSurfaces(std::vector<SolidEntityPtr> ents)
{
    int maxblack[2] = { 0, 0 };
    short blackofs[2] = { 0, 0 };
    int blacklm;

    // generate surface list
    for (auto m : ents) {
        for (auto surf : m->Faces()) {
            if (surf->fsurface->lightmap != -1) {
                surf->lm_samples = data + (surf->fsurface->lightmap * 3); // johnfitz -- lit support via lordhavoc (was "+ i")
            }
            lit_surfs.push_back(surf);
        }
    }

    blacklm = allocateBlock(maxblack[0] + 1, maxblack[1] + 1, &blackofs[0], &blackofs[1]);

    if (lit_surfs.size() == 0)
        return;

    // TODO: do a heap sort
    for (auto surf : lit_surfs) {
        int smax = (surf->extents[0] >> 4) + 1;
        int tmax = (surf->extents[1] >> 4) + 1;
        this->m_sample_count += smax * tmax;

        if (surf->lm_samples) {
            surf->lm_tex_num = allocateBlock(smax, tmax, &surf->lm_s, &surf->lm_t);
            continue;
        } else {
            surf->lm_tex_num = blacklm;
            surf->lm_s = blackofs[0];
            surf->lm_t = blackofs[1];
        }
    }

    // determine combined texture size and allocate memory for it
    int xblocks = (int)std::ceil(std::sqrt(m_count));
    int yblocks = (m_count + xblocks - 1) / xblocks;
    m_lm_width = xblocks * LM_BLOCK_WIDTH;
    m_lm_height = yblocks * LM_BLOCK_HEIGHT;
    int lmsize = m_lm_width * m_lm_height;

    lightmap_data.resize(lmsize);
    // compute offsets for each lightmap block
    for (int i = 0; i < m_count; i++) {
        auto* lm = &lm_offsets[i];
        lm->x = (i % xblocks) * LM_BLOCK_WIDTH;
        lm->y = (i / xblocks) * LM_BLOCK_HEIGHT;
    }

    // fill reserved texel
    lightmap_data[0].Set(0x80, 0x80, 0x80, 0xff);

    // fill lightmap samples
    for (auto ls : lit_surfs) {
        auto lmofs = ((ls->extents[0] >> 4) + 1) / (float)m_lm_width;
        auto lm = &lm_offsets[ls->lm_tex_num];
        float lmscalex = 1.f / 16.f / m_lm_width;
        float lmscaley = 1.f / 16.f / m_lm_height;

        for (auto& v : ls->verts) {
            auto s = v.point.dot(ls->info->u_axis) + ls->info->u_offset;
            s -= ls->texturemins[0];
            s += (ls->lm_s + lm->x) * 16;
            s += 8;
            s *= lmscalex;

            auto t = v.point.dot(ls->info->v_axis) + ls->info->v_offset;
            t -= ls->texturemins[1];
            t += (ls->lm_t + lm->y) * 16;
            t += 8;
            t *= lmscaley;

            v.lm_uv.x = s;
            v.lm_uv.y = t;
        }
        fillSurfaceLightmap(ls);
    }

    return;
}

void
Lightmap::fillSurfaceLightmap(SurfacePtr surf)
{
    if (!data || !surf->lm_samples || surf->fsurface->light[0] == 255) {
        return;
    }

    auto& lm = lm_offsets[surf->lm_tex_num];
    auto smax = (surf->extents[0] / 16) + 1;
    auto tmax = (surf->extents[1] / 16) + 1;
    auto xofs = lm.x + surf->lm_s;
    auto yofs = lm.y + surf->lm_t;
    auto facesize = smax * tmax * 3;

    auto src = surf->lm_samples;
    auto dst = &lightmap_data.front() + yofs * m_lm_width + xofs;

    // fill our RGBA lightmap pixel buffer
    for (int t = 0; t < tmax; t++, dst += m_lm_width) {
        for (int s = 0; s < smax; s++, src += 3) {
            dst[s].Set(src[0], src[1], src[2], 0xff);
        }
    }
}

void
Lightmap::initChart(LightmapChart* chart, int width, int height)
{
    if (chart->width != width) {
        chart->allocated = (int*)realloc(chart->allocated, sizeof(chart->allocated[0]) * width);
    }
    memset(chart->allocated, 0, sizeof(chart->allocated[0]) * width);
    chart->width = width;
    chart->height = height;
    chart->x = 0;
    chart->reverse = false;
}

bool
Lightmap::addChart(LightmapChart* chart, int w, int h, short* outx, short* outy)
{
    int i, x, y;
    if (chart->width < w || chart->height < h) {
        return false;
    }

    // advance horizontally, reversing direction at the edges
    if (chart->reverse) {
        if (chart->x < w) {
            chart->x = 0;
            chart->reverse = false;
            goto forward;
        }
    reverse:
        x = chart->x - w;
        chart->x = x;
    } else {
        if (chart->x + w > chart->width) {
            chart->x = chart->width;
            chart->reverse = true;
            goto reverse;
        }
    forward:
        x = chart->x;
        chart->x += w;
    }

    // find lowest unoccupied vertical position
    y = 0;
    for (i = 0; i < w; i++)
        y = std::max(y, chart->allocated[x + i]);
    if (y + h > chart->height)
        return false;

    // update vertical position for each column
    for (i = 0; i < w; i++)
        chart->allocated[x + i] = y + h;

    *outx = x;
    *outy = y;

    return true;
}

int
Lightmap::allocateBlock(int w, int h, short* x, short* y)
{
    for (int texnum = m_last_allocated; texnum < MAX_SANITY_LIGHTMAPS; texnum++) {
        if (texnum == m_count) {
            m_count++;
            lm_offsets.push_back(vec2i_t{ 0 });
            initChart(&chart, LM_BLOCK_WIDTH, LM_BLOCK_HEIGHT);
            if (m_count == 1) {
                chart.x = 1;
                chart.allocated[0] = 1;
            }
        }

        if (!addChart(&chart, w, h, x, y))
            continue;

        m_last_allocated = texnum;
        return texnum;
    }
    return 0;
}
}