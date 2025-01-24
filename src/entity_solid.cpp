#include <memory>
#include <qbsp/entity_solid.h>

namespace qformats::qbsp
{
    SolidEntity::SolidEntity(const bspFileContent &ctx, BaseEntity &ent) : BaseEntity(ent)
    {
        auto &m = ctx.models[m_modelId];
        for (int fid = m.face_id; fid < m.face_id + m.face_num; fid++)
        {
            auto mface = std::make_shared<Surface>();
            mface->Build(ctx, &ctx.faces[fid]);
            m_faces.push_back(mface);
        }
    }

    void SolidEntity::convertToOpenGLCoords()
    {
        for (auto &surf : m_faces)
        {
            for (auto &v : surf->verts)
            {
                auto temp = v.point.y;
                v.point.y = v.point.z;
                v.point.z = -temp;
            }
        }
    }

    const std::vector<SurfacePtr> &SolidEntity::Faces()
    {
        return m_faces;
    }

    bool SolidEntity::IsWorldSPawn()
    {
        return m_classname != "worldspawn";
    };
} // namespace qformats::qbsp