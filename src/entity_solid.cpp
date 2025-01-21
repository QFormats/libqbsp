#include <qbsp/entity_solid.h>
#include <memory>

namespace qformats::qbsp
{
    SolidEntity::SolidEntity(const bspFileContent &ctx, BaseEntity &ent) : BaseEntity(ent)
    {
        auto &m = ctx.models[modelID];
        for (int fid = m.face_id; fid < m.face_id + m.face_num; fid++)
        {
            auto mface = std::make_shared<Surface>();
            mface->Build(ctx, &ctx.faces[fid]);
            faces.push_back(mface);
        }
    }

    void SolidEntity::convertToOpenGLCoords()
    {
        for (auto &surf : faces)
        {
            for (auto &v : surf->verts)
            {
                auto temp = v.point.y;
                v.point.y = v.point.z;
                v.point.z = -temp;
            }
        }
    }

}