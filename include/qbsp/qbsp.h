#pragma once

#include "bsp_file.h"
#include "entity_solid.h"
#include "lightmap.h"
#include "primitives.h"

#include <fstream>
#include <map>

namespace qformats::qbsp {
enum EQBspStatus
{
    QBSP_OK = 0,
    QBSP_ERR_WRONG_VERSION = -1001,
};

struct bspTexure
{
    bspTexure() = default;
    bspTexure(const miptex_t& mt)
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
    unsigned char* data;
};

struct QBspConfig
{
    // load texture lump.
    bool loadTextures = true;
    // also load texture data when loading texture lump.
    bool loadTextureData = true;
    // convert coordinates to OpenGL;
    bool convertCoordToOGL = true;
};

/**
 *  QBsp
 *  Structure of a BSP file.
 */
class QBsp
{
  public:
    QBsp() = default;
    QBsp(QBspConfig cfg)
      : config(cfg) {};
    ~QBsp() = default;
    int LoadFile(const char* filename);
    uint32_t Version() const { return content.header.version; };

    const SolidEntityPtr WorldSpawn() const { return worldSpawn; }
    const map<string, vector<EntityPtr>>& Entities() const { return entities; }
    bool Entities(const string& className, std::function<bool(EntityPtr)> cb) const;

    const vector<EntityPtr>& PointEntities() const { return pointEntities; }
    const vector<SolidEntityPtr>& SolidEntities() const { return solidEntities; }

    static const SolidEntityPtr ToSolidEntity(EntityPtr ent) { return std::dynamic_pointer_cast<qbsp::SolidEntity>(ent); };

    const bspFileContent& Content() const { return content; }
    const vector<bspTexure>& Textures() const { return textures; };
    const Lightmap* LightMap() const { return lm; };

  private:
    void parseEntities(const char* entsrc);
    int loadTextureInfo();
    void prepareLevel();
    void prepareLightMaps();
    void loadTexelBuff(unsigned char** buffOut, uint32_t offset, uint32_t len);

    std::ifstream istream;
    QBspConfig config;

    vector<EntityPtr> pointEntities;
    map<string, vector<EntityPtr>> entities;
    vector<SolidEntityPtr> solidEntities;

    vector<bspTexure> textures;
    SolidEntityPtr worldSpawn;

    bspFileContent content;
    Lightmap* lm;
};
}