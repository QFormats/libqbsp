#pragma once

#include "ftypes.h"
#include <functional>

namespace qformats::qbsp
{
    class BaseEntity;
    class SolidEntity;

    typedef std::shared_ptr<BaseEntity> entityPtr;
    typedef vector<entityPtr> entityList;

    enum EEntityType
    {
        ETypePontEntity = 0,
        ETypeSolidEntity = 1,
    };

    class BaseEntity
    {
    public:
        BaseEntity() {};
        const string &Classname() const { return classname; };
        const map<string, string> &Attributes() const { return attributes; };
        EEntityType Type() const { return type; };
        bool IsExternalModel() const { return isExternalModel; };
        int ModelID() const { return modelID; };
        static void ParseEntites(const char *entsrc, std::function<void(BaseEntity *ent)> f);

    protected:
        int modelID;
        map<string, string> attributes;
        string classname;
        vec3f_t origin;
        EEntityType type;
        bool isExternalModel;
        float angle;

    private:
        void setup();
    };
}