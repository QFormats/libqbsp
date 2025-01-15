#pragma once

#include "ftypes.h"
#include "entity.h"
#include "primitives.h"

namespace qformats::qbsp
{
    class SolidEntity
    {
    public:
        SolidEntity(SolidEntity &&other);
        SolidEntity(const bspFileContent &ctx, BaseEntity &entity);
        const BaseEntity &Entity() const { return ent; }
        const std::vector<FacePtr> &Faces() { return faces; }
        bool IsWorldSPawn() { return ent.Classname() != "worldspawn"; };

    protected:
    private:
        void buildBSPTree(const fNode_t &);
        void getSurfaceIDsFromLeaf(int leafID);
        int getVertIndexFromEdge(int surfEdge);

        BaseEntity &ent;
        std::vector<FacePtr> faces;
        int modelID;

        friend class QBsp;
    };

    typedef std::shared_ptr<SolidEntity> SolidEntityPtr;

}