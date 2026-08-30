module;

export module config;

import vector;

namespace pbf {

export struct Configuration {
  struct Box {
    Vec3f origin{0.0f, 0.0f, 0.0f};
    Vec3f size{0.5f, 0.5f, 0.5f};
    Vec3u counts{10, 10, 10};
  } box;

  Vec3f bounds{1.0f, 1.0f, 1.0f};
};

} // namespace pbf
