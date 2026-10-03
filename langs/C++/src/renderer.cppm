module;

#include <raygui.h>
#include <raylib.h>
#include <raymath.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <exception>
#include <format>
#include <print>
#include <ranges>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

export module renderer;

import boundary;
import configuration;
import particle;
import solver;
import vector;

namespace pbf {

namespace shader {

inline constexpr const char *vertex = R"GLSL(#version 330
in vec3 vertexPosition;
in vec3 vertexNormal;
in mat4 instanceTransform;
out vec2 fragTexCoord;
out vec4 fragColor;
uniform mat4 mvp;
uniform float radius;
void main() {
  fragTexCoord = vec2(0.0);
  fragColor = vec4(1.0);
  gl_Position = mvp * instanceTransform * vec4(vertexPosition * radius, 1.0);
}
)GLSL";

} // namespace shader

[[nodiscard]] inline constexpr Vector3 ToVector3(const Vec3f &v) noexcept {
  return {v[0], v[1], v[2]};
}

export class Renderer {
  static constexpr const char *font_path = "../../josefka.ttf";
  static constexpr int font_size = 18;
  static constexpr float button_size = font_size + 6.0f;
  static constexpr float margin = 8.0f;
  static constexpr float gap = 4.0f;
  static constexpr float label_width = 128.0f;
  static constexpr Rectangle simulate_bounds{margin, margin, button_size, button_size};
  static constexpr Rectangle restart_bounds{margin + (button_size + gap), margin, button_size,
                                            button_size};
  static constexpr Rectangle configure_bounds{margin + 2 * (button_size + gap), margin, button_size,
                                              button_size};
  static constexpr Rectangle wand_bounds{margin + 3 * (button_size + gap), margin, button_size,
                                         button_size};
  static constexpr Rectangle dump_bounds{margin + 4 * (button_size + gap), margin, button_size,
                                         button_size};

  Configuration &configuration;
  Font font{};
  Mesh mesh{};
  Material material{};
  std::vector<Matrix> instance_matrices;
  int radius_uniform{-1};
  Camera3D camera{};
  float accumulator{0.0f};
  bool simulating{false};
  bool configuring{false};
  bool restarting{false};

  struct Slider {
    const char *label;
    float *value;
    float minimum;
    float maximum;
    const char *format;
  };

  static constexpr std::size_t slider_count = 15;
  float solver_iterations_slider{};
  std::array<Slider, slider_count> rows{};

public:
  explicit Renderer(Configuration &configuration)
      : configuration{configuration},
        camera{.position = ToVector3(configuration.visuals.camera_position),
               .target = ToVector3(configuration.visuals.camera_target),
               .up = {0.0f, 1.0f, 0.0f},
               .fovy = 45.0f,
               .projection = CAMERA_PERSPECTIVE} {
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(static_cast<int>(configuration.visuals.window_width),
               static_cast<int>(configuration.visuals.window_height),
               std::format("Position Based Fluids in {}", PBF_LANGUAGE).c_str());
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(480, 480);
    SetTargetFPS(60);

    font = LoadFontEx(font_path, font_size, nullptr, 0);
    GuiSetFont(font);
    GuiSetStyle(DEFAULT, TEXT_SIZE, font_size);
    GuiSetStyle(DEFAULT, TEXT_SPACING, 0);
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_RIGHT);
    GuiEnableTooltip();

    solver_iterations_slider = static_cast<float>(configuration.parameters.solver_iterations);
    rows = std::to_array<Slider>({
        {"Particle radius", &configuration.particles.radius, 0.001f, 0.2f, "%.3f"},
        {"Solver iterations", &solver_iterations_slider, 1.0f, 5.0f, "%.0f"},
        {"Rest density", &configuration.parameters.rest_density, 10.0f, 3000.0f, "%.0f"},
        {"Relaxation epsilon", &configuration.parameters.relaxation_epsilon, 10.0f, 20000.0f,
         "%.0f"},
        {"Vorticity", &configuration.parameters.vorticity_gain, 0.0f, 0.01f, "%.4f"},
        {"Viscosity", &configuration.parameters.viscosity_gain, 0.0f, 0.02f, "%.4f"},
        {"Artificial pressure", &configuration.parameters.artificial_pressure_gain, 0.0f, 0.01f,
         "%.4f"},
        {"Gravity X", &configuration.parameters.gravity[0], -20.0f, 20.0f, "%.2f"},
        {"Gravity Y", &configuration.parameters.gravity[1], -20.0f, 20.0f, "%.2f"},
        {"Gravity Z", &configuration.parameters.gravity[2], -20.0f, 20.0f, "%.2f"},
        {"Wand radius", &configuration.bounds.wand_radius, 0.05f, 4.0f, "%.2f"},
        {"Wand strength", &configuration.bounds.wand_strength, 0.05f, 100.0f, "%.2f"},
        {"Orbit sensitivity", &configuration.visuals.mouse_orbit_sensitivity, 0.0001f, 0.01f,
         "%.4f"},
        {"Pan sensitivity", &configuration.visuals.mouse_pan_sensitivity, 0.0001f, 0.01f, "%.4f"},
        {"Zoom sensitivity", &configuration.visuals.mouse_wheel_sensitivity, 0.001f, 0.2f, "%.3f"},
    });

    mesh = GenMeshSphere(1.0f, 8, 8);
    material = LoadMaterialDefault();
    material.shader = LoadShaderFromMemory(shader::vertex, nullptr);
    material.maps[MATERIAL_MAP_DIFFUSE].color = SKYBLUE;
    radius_uniform = GetShaderLocation(material.shader, "radius");
  }

  Renderer(const Renderer &) = delete;
  Renderer &operator=(const Renderer &) = delete;
  Renderer(Renderer &&) = delete;
  Renderer &operator=(Renderer &&) = delete;

  ~Renderer() {
    if (IsFontValid(font))
      UnloadFont(font);
    UnloadMaterial(material);
    UnloadMesh(mesh);
    CloseWindow();
  }

  void Run(Particles &particles, Solver &solver) {
    instance_matrices.resize(particles.Positions().size());

    const Particles initial_particles{particles};
    while (!WindowShouldClose()) {
      HandleInput(particles, initial_particles);
      Update(particles, solver);
      Draw(particles);
    }
  }

private:
  void HandleInput(Particles &particles, const Particles &initial_particles) {
    if (IsKeyPressed(KEY_SPACE)) {
      simulating = !simulating;
      accumulator = 0.0f;
    }
    if (IsKeyPressed(KEY_C)) {
      configuring = !configuring;
      accumulator = 0.0f;
    }
    if (IsKeyPressed(KEY_R) || restarting) {
      restarting = false;
      particles = initial_particles;
      accumulator = 0.0f;
    }
    if (IsKeyPressed(KEY_W))
      configuration.bounds.wand_active = !configuration.bounds.wand_active;

    if (!configuring) {
      if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        if (IsKeyDown(KEY_LEFT_SHIFT))
          Pan();
        else
          Orbit();
      }
      Zoom();
    }
  }

  void Update(Particles &particles, Solver &solver) {
    if (!simulating || configuring)
      return;

    const float delta_time = configuration.parameters.delta_time;
    accumulator = std::min(accumulator + GetFrameTime(), 5.0f * delta_time);
    if (accumulator < delta_time)
      return;

    const Ray ray = GetMouseRay(GetMousePosition(), camera);
    const Vector3 direction = Vector3Normalize(ray.direction);
    do {
      if (configuration.bounds.wand_active)
        solver.Step(particles, {
                                   .active = true,
                                   .origin = {ray.position.x, ray.position.y, ray.position.z},
                                   .direction = {direction.x, direction.y, direction.z},
                               });
      else
        solver.Step(particles);
      accumulator -= delta_time;
    } while (accumulator >= delta_time);
  }

  void Draw(const Particles &particles) {
    std::ranges::transform(particles.Positions(), instance_matrices.begin(),
                           [](const auto &position) {
                             return MatrixTranslate(position[0], position[1], position[2]);
                           });

    SetShaderValue(material.shader, radius_uniform, &configuration.particles.radius,
                   SHADER_UNIFORM_FLOAT);

    BeginDrawing();
    ClearBackground(RAYWHITE);
    Draw3D();
    DrawGUI(particles);
    EndDrawing();
  }

  void Draw3D() {
    BeginMode3D(camera);

    if (!instance_matrices.empty())
      DrawMeshInstanced(mesh, material, instance_matrices.data(),
                        static_cast<int>(instance_matrices.size()));

    for (const auto &boundary : configuration.bounds.boundaries)
      std::visit(
          [](const auto &shape) {
            using T = std::remove_cvref_t<decltype(shape)>;
            if constexpr (std::same_as<T, Box>) {
              const Vector3 origin = ToVector3(shape.origin);
              const Vector3 size = ToVector3(shape.size * 2.0f);
              DrawCubeV(origin, size, WHITE);
              DrawCubeWiresV(origin, size, BLACK);
            } else if constexpr (std::same_as<T, Sphere>) {
              const Vector3 center = ToVector3(shape.center);
              DrawSphereEx(center, shape.radius, 8, 8, WHITE);
              DrawSphereWires(center, shape.radius, 8, 8, BLACK);
            } else
              std::unreachable();
          },
          boundary);

    DrawCubeWiresV(Vector3{}, ToVector3(configuration.bounds.domain * 2.0f), BLACK);

    EndMode3D();
  }

  void DrawGUI(const Particles &particles) {
    const Rectangle fps_bounds{GetScreenWidth() - label_width - margin, margin, label_width,
                               button_size};

    GuiSetTooltip(simulating ? "Pause [SPACE]" : "Play [SPACE]");
    if (GuiButton(simulate_bounds,
                  GuiIconText(simulating ? ICON_PLAYER_PAUSE : ICON_PLAYER_PLAY, nullptr))) {
      simulating = !simulating;
      accumulator = 0.0f;
    }
    GuiSetTooltip("Restart [R]");
    if (GuiButton(restart_bounds, GuiIconText(ICON_RESTART, nullptr)))
      restarting = true;
    GuiSetTooltip("Configure [C]");
    if (GuiButton(configure_bounds, GuiIconText(ICON_GEAR, nullptr)))
      configuring = !configuring;
    GuiSetTooltip("Wand [W]");
    GuiToggle(wand_bounds, GuiIconText(ICON_WAVE_SINUS, nullptr),
              &configuration.bounds.wand_active);
    GuiSetTooltip("Dump to 'particles.txt'");
    if (GuiButton(dump_bounds, GuiIconText(ICON_FILE_SAVE_CLASSIC, nullptr))) {
      try {
        particles.Dump();
      } catch (const std::exception &e) {
        std::println("Particle dump error: {}", e.what());
      }
    }

    GuiSetTooltip(nullptr);
    std::array<char, 16> fps_text{};
    auto [out, _] = std::format_to_n(fps_text.data(), fps_text.size() - 1, "{} FPS", GetFPS());
    *out = '\0';
    GuiLabel(fps_bounds, fps_text.data());

    if (configuring)
      DrawConfiguration();
  }

  void DrawConfiguration() {
    constexpr float width = 490.0f;
    constexpr float padding = 10.0f;
    constexpr float row_height = 20.0f;
    constexpr float row_spacing = 6.0f;
    constexpr float label_space = 186.0f;
    constexpr float value_space = 70.0f;

    const float height =
        2 * padding + static_cast<float>(rows.size()) * (row_height + row_spacing) - row_spacing;
    const float panel_width = std::min(width, static_cast<float>(GetScreenWidth()) - 2 * padding);
    const float panel_height =
        std::min(height, static_cast<float>(GetScreenHeight()) - 2 * padding);
    const Rectangle bounds{(static_cast<float>(GetScreenWidth()) - panel_width) / 2.0f,
                           (static_cast<float>(GetScreenHeight()) - panel_height) / 2.0f,
                           panel_width, panel_height};
    GuiPanel(bounds, nullptr);

    float y = bounds.y + padding;
    for (const Slider &row : rows) {
      const Rectangle row_bounds{bounds.x + label_space, y,
                                 bounds.width - label_space - value_space, row_height};
      GuiSliderBar(row_bounds, row.label, TextFormat(row.format, *row.value), row.value,
                   row.minimum, row.maximum);
      y += row_height + row_spacing;
    }

    configuration.parameters.solver_iterations =
        static_cast<unsigned>(std::lround(solver_iterations_slider));
  }

  void Pan() {
    const Vector2 delta = GetMouseDelta();
    const Vector3 up = Vector3Normalize(camera.up);
    const Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    const Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, up));
    const Vector3 camera_up = Vector3CrossProduct(right, forward);

    const float speed = Vector3Distance(camera.position, camera.target) *
                        configuration.visuals.mouse_pan_sensitivity;
    const Vector3 offset =
        Vector3Add(Vector3Scale(right, -delta.x * speed), Vector3Scale(camera_up, delta.y * speed));
    camera.position = Vector3Add(camera.position, offset);
    camera.target = Vector3Add(camera.target, offset);
  }

  void Orbit() {
    const Vector2 delta = GetMouseDelta();
    const float yaw = -delta.x * configuration.visuals.mouse_orbit_sensitivity;
    float pitch = -delta.y * configuration.visuals.mouse_orbit_sensitivity;

    const Vector3 up = Vector3Normalize(camera.up);
    Vector3 view = Vector3Subtract(camera.target, camera.position);

    const float max_angle_up = Vector3Angle(up, view) - 0.001f;
    if (pitch > max_angle_up)
      pitch = max_angle_up;
    const float max_angle_down = -Vector3Angle(Vector3Negate(up), view) + 0.001f;
    if (pitch < max_angle_down)
      pitch = max_angle_down;

    view = Vector3RotateByAxisAngle(view, up, yaw);
    view = Vector3RotateByAxisAngle(
        view, Vector3Normalize(Vector3CrossProduct(Vector3Normalize(view), up)), pitch);
    camera.position = Vector3Subtract(camera.target, view);
  }

  void Zoom() {
    const float distance =
        std::max(Vector3Distance(camera.position, camera.target) *
                     std::exp(-configuration.visuals.mouse_wheel_sensitivity * GetMouseWheelMove()),
                 0.001f);
    camera.position = Vector3Add(
        camera.target,
        Vector3Scale(Vector3Normalize(Vector3Subtract(camera.target, camera.position)), -distance));
  }
};

} // namespace pbf
