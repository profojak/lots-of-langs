module;

export module config;

import vector;

namespace pbf {

export struct Configuration {
  struct Box {
    Vec3f origin{0.0f, 0.0f, 0.0f};
    Vec3f size{1.0f, 1.0f, 1.0f};
    Vec3u counts{10, 10, 10};
  };

  Vec3f bounds{2.0f, 2.0f, 2.0f};
};

} // namespace pbf
