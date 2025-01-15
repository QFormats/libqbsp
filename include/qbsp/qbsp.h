#pragma once

#include "ftypes.h"
#include "primitives.h"
#include "entity_solid.h"

#include <fstream>

namespace qformats::qbsp
{
    enum EQBspStatus
    {
        QBSP_OK = 0,
        QBSP_ERR_WRONG_VERSION = -1001,
    };

    struct bspTexure
    {
        bspTexure() = default;
        bspTexure(const miptex_t &mt)
        {
            width = mt.width;
            height = mt.height;
            name = string(mt.name);
        };
        std::string name;
        uint32_t id;
        uint32_t width;
        uint32_t height;
        bool hasData;
        unsigned char *data;
    };

    struct QBspConfig
    {
        // load texture lump.
        bool loadTextures = false;
        // also load texture data when loading texture lump.
        bool loadTextureData = true;
    };

    /**
     *  QBsp
     *  Structure of a BSP file.
     */
    class QBsp
    {
    public:
        QBsp() = default;
        QBsp(QBspConfig cfg) : config(cfg) {};
        ~QBsp() = default;
        int LoadFile(const char *filename);
        uint32_t Version() const { return content.header.version; };

        const SolidEntityPtr WorldSpawn() const { return worldSpawn; }
        const vector<SolidEntityPtr> &SolidEntities() const { return solidEntities; }
        const bspFileContent &Content() const { return content; }
        const vector<bspTexure> &Textures() const { return textures; };

    private:
        void parseEntities(const char *entsrc);
        int loadTextureInfo();
        void prepareLevel();
        void loadTexelBuff(unsigned char **buffOut, uint32_t offset, uint32_t len);
        std::ifstream istream;
        QBspConfig config;
        vector<SolidEntityPtr> solidEntities;
        vector<bspTexure> textures;
        SolidEntityPtr worldSpawn;

        bspFileContent content;
    };
}