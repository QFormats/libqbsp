#include <qbsp/entity_solid.h>
#include <memory>

namespace qformats::qbsp
{
    SolidEntity::SolidEntity(const bspFileContent &ctx, BaseEntity &ent) : ent(ent)
    {
        auto &m = ctx.models[modelID];
        for (int fid = m.face_id; fid < m.face_num; fid++)
        {
            auto mface = std::make_shared<Face>();

            const auto &face = ctx.faces[fid];
            mface->surface = &ctx.surfaces[face.texinfo_id];
            int edgestep = face.ledge_id;
            for (int eid = 0; eid < face.ledge_num; eid++)
            {
                int surfEdgeID = ctx.surfEdges[face.ledge_id + eid];
                vec3f_t v{0};
                auto &edge = ctx.edges[abs(ctx.surfEdges[edgestep])];
                if (surfEdgeID < 0)
                {
                    v = ctx.vertices[edge.vertex0];
                }
                else
                {
                    v = ctx.vertices[edge.vertex1];
                }
                mface->verts.push_back(Vertex{
                    .point = v,
                    .uv{
                        .x = v.dot(mface->surface->u_axis) + mface->surface->u_offset,
                        .y = v.dot(mface->surface->v_axis) + mface->surface->v_offset,
                    }});
                edgestep++;
            }
            faces.push_back(mface);

            mface->indices.resize((face.ledge_num - 2) * 3);
            int tristep = 1;
            for (int i = 1; i < mface->verts.size() - 1; i++)
            {
                mface->indices[tristep - 1] = 0;
                mface->indices[tristep] = i;
                mface->indices[tristep + 1] = i + 1;
                tristep += 3;
            }
        }
    }
}