#include <qbsp/qbsp.h>

namespace qformats::qbsp {
template <typename T>
bool loadLumptoVector(std::ifstream &istream, const lump_t &lump, vector<T> &v) {
    if (lump.length == 0) {
        return false;
    }
    v.resize(lump.length / sizeof(T));
    istream.seekg(lump.offset, istream.beg);
    istream.read((char *)(&v[0]), lump.length);
    return true;
}

int QBsp::LoadFile(const char *fileName) {
    istream.open(fileName, std::ios::binary);
    istream.read((char *)(&content.header), sizeof(header_t));

    if (content.header.version != MAGIC_V29 && content.header.version != MAGIC_V30) {
        return QBSP_ERR_WRONG_VERSION;
    }

    loadLumptoVector(istream, content.header.lump[LUMP_VERTICES], content.vertices);
    loadLumptoVector(istream, content.header.lump[LUMP_EDGES], content.edges);
    loadLumptoVector(istream, content.header.lump[LUMP_FACES], content.faces);
    loadLumptoVector(istream, content.header.lump[LUMP_TEXINFO], content.surfaces);
    loadLumptoVector(istream, content.header.lump[LUMP_SURFEDGES], content.surfEdges);
    loadLumptoVector(istream, content.header.lump[LUMP_MODELS], content.models);

    loadLumptoVector(istream, content.header.lump[LUMP_PLANES], content.planes);
    loadLumptoVector(istream, content.header.lump[LUMP_NODES], content.nodes);
    loadLumptoVector(istream, content.header.lump[LUMP_LEAFS], content.leafs);

    if (config.loadTextures) {
        loadTextureInfo();
    }

    if (content.header.lump[LUMP_ENTITIES].length > 0) {
        istream.seekg(content.header.lump[LUMP_ENTITIES].offset, istream.beg);
        char *ents = (char *)malloc(content.header.lump[LUMP_ENTITIES].length + 1 / sizeof(char));
        istream.read(ents, content.header.lump[LUMP_ENTITIES].length);
        BaseEntity::ParseEntites(ents, [&](BaseEntity &e) {
            if (e.Type() == ETypeSolidEntity) {
                auto se = std::make_shared<SolidEntity>(this->Content(), e);
                this->solidEntities.emplace_back(se);
                this->entities[se->classname].push_back(se);
                if (se->Classname() == "worldspawn") {
                    this->worldSpawn = se;
                }
                return;
            }
            this->pointEntities.push_back(std::make_shared<BaseEntity>(e));
            this->entities[e.classname].emplace_back(std::make_shared<BaseEntity>(e));
        });
        free(ents);
    }

    prepareLightMaps();
    prepareLevel();
    istream.close();

    return QBSP_OK;
}

void QBsp::prepareLightMaps() {
    int lm_size = content.header.lump[LUMP_LIGHTING].length;
    uint8_t *lm_dataBW = (uint8_t *)calloc(sizeof(uint8_t), lm_size);
    istream.seekg(content.header.lump[LUMP_LIGHTING].offset, istream.beg);
    istream.read((char *)lm_dataBW, content.header.lump[LUMP_LIGHTING].length);

    uint8_t *lm_dataRGB = (uint8_t *)calloc(sizeof(uint8_t), lm_size * 3);
    int i2 = 0;
    for (int i = 0; i < lm_size; i++) {
        uint8_t d = lm_dataBW[i];
        lm_dataRGB[i2++] = d;
        lm_dataRGB[i2++] = d;
        lm_dataRGB[i2++] = d;
    }
    free(lm_dataBW);
    lm = new Lightmap(lm_dataRGB, lm_size);
    lm->PackLitSurfaces(solidEntities);
}

void QBsp::prepareLevel() {
    if (config.convertCoordToOGL) {
        for (auto pe : entities) {
            for (auto &e : pe.second) {
                e->convertToOpenGLCoords();
            }
        }
    }
}

int QBsp::loadTextureInfo() {
    if (!istream.is_open() || content.header.version == 0) {
        return -1;
    }

    mipheader_t mh;
    istream.seekg(content.header.lump[LUMP_TEXTURES].offset, istream.beg);
    istream.read((char *)(&mh.numtex), sizeof(int32_t));
    mh.offset = (int32_t *)malloc(sizeof(int32_t) * mh.numtex);
    istream.read((char *)(mh.offset), sizeof(int32_t) * mh.numtex);

    for (int i = 0; i < mh.numtex; i++) {
        auto mho = mh.offset[i];
        if (mh.offset[i] < 0) continue;
        miptex_t miptex;
        auto t = mh.offset[i];
        istream.seekg(content.header.lump[LUMP_TEXTURES].offset + mh.offset[i], istream.beg);
        istream.read((char *)(&miptex), sizeof(miptex_t));
        content.miptextures.push_back(miptex);
        bspTexure tex(miptex);
        if (config.loadTextureData) {
            auto texOffset = content.header.lump[LUMP_TEXTURES].offset + mh.offset[i] + miptex.offset[0];
            loadTexelBuff(&tex.data, texOffset, miptex.width * miptex.height);
            tex.hasData = true;
            tex.name = miptex.name;
        }
        textures.push_back(tex);
    }

    return 0;
}

void QBsp::loadTexelBuff(unsigned char **buffOut, uint32_t offset, uint32_t len) {
    istream.seekg(offset, istream.beg);
    *buffOut = (unsigned char *)malloc(len);
    istream.read((char *)(*buffOut), len);
    return;
}

bool QBsp::Entities(const string &className, std::function<bool(EntityPtr)> cb) const {
    if (auto ev = entities.find(className); ev != entities.end()) {
        for (const auto e : entities.find(className)->second) {
            if (!cb(e)) break;
        }
    }

    return false;
}
}  // namespace qformats::qbsp