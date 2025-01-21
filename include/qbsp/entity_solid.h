#pragma once

#include "bsp_file.h"
#include "entity.h"
#include "primitives.h"

namespace qformats::qbsp
{
    class SolidEntity : public BaseEntity
    {
    public:
        SolidEntity(SolidEntity &&other);
        SolidEntity(const bspFileContent &ctx, BaseEntity &entity);
        const std::vector<SurfacePtr> &Faces() { return faces; }
        bool IsWorldSPawn() { return classname != "worldspawn"; };

    protected:
        virtual void convertToOpenGLCoords();

    private:
        void buildBSPTree(const fNode_t &);
        void getSurfaceIDsFromLeaf(int leafID);
        int getVertIndexFromEdge(int surfEdge);
        std::vector<SurfacePtr> faces;

        friend class QBsp;
    };

    typedef std::shared_ptr<SolidEntity> SolidEntityPtr;

}