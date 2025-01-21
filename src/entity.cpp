#include <qbsp/entity.h>
#include <sstream>
#include <regex>

namespace qformats::qbsp
{
    inline string &ltrim(string &s, const char *t = " ")
    {
        s.erase(0, s.find_first_not_of(t));
        return s;
    }

    static vector<string> rexec_vec(string line, const string &regexstr)
    {
        std::stringstream results;
        std::regex re(regexstr, std::regex::icase);

        vector<string> matches;
        const std::sregex_token_iterator end;
        for (std::sregex_token_iterator it(line.begin(), line.end(), re, 1); it != end; it++)
        {
            matches.push_back(*it);
        }

        return matches;
    }

    void BaseEntity::ParseEntites(const char *entsrc, std::function<void(BaseEntity &ent)> f)
    {
        auto entstr = std::stringstream(entsrc);
        int current_index = 0;
        auto current_ent = BaseEntity();

        for (string line; std::getline(entstr, line);)
        {
            std::erase(line, '\r');
            if (line.empty())
            {
                continue;
            }
            if (line.starts_with("//"))
            {
                continue;
            }
            line = ltrim(line);
            if (line == "{")
            {
                current_ent = BaseEntity();
                continue;
            }

            if (line == "}")
            {
                current_ent.setup();

                f(current_ent);
                continue;
            }

            auto matches = rexec_vec(line, R"(\"([\s\S]*?)\")");
            if (matches.empty())
            {
                continue;
            }

            current_ent.attributes.emplace(matches[0], matches[1]);
        }
    }

    void BaseEntity::setup()
    {
        if (attributes.contains("classname"))
        {
            classname = attributes["classname"];
            if (classname == "worldspawn")
            {
                this->type = ETypeSolidEntity;
                this->modelID = 0;
            }
        }

        if (attributes.contains("origin"))
        {
            std::stringstream stream(attributes["origin"]);
            stream >> origin.x >> origin.y >> origin.z;
            return;
        }

        if (attributes.contains("model"))
        {
            const auto &model = attributes["model"];
            if (model.starts_with("*"))
            {
                modelID = std::stoi(attributes["model"].c_str() + 1);
                type = ETypeSolidEntity;
                return;
            }
            isExternalModel = true;
            return;
        }

        if (attributes.contains("angle"))
        {

            std::stringstream stream(attributes["angle"]);
            stream >> angle;
            return;
        }
    }

    void BaseEntity::convertToOpenGLCoords()
    {
        auto temp = origin.y;
        origin.y = origin.z;
        origin.z = -temp;
        angle += 180;
    }
}