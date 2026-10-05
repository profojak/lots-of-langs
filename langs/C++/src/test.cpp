#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <format>
#include <print>
#include <random>
#include <ranges>
#include <source_location>
#include <stdexcept>
#include <string_view>

import vector;
import kernel;

namespace {

constexpr std::uint32_t seed = 0xC0FFEE42;
constexpr int samples{100};

struct Test {
  using TestFunction = void(std::mt19937 &);
  std::string_view name;
  TestFunction *run;
};

void Check(bool condition, std::string_view message,
           std::source_location location = std::source_location::current()) {
  if (!condition)
    throw std::runtime_error(std::format("{}:{} in {}: {}", location.file_name(), location.line(),
                                         location.function_name(), message));
}

template <std::floating_point T>
[[nodiscard]] constexpr bool IsNear(T actual, T expected,
                                    T tolerance = static_cast<T>(1e-10)) noexcept {
  if (!std::isfinite(actual) || !std::isfinite(expected))
    return false;
  return std::abs(actual - expected) <= tolerance;
}

constexpr auto tests = std::to_array<Test>({
    Test{"vector arithmetic",
         [](std::mt19937 &) {
           const pbf::Vector<int, 3> a{1, 2, 3};
           const pbf::Vector<int, 3> b{4, 5, 6};
           const auto sum = a + b;
           const auto cross = Cross(a, b);
           constexpr pbf::Vector<int, 3> expected_sum{5, 7, 9};
           constexpr pbf::Vector<int, 3> expected_cross{-3, 6, -3};
           for (const std::size_t axis : std::views::iota(0uz, 3uz)) {
             Check(sum[axis] == expected_sum[axis],
                   std::format("axis {}: addition must return expected values", axis));
             Check(cross[axis] == expected_cross[axis],
                   std::format("axis {}: cross product must return expected values", axis));
           }
           Check(Dot(a, b) == 32.0, "dot product must return expected value");
           const pbf::Vector<int, 3> x{1, 0, 0};
           const pbf::Vector<int, 3> y{0, 1, 0};
           const auto z = Cross(x, y);
           Check(z[0] == 0 && z[1] == 0 && z[2] == 1,
                 "cross product must return right-handed result");
         }},

    Test{"random vector arithmetic",
         [](std::mt19937 &random) {
           std::uniform_real_distribution<double> distribution{-10.0, 10.0};
           for ([[maybe_unused]] const int sample : std::views::iota(0, samples)) {
             const pbf::Vector<double, 3> a{distribution(random), distribution(random),
                                            distribution(random)};
             const pbf::Vector<double, 3> b{distribution(random), distribution(random),
                                            distribution(random)};
             const auto recovered = (a + b) - b;
             for (const std::size_t axis : std::views::iota(0uz, 3uz))
               Check(IsNear(recovered[axis], a[axis]),
                     std::format("axis {}: '(a + b) - b' must equal 'a'", axis));
             Check(IsNear(Dot(a, b), Dot(b, a)), "dot product must be symmetric");
             const auto cross = Cross(a, b);
             Check(IsNear(Dot(cross, a), 0.0), "cross product must be perpendicular");
             Check(IsNear(Dot(cross, b), 0.0), "cross product must be perpendicular");
           }
         }},

    Test{"zero vector normalization",
         [](std::mt19937 &) {
           const auto result = pbf::Vec3f{0.0f}.Normalize();
           Check(!result.has_value(), "zero vector must not normalize");
           Check(result.error() == pbf::Vec3f::Error::ZeroLength,
                 "normalization must return expected error");
         }},

    Test{"random nonzero vector normalization",
         [](std::mt19937 &random) {
           std::uniform_real_distribution<double> distribution{-10.0, 10.0};
           for ([[maybe_unused]] const int sample : std::views::iota(0, samples)) {
             const pbf::Vector<double, 3> vector{distribution(random), distribution(random),
                                                 distribution(random)};
             const double length = vector.Length();
             if (length == 0.0)
               continue;
             const auto result = vector.Normalize();
             Check(result.has_value(), "nonzero vector must normalize");
             Check(IsNear(result->Length(), 1.0), "normalized vector must have unit length");
             for (const std::size_t axis : std::views::iota(0uz, 3uz))
               Check(IsNear((*result)[axis], vector[axis] / length),
                     std::format("axis {}: normalization must preserve direction", axis));
           }
         }},

    Test{"smoothing kernel radius",
         [](std::mt19937 &) {
           Check(pbf::Poly6(0.0f) > 0.0f, "Poly6 must be positive at origin");
           Check(pbf::Spiky(pbf::Vec3f{0.0f}).LengthSquared() == 0.0f,
                 "Spiky must handle origin without division by zero");

           constexpr std::array distances{pbf::smoothing_radius, 2.0f * pbf::smoothing_radius};
           for (const float distance : distances) {
             const pbf::Vec3f offset{distance, 0.0f, 0.0f};
             Check(pbf::Poly6(offset) == 0.0f, "Poly6 must vanish at and beyond its radius");
             Check(pbf::Spiky(offset).LengthSquared() == 0.0f,
                   "Spiky must vanish at and beyond its radius");
           }
         }},
});

} // namespace

int main() {
  std::println("Running {} tests", tests.size());
  std::size_t failures = 0;
  for (const std::size_t index : std::views::iota(0uz, tests.size())) {
    const auto &test = tests[index];
    std::mt19937 random{static_cast<std::mt19937::result_type>(seed + index)};
    try {
      test.run(random);
      std::println("[PASS] {}", test.name);
    } catch (const std::exception &error) {
      ++failures;
      std::println(stderr, "[FAIL] {}: {}", test.name, error.what());
    } catch (...) {
      ++failures;
      std::println(stderr, "[FAIL] {}: unknown error", test.name);
    }
  }
  std::println("{} passed, {} failed", tests.size() - failures, failures);
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
