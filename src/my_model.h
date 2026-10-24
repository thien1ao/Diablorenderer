#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "geometry.h"

class Model {
  std::vector<vec3> vertices = {};
  std::vector<int> faces = {};
public:
  Model(const std::string& filename);
  int nvertices() const;
  int nfaces() const;
  vec3 vert(const int i) const;
  vec3 vert(const int iface, const int nthvert) const; // query one of three vertices in triangle
};
