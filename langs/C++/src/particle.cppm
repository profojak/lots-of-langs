module;

#include <ranges>
#include <tuple>
#include <utility>
#include <vector>

export module particle;

import config;
import vector;

namespace pbf {

export struct Particle {
  Vec3f position;
  Vec3f predicted_position;
  Vec3f updated_position;
  Vec3f velocity;
  Vec3f updated_velocity;
  Vec3f vorticity;
  float lambda;

  [[nodiscard]] constexpr auto Members(this Particle &self) noexcept {
    auto &[p, pp, up, v, uv, vr, l] = self;
    return std::tie(p, pp, up, v, uv, vr, l);
  }
};

template <typename Tuple> struct SoA;
template <typename... Ts> struct SoA<std::tuple<Ts...>> {
  using type = std::tuple<std::vector<std::remove_cvref_t<Ts>>...>;
};

export class Particles {
  using types = decltype(std::declval<Particle &>().Members());
  typename SoA<types>::type data;

  template <std::size_t N> [[nodiscard]] constexpr decltype(auto) Slice(this auto &&self) noexcept {
    return std::get<N>(std::forward<decltype(self)>(self).data);
  }

public:
  [[nodiscard]] constexpr decltype(auto) Positions(this auto &&self) noexcept {
    return self.template Slice<0>();
  }

  [[nodiscard]] constexpr decltype(auto) PredictedPositions(this auto &&self) noexcept {
    return self.template Slice<1>();
  }

  [[nodiscard]] constexpr decltype(auto) UpdatedPositions(this auto &&self) noexcept {
    return self.template Slice<2>();
  }

  [[nodiscard]] constexpr decltype(auto) Velocities(this auto &&self) noexcept {
    return self.template Slice<3>();
  }

  [[nodiscard]] constexpr decltype(auto) UpdatedVelocities(this auto &&self) noexcept {
    return self.template Slice<4>();
  }

  [[nodiscard]] constexpr decltype(auto) Vorticities(this auto &&self) noexcept {
    return self.template Slice<5>();
  }

  [[nodiscard]] constexpr decltype(auto) Lambdas(this auto &&self) noexcept {
    return self.template Slice<6>();
  }

  explicit Particles(const Configuration::Particles &particles) {
    const std::size_t total =
        particles.resolution[0] * particles.resolution[1] * particles.resolution[2];
    if (total == 0)
      return;

    std::apply([total](auto &...vectors) { (vectors.resize(total), ...); }, data);
    auto &positions = std::get<0>(data);

    const Vec3f size = particles.size * 2.0f / Vec3f{particles.resolution};
    const Vec3f start = particles.origin - particles.size + size * 0.5f;

    std::size_t index = 0;
    for (std::size_t i : std::views::iota(0uz, particles.resolution[0]))
      for (std::size_t j : std::views::iota(0uz, particles.resolution[1]))
        for (std::size_t k : std::views::iota(0uz, particles.resolution[2]))
          positions[index++] = start + Vec3f{i, j, k} * size;
  }
};

} // namespace pbf
