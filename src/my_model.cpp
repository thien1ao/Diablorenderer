#include "my_model.h"
#include <fstream>
#include <sstream>

Model::Model(const std::string& filename) {
    // open stage, check fail
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail()) return;

    // read file linewise
    std::string line;
    while (std::getline(in, line)) {
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash; // write "v" to trash
            vec3 v;
            for (int i : {0,1,2}) iss >> v[i];
            vertices.push_back(v);

        } else if (!line.compare(0, 2, "f ")) {
            int f,t,n, cnt = 0;
            // f: index geom vert
            // t: index texture vert
            // n: index normal
            iss >> trash;

            while (iss >> f >> trash >> t >> trash >> n) {
                faces.push_back(--f);
                cnt++;
            }
            if (3!=cnt) {
                std::cerr << "Error: the obj file is supposed to be triangulated" << std::endl;
                return;
            }
        }
    }
    std::cerr << "# v# " << nvertices() << " f# "  << nfaces() << std::endl;
}

int Model::nvertices() const { return vertices.size(); }

int Model::nfaces() const { return faces.size() / 3; }

vec3 Model::vert(const int i) const { return vertices[i]; }

vec3 Model::vert(const int iface, const int nthvert) const { return vertices[faces[iface * 3 + nthvert]]; }
