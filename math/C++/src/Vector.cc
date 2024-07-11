module;

#include <array>
#include <concepts>
#include <execution>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <type_traits>

#include <cmath>

export module Vector;

export namespace math {

/**
 * @brief Arithmetic concept.
 * @tparam T Type to check.
 */
template<typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

/**
 * @brief Vector class.
 * @tparam T Arithmetic type of elements.
 * @tparam N Positive number of elements.
 */
template<Arithmetic T, std::integral auto N>
    requires (N > 0)
class Vector {
private:
    /*! @brief Elements. */
    std::array<T, N> elements;
public:
    /*! @brief Default constructor. */
    Vector() noexcept = default;

    /*! @brief Copy constructor. */
    Vector(const Vector&) noexcept = default;

    /*! @brief Move constructor. */
    Vector(Vector&&) noexcept = default;

    /*! @brief Destructor. */
    ~Vector() noexcept = default;

    /*! @brief Copy constructor that accepts array. */
    Vector(const std::array<T, N>& array) noexcept : elements{array} {}

    /*! @brief Move constructor that accepts array. */
    Vector(std::array<T, N>&& array) noexcept : elements{std::move(array)} {}

    /**
     * @brief Constructor that accepts variadic number of arguments.
     * @param arguments Arguments.
     */
    template<typename... Args>
        requires (sizeof...(Args) == N)
    Vector(const Args... arguments) noexcept :
        elements{static_cast<T>(arguments)...} {}

    /*! @brief Copy assignment operator. */
    inline Vector& operator=(const Vector&) noexcept = default;

    /*! @brief Move assignment operator. */
    inline Vector& operator=(Vector&&) noexcept = default;

    /*! @brief Subscript operator. */
    inline auto operator[](const std::integral auto index) const noexcept {
        return elements[index];
    }

    /*! @brief Subscript operator. */
    inline auto& operator[](const std::integral auto index) noexcept {
        return elements[index];
    }

    /*! @brief Iterator wrapper. */
    inline auto begin() const noexcept { return elements.begin(); }

    /*! @brief Iterator wrapper. */
    inline auto begin() noexcept { return elements.begin(); }

    /*! @brief Iterator wrapper. */
    inline auto end() const noexcept { return elements.end(); }

    /*! @brief Iterator wrapper. */
    inline auto end() noexcept { return elements.end(); }

    /**
     * @brief Compute length.
     * @return Length.
     */
    inline auto Length() const {
        return std::sqrt(Length2());
    }

    /**
     * @brief Compute squared length.
     * @return Squared length.
     */
    inline auto Length2() const {
        return std::transform_reduce(std::execution::par_unseq,
            this->begin(), this->end(), T{0},
            std::plus<>{}, [](const auto value) { return value * value; });
    }

    /**
     * @brief Compute dot product with another vector.
     * @param vector Vector.
     * @return Dot product with another vector.
     */
    inline auto Dot(const Vector& vector) const {
        return std::transform_reduce(std::execution::par_unseq,
            this->begin(), this->end(), vector.begin(), T{0},
            std::plus<>{}, std::multiplies<>{});
    }

    /**
     * @brief Compute cross product with another vector.
     * @param vector Vector.
     * @return Cross product with another vector.
     */
    inline auto Cross(const Vector& vector) const noexcept {
        static_assert(N == 3);
        return Vector(elements[1] * vector[2] - elements[2] * vector[1],
                      elements[2] * vector[0] - elements[0] * vector[2],
                      elements[0] * vector[1] - elements[1] * vector[0]);
    }

    /**
     * @brief Normalize.
     * @return Normalized vector.
     */
    inline auto Normalize() const { return *this / Length(); }

    /**
     * @brief Reflect.
     * @param normal Normal vector.
     * @return Reflected vector.
     */
    inline auto Reflect(const Vector& normal) const {
        static_assert(N == 3);
        return *this - T{2} * Dot(normal) * normal;
    }

    /**
     * @brief Refract.
     * @param normal Normal vector.
     * @param ratio Refraction ratio.
     * @return Refracted vector.
     */
    auto Refract(const Vector& normal, const T ratio) const {
        static_assert(N == 3);
        const auto cos_theta = std::min((-*this).Dot(normal), T{1});
        const auto perpendicular = ratio * (*this + cos_theta * normal);
        const auto parallel =
            -std::sqrt(std::abs(T{1} - perpendicular.Length2())) * normal;
        return perpendicular + parallel;
    }

    /*! @brief Unary - operator. */
    inline Vector operator-() const {
        Vector result;
        std::transform(std::execution::par_unseq,
            this->begin(), this->end(), result.begin(),
            [](const auto value) { return -value; });
        return result;
    }

    /*! @brief += operator. */
    inline Vector& operator+=(const Vector& vector) {
        std::transform(std::execution::par_unseq,
            vector.begin(), vector.end(), this->begin(), this->begin(),
            [](const auto left, const auto right){ return left + right; });
        return *this;
    }

    /*! @brief -= operator. */
    inline Vector& operator-=(const Vector& vector) {
        std::transform(std::execution::par_unseq,
            vector.begin(), vector.end(), this->begin(), this->begin(),
            [](const auto left, const auto right){ return left - right; });
        return *this;
    }

    /*! @brief *= operator. */
    inline Vector& operator*=(const Vector& vector) {
        std::transform(std::execution::par_unseq,
            vector.begin(), vector.end(), this->begin(), this->begin(),
            [](const auto left, const auto right){ return left * right; });
        return *this;
    }

    /*! @brief *= operator. */
    template<Arithmetic U>
    inline Vector& operator*=(const U number) {
        const auto multiplier = static_cast<T>(number);
        std::transform(std::execution::par_unseq,
            this->begin(), this->end(), this->begin(),
            [multiplier](const auto value) { return value * multiplier; });
        return *this;
    }

    /*! @brief /= operator. */
    template<Arithmetic U>
    inline Vector& operator/=(const U number) {
        return *this *= T{1} / static_cast<T>(number);
    }

    /*! @brief + operator. */
    friend inline Vector operator+(const Vector& left, const Vector& right) {
        Vector result;
        std::transform(std::execution::par_unseq,
            left.begin(), left.end(), right.begin(), result.begin(),
            [](const auto left, const auto right) { return left + right; });
        return result;
    }

    /*! @brief - operator. */
    friend inline Vector operator-(const Vector& left, const Vector& right) {
        Vector result;
        std::transform(std::execution::par_unseq,
            left.begin(), left.end(), right.begin(), result.begin(),
            [](const auto left, const auto right) { return left - right; });
        return result;
    }

    /*! @brief * operator. */
    friend inline Vector operator*(const Vector& left, const Vector& right) {
        Vector result;
        std::transform(std::execution::par_unseq,
            left.begin(), left.end(), right.begin(), result.begin(),
            [](const auto left, const auto right) { return left * right; });
        return result;
    }

    /*! @brief * operator. */
    template<Arithmetic U>
    friend inline Vector operator*(const Vector& vector, const U number) {
        const auto multiplier = static_cast<T>(number);
        Vector result;
        std::transform(std::execution::par_unseq,
            vector.begin(), vector.end(), result.begin(),
            [multiplier](const auto value) { return value * multiplier; });
        return result;
    }

    /*! @brief * operator. */
    template<Arithmetic U>
    friend inline Vector operator*(const U number, const Vector& vector) {
        return vector * number;
    }

    /*! @brief / operator. */
    template<Arithmetic U>
    friend inline Vector operator/(const Vector& vector, const U number) {
        return vector * (T{1} / static_cast<T>(number));
    }

    /*! @brief / operator. */
    template<Arithmetic U>
    friend inline Vector operator/(const U number, const Vector& vector) {
        return vector / number;
    }

    /*! @brief Output stream << operator. */
    friend std::ostream& operator<<(std::ostream& out, const Vector& vector) {
        out << '[';
        for (const auto& value : vector)
            out << std::setw(10) << std::setprecision(4) << std::fixed << value;
        return out << ']';
    }
};

/**
 * @brief Compute length of vector.
 * @tparam T Type of elements.
 * @tparam N Number of elements.
 * @param vector Vector.
 * @return Length.
 */
template<Arithmetic T, std::integral auto N>
    requires (N > 0)
inline auto Length(const Vector<T, N>& vector) { vector.Length(); }

/**
 * @brief Compute squared length of vector.
 * @tparam T Type of elements.
 * @tparam N Number of elements.
 * @param vector Vector.
 * @return Squared length.
 */
template<Arithmetic T, std::integral auto N>
    requires (N > 0)
inline auto Length2(const Vector<T, N>& vector) { return vector.Length2(); }

/**
 * @brief Compute dot product of vectors.
 * @tparam T Type of elements.
 * @tparam N Number of elements.
 * @param left First vector.
 * @param right Second vector.
 * @return Dot product.
 */
template<Arithmetic T, std::integral auto N>
    requires (N > 0)
inline auto Dot(const Vector<T, N>& left, const Vector<T, N>& right) {
    return left.Dot(right);
}

/**
 * @brief Compute cross product of two vectors.
 * @tparam T Type of elements.
 * @tparam N Number of elements.
 * @param left First vector.
 * @param right Second vector.
 * @return Cross product.
 */
template<Arithmetic T, std::integral auto N>
    requires (N == 3)
inline auto Cross(const Vector<T, N>& left, const Vector<T, N>& right) {
    return left.Cross(right);
}

/**
 * @brief Normalize vector.
 * @tparam T Type of elements.
 * @tparam N Number of elements.
 * @param vector Vector.
 * @return Normalized vector.
 */
template<Arithmetic T, std::integral auto N>
    requires (N > 0)
inline auto Normalize(const Vector<T, N>& vector) {
    return vector.Normalize();
}

/**
 * @brief Reflect vector.
 * @tparam T Type of elements.
 * @tparam N Number of elements.
 * @param vector Vector.
 * @param normal Normal vector.
 * @return Reflected vector.
 */
template<Arithmetic T, std::integral auto N>
    requires (N == 3)
inline auto Reflect(const Vector<T, N>& vector, const Vector<T, N>& normal) {
    return vector.Reflect(normal);
}

/**
 * @brief Refract vector.
 * @tparam T Type of elements.
 * @tparam N Number of elements.
 * @param vector Vector.
 * @param normal Normal vector.
 * @param ratio Refraction ratio.
 * @return Refracted vector.
 */
template<Arithmetic T, std::integral auto N>
    requires (N == 3)
auto Refract(const Vector<T, N>& vector, const Vector<T, N>& normal,
    const T ratio) {
    return vector.Refract(normal, ratio);
}

} // namespace math
