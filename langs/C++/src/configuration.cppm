module;

#include <vector>

export module configuration;

import boundary;
import vector;

namespace pbf {

export struct Configuration {
  struct Particles {
    Vec3f origin{0.0f, 0.25f, 0.0f};
    Vec3f size{1.0f, 1.25f, 1.0f};
    Vec3u resolution{20, 25, 20};
    float radius = 0.02f;
  } particles;

  struct Parameters {
    unsigned threads = 0;
    float delta_time = 0.01f;
    unsigned solver_iterations = 3;
    unsigned simulation_steps = 1000;
    Vec3f gravity{0.0f, -9.81f, 0.0f};
    float rest_density = 1010.0f;
    float relaxation_epsilon = 450.0f;
    float vorticity_gain = 0.002f;
    float viscosity_gain = 0.00075f;
    float artificial_pressure_gain = 0.00025f;
  } parameters;

  struct Bounds {
    Vec3f domain{2.0f, 2.0f, 2.0f};
    bool wand_active = true;
    float wand_radius = 1.0f;
    float wand_strength = 30.0f;
    std::vector<Boundary> boundaries{Box{{-0.75f, -1.45f, -1.0f}, {1.25f, 0.55f, 1.0f}},
                                     Box{{1.25f, -1.75f, 0.0f}, {0.75f, 0.25f, 2.0f}},
                                     Sphere{{1.25f, -1.45f, 1.25f}, 0.55f}};
  } bounds;

  struct Visuals {
    unsigned window_width = 800;
    unsigned window_height = 600;
    Vec3f camera_position{0.0f, 3.0f, 8.0f};
    Vec3f camera_target{0.0f, -0.4f, 0.0f};
    float mouse_orbit_sensitivity = 0.003f;
    float mouse_pan_sensitivity = 0.002f;
    float mouse_wheel_sensitivity = 0.01f;
  } visuals;
};

} // namespace pbf
