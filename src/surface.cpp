#include <algorithm>
#include <cmath>
#include <float.h>
#include <qbsp/primitives.h>

namespace qformats::qbsp
{

void Surface::Build(const bspFileContent &ctx, const fFace_t *fsurf)
{
    float mins[2], maxs[2], val;
    int bmins[2], bmaxs[2];

    mins[0] = mins[1] = FLT_MAX;
    maxs[0] = maxs[1] = -FLT_MAX;

    float max_u = 0, max_v = 0;
    float min_u = 0, min_v = 0;

    fsurface = fsurf;
    info = &ctx.surfaces[fsurface->texinfo_id];
    const auto &tex = ctx.miptextures[info->texture_id];

    textureReference = &tex;

    const double tex_vecs[2][4] = {
        {info->u_axis.x, info->u_axis.y, info->u_axis.z, info->u_offset},
        {info->v_axis.x, info->v_axis.y, info->v_axis.z, info->v_offset},
    };

    verts.resize(fsurface->ledge_num);
    for (int i = 0; i < fsurface->ledge_num; i++)
    {
        int e = ctx.surfEdges[fsurface->ledge_id + i];
        auto &v = verts[fsurface->ledge_num - i - 1];
        if (e >= 0)
            v.point = ctx.vertices[ctx.edges[e].vertex0];
        else
            v.point = ctx.vertices[ctx.edges[-e].vertex1];

        for (int i = 0; i < 2; i++)
        {
            val = ((double)v.point.x * tex_vecs[i][0]);
            val += ((double)v.point.y * tex_vecs[i][1]);
            val += ((double)v.point.z * tex_vecs[i][2]);
            val += tex_vecs[i][3];

            mins[i] = std::min(mins[i], val);
            maxs[i] = std::max(maxs[i], val);
        }

        v.uv.x = (v.point.dot(info->u_axis) + info->u_offset) / tex.width;
        v.uv.y = (v.point.dot(info->v_axis) + info->v_offset) / tex.height;

        max_u = std::max(max_u, v.uv.x);
        max_v = std::max(max_v, v.uv.y);

        min_u = std::min(min_u, v.uv.x);
        min_v = std::min(min_v, v.uv.y);
    }

    for (int i = 0; i < 2; i++)
    {
        bmins[i] = floor(mins[i] / 16);
        bmaxs[i] = ceil(maxs[i] / 16);

        texturemins[i] = bmins[i] * 16;
        extents[i] = (bmaxs[i] - bmins[i]) * 16;
    }

    indices.resize((fsurface->ledge_num - 2) * 3);
    int tristep = 1;
    for (int i = 1; i < verts.size() - 1; i++)
    {
        indices[tristep - 1] = 0;
        indices[tristep] = i;
        indices[tristep + 1] = i + 1;
        tristep += 3;
    }
}
} // namespace qformats::qbsp