module;

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <mdspan>
#include <span>
#include <vector>

export module grid;

import kernel;

import vector;

namespace pbf {

export class Grid {
  std::array<float, 3> offset;
  std::dextents<std::size_t, 3> dimensions;
  std::size_t cell_count = 0;
  std::vector<std::size_t> cell_counts;
  std::vector<std::size_t> cell_offsets;
  std::vector<std::size_t> particle_cells;
  std::vector<std::size_t> particle_indices;

  template <std::size_t Axis>
  [[nodiscard]] std::size_t CellCoordinateAt(const Vec3f &position) const {
    const float cell = std::floor((position[Axis] + offset[Axis]) / smoothing_radius);
    const float clamped = std::clamp(cell, 0.0f, static_cast<float>(dimensions.extent(Axis) - 1uz));
    return static_cast<std::size_t>(clamped);
  }

  [[nodiscard]] Vec3u CellCoordinates(const Vec3f &position) const {
    return {CellCoordinateAt<0>(position), CellCoordinateAt<1>(position),
            CellCoordinateAt<2>(position)};
  }

  [[nodiscard]] std::size_t CellIndex(const Vec3u &cell_coordinates) const {
    return std::layout_left::mapping{dimensions}(cell_coordinates[0], cell_coordinates[1],
                                                 cell_coordinates[2]);
  }

  template <std::size_t Axis>
  [[nodiscard]] std::pair<std::size_t, std::size_t> CellNeighborsCoordinates(std::size_t c) const {
    return {c == 0 ? 0 : c - 1, std::min(c + 1, dimensions.extent(Axis) - 1uz)};
  }

public:
  explicit Grid(const Vec3f &domain) {
    std::array<std::size_t, 3> extents{};
    for (std::size_t k = 0; k < 3; ++k) {
      offset[k] = domain[k] + smoothing_radius;
      extents[k] =
          std::max(1uz, static_cast<std::size_t>(std::ceil(offset[k] * 2.0f / smoothing_radius)));
    }
    dimensions = std::dextents<std::size_t, 3>{extents[0], extents[1], extents[2]};
    cell_count = dimensions.extent(0) * dimensions.extent(1) * dimensions.extent(2);
    assert(cell_count >= dimensions.extent(0) && cell_count >= dimensions.extent(1) &&
           cell_count >= dimensions.extent(2));

    cell_counts.resize(cell_count);
    cell_offsets.resize(cell_count + 1);
  }

  template <typename Self>
  [[nodiscard]] std::span<const std::size_t> operator[](this Self &&self, std::size_t x,
                                                       std::size_t y, std::size_t z) {
    const std::size_t cell = std::forward<Self>(self).CellIndex({x, y, z});
    const auto first = std::forward<Self>(self).particle_indices.begin();
    return {first + std::forward<Self>(self).cell_offsets[cell],
            first + std::forward<Self>(self).cell_offsets[cell + 1]};
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

    const float max_distance_squared = smoothing_radius * smoothing_radius;
    for (std::size_t z = z0; z <= z1; ++z)
      for (std::size_t y = y0; y <= y1; ++y)
        for (std::size_t x = x0; x <= x1; ++x)
          for (const std::size_t j : (*this)[x, y, z])
            if (j != i && (position - positions[j]).LengthSquared() <= max_distance_squared)
              f(j);
  }
};

} // namespace pbf
