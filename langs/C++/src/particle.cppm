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
  Vec3f velocity;
  Vec3f density_delta;
  float lambda;

  [[nodiscard]] constexpr auto Fields(this Particle &self) noexcept {
    auto &[p, pp, v, dd, l] = self;
    return std::tie(p, pp, v, dd, l);
  }
};

template <typename Tuple> struct SoA;
template <typename... Ts> struct SoA<std::tuple<Ts...>> {
  using type = std::tuple<std::vector<std::remove_cvref_t<Ts>>...>;
};

export class Particles {
  using types = decltype(std::declval<Particle &>().Fields());
  typename SoA<types>::type data;
  static constexpr std::size_t field_count = std::tuple_size_v<types>;

public:
  enum class Field {
    Position = 0,
    PredictedPosition = 1,
    Velocity = 2,
    DensityDelta = 3,
    Lambda = 4
  };
  static_assert(static_cast<std::size_t>(Field::Lambda) + 1 == field_count);

  explicit Particles(const Configuration::Box &box) {
    const std::size_t total = box.counts[0] * box.counts[1] * box.counts[2];
    if (total == 0)
      return;

    std::apply([total](auto &...vecs) { (vecs.resize(total), ...); }, data);
    auto &positions = std::get<static_cast<std::size_t>(Field::Position)>(data);

    const Vec3f size = {box.size[0] * 2.0f / static_cast<float>(box.counts[0]),
                        box.size[1] * 2.0f / static_cast<float>(box.counts[1]),
                        box.size[2] * 2.0f / static_cast<float>(box.counts[2])};

    const Vec3f start = {box.origin[0] - box.size[0] + size[0] * 0.5f,
                         box.origin[1] - box.size[1] + size[1] * 0.5f,
                         box.origin[2] - box.size[2] + size[2] * 0.5f};

    std::size_t idx = 0;
    for (std::size_t i : std::views::iota(0uz, box.counts[0])) {
      for (std::size_t j : std::views::iota(0uz, box.counts[1])) {
        for (std::size_t k : std::views::iota(0uz, box.counts[2])) {
          positions[idx++] = Vec3f{start[0] + static_cast<float>(i) * size[0],
                                   start[1] + static_cast<float>(j) * size[1],
                                   start[2] + static_cast<float>(k) * size[2]};
        }
      }
    }
  }

  template <Field F>
  [[nodiscard]] constexpr decltype(auto) Get(this auto &&self) noexcept {
    constexpr std::size_t field_index = static_cast<std::size_t>(F);
    static_assert(field_index < field_count, "Field out of range!");
    return std::get<field_index>(std::forward<decltype(self)>(self).data);
  }
};

} // namespace pbf
