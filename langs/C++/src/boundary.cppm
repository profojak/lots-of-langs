module;

#include <variant>

export module boundary;

import vector;

export namespace pbf {

struct Wand {
  bool active = false;
  Vec3f origin{};
  Vec3f direction{};
};

struct Box {
  Vec3f origin{};
  Vec3f size{};
};

struct Sphere {
  Vec3f center{};
  float radius{};
};

using Boundary = std::variant<Box, Sphere>;

} // namespace pbf
