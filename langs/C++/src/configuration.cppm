export module configuration;

import vector;

namespace pbf {

export struct Configuration {
  struct Particles {
    Vec3f origin{0.0f, 0.0f, 0.0f};
    Vec3f size{0.5f, 0.5f, 0.5f};
    Vec3u resolution{10, 10, 10};
  } particles;

  struct Parameters {
    float delta_time = 0.01f;
    float particle_size = 0.001f;
    int solver_iterations = 3;
    Vec3f gravity{0.0f, -9.81f, 0.0f};
    float rest_density = 1010.0f;
    float relaxation_epsilon = 450.0f;
    float vorticity_gain = 0.002f;
    float viscosity_gain = 0.001f;
    float artificial_pressure_gain = 0.001f;
  } parameters;

  Vec3f bounds{1.0f, 1.0f, 1.0f};
};

} // namespace pbf
