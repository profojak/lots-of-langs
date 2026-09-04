module;

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <format>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>

export module configuration;

import argument;
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

  void LoadFromFile(std::string_view path);
  void LoadFromArguments(const Arguments &args);

private:
  void Set(std::string_view key, std::string_view value);
};

namespace detail {

template <typename T> struct Tag {};

[[noreturn]] inline void InvalidValue(std::string_view value, std::string_view type) {
  throw std::invalid_argument(std::format("Invalid {} value '{}'!", type, value));
}

template <typename T>
T Parse(Tag<T>, std::string_view value)
  requires std::same_as<T, bool>
{
  if (value == "true" || value == "1")
    return true;
  if (value == "false" || value == "0")
    return false;
  InvalidValue(value, "bool");
}

template <typename T>
T Parse(Tag<T>, std::string_view value)
  requires arithmetic<T> && (!std::same_as<T, bool>)
{
  T result{};
  const auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), result);
  const bool ok = ec == std::errc{} && ptr == value.data() + value.size() &&
                  (!std::floating_point<T> || std::isfinite(result));
  if (!ok)
    InvalidValue(value, "scalar");
  return result;
}

template <arithmetic T, std::size_t N>
Vector<T, N> Parse(Tag<Vector<T, N>>, std::string_view value) {
  Vector<T, N> result;
  std::size_t start = 0;
  for (std::size_t i = 0; i < N; ++i) {
    const auto comma = value.find(',', start);
    const bool last = i == N - 1;
    if (last != (comma == std::string_view::npos))
      InvalidValue(value, "vector");
    result[i] = Parse(Tag<T>{}, value.substr(start, comma - start));
    start = comma + 1;
  }
  return result;
}

template <auto Outer, auto Inner> void Assign(Configuration &c, std::string_view value) {
  using T = std::remove_cvref_t<decltype((c.*Outer).*Inner)>;
  (c.*Outer).*Inner = Parse(Tag<T>{}, value);
}

template <typename B> void EmplaceBoundary(Configuration &c, std::string_view value) {
  if constexpr (std::same_as<B, Box>) {
    const auto v = Parse(Tag<Vector<float, 6>>{}, value);
    c.bounds.boundaries.emplace_back(Box{{v[0], v[1], v[2]}, {v[3], v[4], v[5]}});
  } else if constexpr (std::same_as<B, Sphere>) {
    const auto v = Parse(Tag<Vector<float, 4>>{}, value);
    c.bounds.boundaries.emplace_back(Sphere{{v[0], v[1], v[2]}, v[3]});
  } else {
    static_assert(std::same_as<B, Box> || std::same_as<B, Sphere>);
    std::unreachable();
  }
}

struct Field {
  std::string_view key;
  void (*assign)(Configuration &, std::string_view);
};

inline constexpr auto fields = std::to_array<Field>({
    {"particles.origin", Assign<&Configuration::particles, &Configuration::Particles::origin>},
    {"particles.size", Assign<&Configuration::particles, &Configuration::Particles::size>},
    {"particles.resolution",
     Assign<&Configuration::particles, &Configuration::Particles::resolution>},
    {"particles.radius", Assign<&Configuration::particles, &Configuration::Particles::radius>},
    {"parameters.threads", Assign<&Configuration::parameters, &Configuration::Parameters::threads>},
    {"parameters.delta_time",
     Assign<&Configuration::parameters, &Configuration::Parameters::delta_time>},
    {"parameters.solver_iterations",
     Assign<&Configuration::parameters, &Configuration::Parameters::solver_iterations>},
    {"parameters.simulation_steps",
     Assign<&Configuration::parameters, &Configuration::Parameters::simulation_steps>},
    {"parameters.gravity", Assign<&Configuration::parameters, &Configuration::Parameters::gravity>},
    {"parameters.rest_density",
     Assign<&Configuration::parameters, &Configuration::Parameters::rest_density>},
    {"parameters.relaxation_epsilon",
     Assign<&Configuration::parameters, &Configuration::Parameters::relaxation_epsilon>},
    {"parameters.vorticity_gain",
     Assign<&Configuration::parameters, &Configuration::Parameters::vorticity_gain>},
    {"parameters.viscosity_gain",
     Assign<&Configuration::parameters, &Configuration::Parameters::viscosity_gain>},
    {"parameters.artificial_pressure_gain",
     Assign<&Configuration::parameters, &Configuration::Parameters::artificial_pressure_gain>},
    {"bounds.domain", Assign<&Configuration::bounds, &Configuration::Bounds::domain>},
    {"bounds.wand_active", Assign<&Configuration::bounds, &Configuration::Bounds::wand_active>},
    {"bounds.wand_radius", Assign<&Configuration::bounds, &Configuration::Bounds::wand_radius>},
    {"bounds.wand_strength", Assign<&Configuration::bounds, &Configuration::Bounds::wand_strength>},
    {"bounds.boundaries.box", EmplaceBoundary<Box>},
    {"bounds.boundaries.sphere", EmplaceBoundary<Sphere>},
    {"visuals.window_width",
     Assign<&Configuration::visuals, &Configuration::Visuals::window_width>},
    {"visuals.window_height",
     Assign<&Configuration::visuals, &Configuration::Visuals::window_height>},
    {"visuals.camera_position",
     Assign<&Configuration::visuals, &Configuration::Visuals::camera_position>},
    {"visuals.camera_target",
     Assign<&Configuration::visuals, &Configuration::Visuals::camera_target>},
    {"visuals.mouse_orbit_sensitivity",
     Assign<&Configuration::visuals, &Configuration::Visuals::mouse_orbit_sensitivity>},
    {"visuals.mouse_pan_sensitivity",
     Assign<&Configuration::visuals, &Configuration::Visuals::mouse_pan_sensitivity>},
    {"visuals.mouse_wheel_sensitivity",
     Assign<&Configuration::visuals, &Configuration::Visuals::mouse_wheel_sensitivity>},
});

} // namespace detail

inline void Configuration::LoadFromFile(std::string_view path) {
  std::ifstream file{std::string(path)};
  if (!file)
    throw std::runtime_error(std::format("Failed to open configuration file '{}'!", path));

  Configuration parsed;
  parsed.bounds.boundaries.clear();
  for (std::string line; std::getline(file, line);) {
    if (line.empty() || line.starts_with('#'))
      continue;
    const auto equals = line.find('=');
    if (equals == std::string::npos)
      throw std::invalid_argument(std::format("Invalid line '{}' in configuration file!", line));
    const std::string_view key = std::string_view{line}.substr(0, equals);
    const std::string_view value = std::string_view{line}.substr(equals + 1);
    if (key.empty() || value.empty())
      throw std::invalid_argument(
          std::format("Key or value cannot be empty, got '{}={}'!", key, value));
    parsed.Set(key, value);
  }
  *this = std::move(parsed);
}

inline void Configuration::LoadFromArguments(const Arguments &args) {
  Configuration parsed = *this;
  for (const auto &[key, value] : args.parameters)
    parsed.Set(key, value);
  *this = std::move(parsed);
}

inline void Configuration::Set(std::string_view key, std::string_view value) {
  const auto field = std::ranges::find(detail::fields, key, &detail::Field::key);
  if (field == std::ranges::end(detail::fields))
    throw std::invalid_argument(std::format("Unknown configuration key '{}'!", key));
  field->assign(*this, value);
}

} // namespace pbf
