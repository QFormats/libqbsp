#pragma once
#include <memory>

#include "ftypes.h"
#include "entity.h"

namespace qformats::qbsp
{
    struct Vertex
    {
        vec3f_t point;
        vec3f_t normal;
        vec2f_t uv;
        vec2f_t lm_uv;
    };

    struct Face
    {
        int id;
        int lightmapID;
        const fSurface_t *surface;
        vector<Vertex> verts;
        vector<uint32_t> indices;
    };

    typedef std::shared_ptr<Face> FacePtr;
}