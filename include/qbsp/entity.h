#pragma once

#include "bsp_file.h"
#include <functional>

namespace qformats::qbsp
{
    class BaseEntity;
    class SolidEntity;

    typedef std::shared_ptr<BaseEntity> EntityPtr;

    enum EEntityType
    {
        ETypePontEntity = 0,
        ETypeSolidEntity = 1,
    };

    class BaseEntity
    {
    public:
        BaseEntity() {};
        virtual ~BaseEntity() = default;
        const string &Classname() const { return classname; };
        const map<string, string> &Attributes() const { return attributes; };
        EEntityType Type() const { return type; };
        bool IsExternalModel() const { return isExternalModel; };
        int ModelID() const { return modelID; };
        const vec3f_t &Origin() const { return origin; };
        const float &Angle() const { return angle; };
        static void ParseEntites(const char *entsrc, std::function<void(BaseEntity &ent)> f);

    protected:
        virtual void convertToOpenGLCoords();

        int modelID = 0;
        map<string, string> attributes;
        string classname = "";
        vec3f_t origin = {0};
        EEntityType type = ETypePontEntity;
        bool isExternalModel = false;
        float angle = 0;

    private:
        void setup();

        friend class QBsp;
    };
}