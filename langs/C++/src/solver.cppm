module;

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <expected>
#include <functional>
#include <limits>
#include <ranges>
#include <span>
#include <type_traits>
#include <utility>
#include <variant>

export module solver;

import boundary;
import configuration;
import grid;
import particle;
import thread;
import vector;
import kernel;

namespace pbf {

using Parameters = Configuration::Parameters;

export class Solver {
public:
  virtual ~Solver() = default;

  virtual void Step(Particles &particles, const Wand &wand = {}) = 0;
};

export class PBFSolver final : public Solver {
  const Configuration &configuration;
  Threads threads;
  Grid grid;

  static void ComputeLambdas(std::size_t begin, std::size_t end, std::span<const Vec3f> predictions,
                             std::span<float> lambdas, const float inv_rest_density,
                             const Parameters &parameters, const Grid &grid) noexcept {
    for (std::size_t i = begin; i < end; ++i) {
      float density = 0.0f;
      Vec3f gradient_sum{0.0f};
      float gradient_sum_squared = 0.0f;
      grid.ForEachNeighbor(predictions, i, [&](std::size_t j) noexcept {
        density += Poly6(predictions[i] - predictions[j]);
        const Vec3f gradient = Spiky(predictions[i] - predictions[j]);
        gradient_sum += gradient;
        gradient_sum_squared += Dot(gradient, gradient);
      });
      density += Poly6(0.0f);
      const float density_constraint = density * inv_rest_density - 1.0f;

      float denominator = (Dot(gradient_sum, gradient_sum) + gradient_sum_squared) *
                              (inv_rest_density * inv_rest_density) +
                          parameters.relaxation_epsilon;
      lambdas[i] = -density_constraint / denominator;
    }
  }

  static void DetectCollisions(Vec3f &correction, const Vec3f &prediction,
                               const Configuration &configuration) noexcept {
    const Vec3f position = prediction + correction;
    for (const std::size_t axis : std::views::iota(0uz, 3uz)) {
      const float minimum = -configuration.bounds.domain[axis] + configuration.particles.radius;
      const float maximum = configuration.bounds.domain[axis] - configuration.particles.radius;
      if (position[axis] < minimum)
        correction[axis] = minimum - prediction[axis];
      else if (position[axis] > maximum)
        correction[axis] = maximum - prediction[axis];
    }

    for (const Boundary &boundary : configuration.bounds.boundaries) {
      const Vec3f target = prediction + correction;
      std::visit(
          [&](const auto &shape) {
            using Shape = std::remove_cvref_t<decltype(shape)>;
            if constexpr (std::same_as<Shape, Box>) {
              const float radius = configuration.particles.radius;
              const Vec3f difference = target - shape.origin;
              const Vec3f penetration{std::abs(difference[0]) - shape.size[0] - radius,
                                      std::abs(difference[1]) - shape.size[1] - radius,
                                      std::abs(difference[2]) - shape.size[2] - radius};

              std::size_t axis = 0uz;
              for (const std::size_t i : std::views::iota(1uz, 3uz))
                if (penetration[i] > penetration[axis])
                  axis = i;
              if (penetration[axis] > 0.0f)
                return;

              const float direction = difference[axis] < 0.0f ? -1.0f : 1.0f;
              correction[axis] += direction * (shape.size[axis] + configuration.particles.radius) -
                                  difference[axis];
            } else if constexpr (std::same_as<Shape, Sphere>) {
              const Vec3f difference = target - shape.center;
              const float distance = difference.Length();
              const float minimum = shape.radius + configuration.particles.radius;
              if (distance >= minimum)
                return;

              const Vec3f direction = distance > std::numeric_limits<float>::epsilon()
                                          ? difference / distance
                                          : Vec3f{0.0f, 1.0f, 0.0f};
              correction = shape.center + direction * minimum - prediction;
            } else
              std::unreachable();
          },
          boundary);
    }
  }

  static void ComputeDeltas(std::size_t begin, std::size_t end, std::span<const Vec3f> predictions,
                            std::span<Vec3f> updated_positions, std::span<const float> lambdas,
                            const float inv_rest_density, const Configuration &configuration,
                            const Grid &grid) noexcept {
    for (std::size_t i = begin; i < end; ++i) {
      Vec3f &correction = updated_positions[i];
      grid.ForEachNeighbor(predictions, i, [&](std::size_t j) noexcept {
        const Vec3f gradient = Spiky(predictions[i] - predictions[j]);
        const float ratio = Poly6(predictions[i] - predictions[j]) / poly6_delta_q;
        const float artificial_pressure =
            -configuration.parameters.artificial_pressure_gain * ratio * ratio * ratio * ratio;
        correction += (lambdas[i] + lambdas[j] + artificial_pressure) * gradient;
      });
      correction *= inv_rest_density;

      DetectCollisions(correction, predictions[i], configuration);
    }
  }

  static void ComputeVorticities(std::size_t begin, std::size_t end,
                                 std::span<const Vec3f> predictions,
                                 std::span<const Vec3f> velocities, std::span<Vec3f> vorticities,
                                 const Grid &grid) noexcept {
    for (std::size_t i = begin; i < end; ++i) {
      Vec3f vorticity{0.0f};
      grid.ForEachNeighbor(predictions, i, [&](std::size_t j) noexcept {
        vorticity += Cross(velocities[j] - velocities[i], Spiky(predictions[i] - predictions[j]));
      });
      vorticities[i] = vorticity;
    }
  }

  static void ApplyVorticityViscosity(std::size_t begin, std::size_t end,
                                      std::span<const Vec3f> predictions,
                                      std::span<const Vec3f> velocities,
                                      std::span<Vec3f> updated_velocities,
                                      std::span<const Vec3f> vorticities,
                                      const Parameters &parameters, const Grid &grid) noexcept {
    for (std::size_t i = begin; i < end; ++i) {
      const float vorticity_length = vorticities[i].Length();
      Vec3f eta{0.0f};
      Vec3f viscosity{0.0f};
      grid.ForEachNeighbor(predictions, i, [&](std::size_t j) noexcept {
        const Vec3f difference = predictions[i] - predictions[j];
        eta += (vorticities[j].Length() - vorticity_length) * Spiky(difference);
        viscosity += (velocities[j] - velocities[i]) * Poly6(difference);
      });

      const Vec3f confinement = eta.Normalize()
                                    .transform([&](const NVec3f &unit_eta) {
                                      return parameters.vorticity_gain *
                                             Cross(unit_eta, vorticities[i]) *
                                             parameters.delta_time;
                                    })
                                    .value_or(Vec3f{});

      updated_velocities[i] = velocities[i] + confinement + parameters.viscosity_gain * viscosity;
    }
  }

  static void PushWithWand(std::size_t begin, std::size_t end, std::span<Vec3f> velocities,
                           std::span<const Vec3f> positions, const Wand &wand,
                           const Configuration &configuration) noexcept {
    const float wand_radius_2 = configuration.bounds.wand_radius * configuration.bounds.wand_radius;
    const float impulse = configuration.bounds.wand_strength * configuration.parameters.delta_time;

    for (std::size_t i = begin; i < end; ++i) {
      const Vec3f offset = positions[i] - wand.origin;
      const float t = std::max(Dot(offset, wand.direction), 0.0f);
      const Vec3f difference = offset - wand.direction * t;
      const float distance_squared = difference.LengthSquared();
      if (distance_squared >= wand_radius_2)
        continue;

      const float distance = std::sqrt(distance_squared);
      const Vec3f direction = distance > std::numeric_limits<float>::epsilon()
                                  ? difference / distance
                                  : Vec3f{0.0f, 1.0f, 0.0f};
      velocities[i] += direction * (impulse * (1.0f - distance / configuration.bounds.wand_radius));
    }
  }

public:
  explicit PBFSolver(const Configuration &configuration)
      : configuration{configuration}, threads{configuration.parameters.threads},
        grid{configuration.bounds.domain} {}

  void Step(Particles &particles, const Wand &wand = {}) override {
    const Parameters &parameters = configuration.parameters;

    std::span positions{particles.Positions()};
    std::span predicted_positions{particles.PredictedPositions()};
    std::span updated_positions{particles.UpdatedPositions()};
    std::span velocities{particles.Velocities()};
    std::span updated_velocities{particles.UpdatedVelocities()};
    std::span vorticities{particles.Vorticities()};
    std::span lambdas{particles.Lambdas()};

    if (parameters.delta_time <= 0.0f || positions.empty())
      return;
    const auto count = positions.size();
    const float inv_rest_density = 1.0f / parameters.rest_density;

    if (wand.active)
      threads.ParallelFor(count, std::bind_back(PushWithWand, velocities, positions,
                                                std::cref(wand), std::cref(configuration)));

    threads.ParallelFor(
        count, [positions, predicted_positions, velocities,
                &parameters = parameters](std::size_t begin, std::size_t end) noexcept {
          for (std::size_t i = begin; i < end; ++i) {
            velocities[i] += parameters.gravity * parameters.delta_time;
            predicted_positions[i] = positions[i] + velocities[i] * parameters.delta_time;
          }
        });

    grid.Rebuild(predicted_positions);

    for (unsigned iteration = 0; iteration < parameters.solver_iterations; ++iteration) {
      threads.ParallelFor(count,
                          std::bind_back(ComputeLambdas, predicted_positions, lambdas,
                                         inv_rest_density, std::cref(parameters), std::cref(grid)));

      std::ranges::fill(updated_positions, Vec3f{});
      threads.ParallelFor(count, std::bind_back(ComputeDeltas, predicted_positions,
                                                updated_positions, lambdas, inv_rest_density,
                                                std::cref(configuration), std::cref(grid)));

      threads.ParallelFor(count, [predicted_positions,
                                  updated_positions](std::size_t begin, std::size_t end) noexcept {
        for (std::size_t i = begin; i < end; ++i)
          predicted_positions[i] += updated_positions[i];
      });
    }

    threads.ParallelFor(
        count, [velocities, positions, predicted_positions,
                &parameters = parameters](std::size_t begin, std::size_t end) noexcept {
          for (std::size_t i = begin; i < end; ++i)
            velocities[i] = (predicted_positions[i] - positions[i]) / parameters.delta_time;
        });

    threads.ParallelFor(count, std::bind_back(ComputeVorticities, predicted_positions, velocities,
                                              vorticities, std::cref(grid)));

    threads.ParallelFor(count, std::bind_back(ApplyVorticityViscosity, predicted_positions,
                                              velocities, updated_velocities, vorticities,
                                              std::cref(parameters), std::cref(grid)));
    std::ranges::copy(updated_velocities, velocities.begin());
    std::ranges::copy(predicted_positions, positions.begin());
  }
};

} // namespace pbf
