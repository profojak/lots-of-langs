module;

#include <raylib.h>
#include <raymath.h>

#include <algorithm>
#include <cmath>
#include <concepts>
#include <format>
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

static constexpr const char *vertex = R"GLSL(#version 330
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

[[nodiscard]] inline Vector3 ToVector3(const Vec3f &vector) noexcept {
  return {vector[0], vector[1], vector[2]};
}

export class Renderer {
  const Configuration &configuration;

  Mesh mesh{};
  Material material{};
  std::vector<Matrix> instance_matrices;
  int radius_uniform{-1};

  inline void Pan(Camera3D &camera) {
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

  inline void Orbit(Camera3D &camera) {
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

  inline void Zoom(Camera3D &camera) {
    const float distance =
        std::max(Vector3Distance(camera.position, camera.target) *
                     std::exp(-configuration.visuals.mouse_wheel_sensitivity * GetMouseWheelMove()),
                 0.001f);
    camera.position = Vector3Add(
        camera.target,
        Vector3Scale(Vector3Normalize(Vector3Subtract(camera.target, camera.position)), -distance));
  }

public:
  explicit Renderer(const Configuration &configuration) : configuration{configuration} {
    InitWindow(configuration.visuals.window_width, configuration.visuals.window_height,
               std::format("Position Based Fluids in {}", PBF_LANGUAGE).c_str());
    SetTargetFPS(60);

    mesh = GenMeshSphere(1.0f, 8, 8);
    material = LoadMaterialDefault();
    material.shader = LoadShaderFromMemory(shader::vertex, nullptr);
    material.maps[MATERIAL_MAP_DIFFUSE].color = SKYBLUE;
    radius_uniform = GetShaderLocation(material.shader, "radius");
  }

  Renderer(const Renderer &) = delete;
  Renderer &operator=(const Renderer &) = delete;

  ~Renderer() {
    UnloadMaterial(material);
    UnloadMesh(mesh);
    CloseWindow();
  }

  void Run(Particles &particles, Solver &solver) {
    instance_matrices.resize(particles.Positions().size());

    Camera3D camera = {
        .position = ToVector3(configuration.visuals.camera_position),
        .target = ToVector3(configuration.visuals.camera_target),
        .up = {0.0f, 1.0f, 0.0f},
        .fovy = 45.0f,
        .projection = CAMERA_PERSPECTIVE,
    };

    constexpr int max_steps_per_frame = 5;
    float accumulator = 0.0f;
    while (!WindowShouldClose()) {
      if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        if (IsKeyDown(KEY_LEFT_SHIFT))
          Pan(camera);
        else
          Orbit(camera);
      }
      Zoom(camera);

      const Ray ray = GetMouseRay(GetMousePosition(), camera);
      const Vector3 direction = Vector3Normalize(ray.direction);
      accumulator = std::min(accumulator + GetFrameTime(),
                             max_steps_per_frame * configuration.parameters.delta_time);
      while (accumulator >= configuration.parameters.delta_time) {
        if (configuration.bounds.wand_active)
          solver.Step(particles, {
                                     .active = true,
                                     .origin = {ray.position.x, ray.position.y, ray.position.z},
                                     .direction = {direction.x, direction.y, direction.z},
                                 });
        else
          solver.Step(particles);
        accumulator -= configuration.parameters.delta_time;
      }

      std::ranges::transform(particles.Positions(), instance_matrices.begin(),
                             [](const auto &position) {
                               return MatrixTranslate(position[0], position[1], position[2]);
                             });

      SetShaderValue(material.shader, radius_uniform, &configuration.particles.radius,
                     SHADER_UNIFORM_FLOAT);

      BeginDrawing();
      ClearBackground(RAYWHITE);
      BeginMode3D(camera);

      {
        if (!instance_matrices.empty())
          DrawMeshInstanced(mesh, material, instance_matrices.data(),
                            static_cast<int>(instance_matrices.size()));

        for (const auto &boundary : configuration.bounds.boundaries)
          std::visit(
              [](const auto &shape) {
                using T = std::remove_cvref_t<decltype(shape)>;
                if constexpr (std::same_as<T, Box>) {
                  DrawCubeV(ToVector3(shape.origin), ToVector3(shape.size * 2.0f), WHITE);
                  DrawCubeWiresV(ToVector3(shape.origin), ToVector3(shape.size * 2.0f), BLACK);
                } else if constexpr (std::same_as<T, Sphere>) {
                  DrawSphereEx(ToVector3(shape.center), shape.radius, 8, 8, WHITE);
                  DrawSphereWires(ToVector3(shape.center), shape.radius, 8, 8, BLACK);
                } else
                  std::unreachable();
              },
              boundary);

        DrawCubeWiresV(Vector3{}, ToVector3(configuration.bounds.domain * 2.0f), BLACK);
      }

      EndMode3D();
      EndDrawing();
    }
  }
};

} // namespace pbf
