module;

#include <fstream>
#include <sstream>

module Model;

namespace render {

auto Model::Load(const std::string& filename) -> Model {
    Model model;
    std::ifstream in(filename, std::ifstream::in);
    if(in.fail())
        throw ModelError("Error loading model file " + filename);

    std::string line;
    while(!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;

        if(!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vector3f v;
            for(auto i: {0, 1, 2})
                iss >> v[i];
            model.vertices.push_back(v);

        } else if(!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            Vector3f n;
            for(auto i: {0, 1, 2})
                iss >> n[i];
            model.normals.emplace_back(n.Normalize());

        } else if(!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            Vector2f uv;
            for(auto i: {0, 1})
                iss >> uv[i];
            model.texels.emplace_back(uv[0], 1 - uv[1]);

        } else if(!line.compare(0, 2, "f ")) {
            int face, texel, normal;
            auto count = 0;
            iss >> trash;
            while(iss >> face >> trash >> texel >> trash >> normal) {
                model.facet_vertices.push_back(--face);
                model.facet_texels.push_back(--texel);
                model.facet_normals.push_back(--normal);
                ++count;
            }
            if(3 != count)
                throw ModelError("Error loading facet");
        }
    }

    auto dot = filename.find_last_of('.');
    model.texture.ReadTgaFile(filename.substr(0, dot) + ".tga");
    return model;
}

} // namespace render
