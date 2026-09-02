export module configuration;

import vector;

namespace pbf {

export struct Configuration {
  struct Particles {
    Vec3f origin{0.0f, 0.0f, 0.0f};
    Vec3f size{0.5f, 0.5f, 0.5f};
    Vec3u resolution{10, 10, 10};
    float radius = 0.001f;
  } particles;

  struct Parameters {
    float delta_time = 0.01f;
    int solver_iterations = 3;
    int simulation_steps = 1000;
    Vec3f gravity{0.0f, -9.81f, 0.0f};
    float rest_density = 1010.0f;
    float relaxation_epsilon = 450.0f;
    float vorticity_gain = 0.002f;
    float viscosity_gain = 0.001f;
    float artificial_pressure_gain = 0.001f;
  } parameters;

  struct Bounds {
    Vec3f domain{1.0f, 1.0f, 1.0f};
  } bounds;

  struct Visuals {
    int window_width = 800;
    int window_height = 600;
  } visuals;
};

} // namespace pbf
