module;

#include <array>
#include <concepts>
#include <type_traits>

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
    /*! @brief Vector elements. */
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
     * @param args Arguments.
     */
    template<typename... Args>
        requires (sizeof...(Args) == N)
    Vector(const Args... args) noexcept : elements{static_cast<T>(args)...} {}

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
};

} // namespace math
