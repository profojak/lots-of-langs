module;

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <span>
#include <vector>

export module grid;

import kernel;

import vector;

namespace pbf {

export class Grid {
  std::array<float, 3> offset;
  std::array<std::size_t, 3> dimensions;
  std::size_t cell_count = 0;
  std::vector<std::size_t> cell_counts;
  std::vector<std::size_t> cell_offsets;
  std::vector<std::size_t> particle_cells;
  std::vector<std::size_t> particle_indices;

  template <std::size_t Axis>
  [[nodiscard]] std::size_t CellCoordinateAt(const Vec3f &position) const {
    const float cell = std::floor((position[Axis] + offset[Axis]) / radius);
    const float clamped = std::clamp(cell, 0.0f, static_cast<float>(dimensions[Axis] - 1uz));
    return static_cast<std::size_t>(clamped);
  }

  [[nodiscard]] Vec3u CellCoordinates(const Vec3f &position) const {
    return {CellCoordinateAt<0>(position), CellCoordinateAt<1>(position),
            CellCoordinateAt<2>(position)};
  }

  [[nodiscard]] std::size_t CellIndex(const Vec3u &cell_coordinates) const {
    return cell_coordinates[0] +
           dimensions[0] * (cell_coordinates[1] + dimensions[1] * cell_coordinates[2]);
  }

  template <std::size_t Axis>
  [[nodiscard]] std::pair<std::size_t, std::size_t> CellNeighborsCoordinates(std::size_t c) const {
    return {c == 0 ? 0 : c - 1, std::min(c + 1, dimensions[Axis] - 1uz)};
  }

public:
  explicit Grid(const Vec3f &bounds) {
    for (std::size_t k = 0; k < 3; ++k) {
      offset[k] = bounds[k] + radius;
      dimensions[k] = std::max(1uz, static_cast<std::size_t>(std::ceil(offset[k] * 2.0f / radius)));
    }
    cell_count = dimensions[0] * dimensions[1] * dimensions[2];
    assert(cell_count >= dimensions[0] && cell_count >= dimensions[1] &&
           cell_count >= dimensions[2]);

    cell_counts.resize(cell_count);
    cell_offsets.resize(cell_count + 1);
  }

  void Rebuild(std::span<const Vec3f> positions) {
    const std::size_t particle_count = positions.size();
    particle_cells.resize(particle_count);
    particle_indices.resize(particle_count);

    std::ranges::fill(cell_counts, 0uz);
    for (std::size_t i = 0; i < particle_count; ++i) {
      const std::size_t cell = CellIndex(CellCoordinates(positions[i]));
      particle_cells[i] = cell;
      ++cell_counts[cell];
    }

    std::size_t running = 0;
    for (std::size_t c = 0; c < cell_count; ++c) {
      const std::size_t count = cell_counts[c];
      cell_offsets[c] = running;
      cell_counts[c] = running;
      running += count;
    }
    cell_offsets[cell_count] = running;

    for (std::size_t i = 0; i < particle_count; ++i)
      particle_indices[cell_counts[particle_cells[i]]++] = i;
  }

  template <typename F>
    requires std::invocable<F &, std::size_t>
  void ForEachNeighbor(std::span<const Vec3f> positions, std::size_t i, F &&f) const {
    const Vec3f &position = positions[i];
    const Vec3u cell_coordinates = CellCoordinates(position);
    const auto [x0, x1] = CellNeighborsCoordinates<0>(cell_coordinates[0]);
    const auto [y0, y1] = CellNeighborsCoordinates<1>(cell_coordinates[1]);
    const auto [z0, z1] = CellNeighborsCoordinates<2>(cell_coordinates[2]);

    const float max_distance_squared = radius * radius;
    for (std::size_t z = z0; z <= z1; ++z)
      for (std::size_t y = y0; y <= y1; ++y)
        for (std::size_t x = x0; x <= x1; ++x) {
          const std::size_t cell = CellIndex({x, y, z});
          for (std::size_t k = cell_offsets[cell]; k < cell_offsets[cell + 1]; ++k) {
            const std::size_t j = particle_indices[k];
            if (j != i && (position - positions[j]).LengthSquared() <= max_distance_squared)
              f(j);
          }
        }
  }
};

} // namespace pbf
