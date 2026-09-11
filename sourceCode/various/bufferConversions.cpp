#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace bufferTransform
{
template<class T>
struct TypeTag
{
    using type = T;
};

// Safe integer comparison without signed/unsigned conversion surprises.
template<class Left, class Right>
constexpr bool integerLess(Left left, Right right) noexcept
{
    static_assert(std::is_integral_v<Left>);
    static_assert(std::is_integral_v<Right>);

    if constexpr (std::is_signed_v<Left> == std::is_signed_v<Right>)
    {
        return left < right;
    }
    else if constexpr (std::is_signed_v<Left>)
    {
        if (left < 0)
            return true;

        return static_cast<std::make_unsigned_t<Left>>(left) < right;
    }
    else
    {
        if (right < 0)
            return false;

        return left < static_cast<std::make_unsigned_t<Right>>(right);
    }
}

// Explicit modulo conversion, including signed destinations.
//
// In C++17, an out-of-range conversion to a signed integer is
// implementation-defined. This implements the intended wrapping
// behavior without relying on that conversion.
template<class To, class From>
constexpr To integerWrap(From value) noexcept
{
    static_assert(std::is_integral_v<To>);
    static_assert(std::is_integral_v<From>);

    using UnsignedTo = std::make_unsigned_t<To>;
    using Limits = std::numeric_limits<To>;

    // Conversion to unsigned is well-defined modulo 2^N.
    const UnsignedTo wrapped = static_cast<UnsignedTo>(value);

    if constexpr (std::is_unsigned_v<To>)
    {
        return wrapped;
    }
    else
    {
        static_assert(
            Limits::lowest() == -Limits::max() - 1,
            "Signed wrapping requires a two's-complement integer range.");

        const UnsignedTo signedMaximum =
            static_cast<UnsignedTo>(Limits::max());

        if (wrapped <= signedMaximum)
            return static_cast<To>(wrapped);

        // The distance is always representable by To.
        const UnsignedTo distance =
            static_cast<UnsignedTo>(
                std::numeric_limits<UnsignedTo>::max() - wrapped);

        return static_cast<To>(-1 - static_cast<To>(distance));
    }
}

struct Options
{
    double scale = 1.0;
    double offset = 0.0;
    bool clamp = false;

    bool transform() const
    {
        // Preserve exact 64-bit integer conversions for identity transforms.
        return scale != 1.0 || offset != 0.0;
    }
};

inline std::string allocateBuffer(size_t count, size_t elementSize)
{
    std::string result;
    if (count > result.max_size() / elementSize)
        throw std::length_error("Output buffer is too large.");

    result.resize(count * elementSize);
    return result;
}

template<class T>
T load(const char* source)
{
    T value;
    std::memcpy(&value, source, sizeof(value));
    return value;
}

template<class T>
void store(char* destination, T value)
{
    std::memcpy(destination, &value, sizeof(value));
}

template<class To, class From>
To numericCast(From value, bool clamp)
{
    using Limits = std::numeric_limits<To>;

    if constexpr (std::is_integral_v<From> &&
                  std::is_integral_v<To>)
    {
        if (clamp)
        {
            if (integerLess(value, Limits::lowest()))
                return Limits::lowest();

            if (integerLess(Limits::max(), value))
                return Limits::max();

            // The value is now known to be representable.
            return static_cast<To>(value);
        }

        return integerWrap<To>(value);
    }
    else if constexpr (std::is_integral_v<To>)
    {
        const long double v = static_cast<long double>(value);

        if (std::isnan(v))
            throw std::range_error("Cannot convert NaN to an integer.");

        // Use exact powers of two rather than floating-point versions of
        // INT64_MAX/UINT64_MAX, which might round up on some platforms.
        constexpr int digits = Limits::digits;
        const long double upperExclusive = std::ldexp(1.0L, digits);
        const long double lower =
            std::is_signed_v<To> ? -upperExclusive : 0.0L;

        const long double truncated = std::trunc(v);

        if (truncated < lower)
        {
            if (clamp)
                return Limits::lowest();

            throw std::range_error("Value is below the integer output range.");
        }

        if (truncated >= upperExclusive)
        {
            if (clamp)
                return Limits::max();

            throw std::range_error("Value exceeds the integer output range.");
        }

        return static_cast<To>(truncated);
    }
    else
    {
        const long double v = static_cast<long double>(value);
        const long double maximum =
            static_cast<long double>(Limits::max());

        if (std::isnan(v))
            return static_cast<To>(value);

        if (v > maximum)
        {
            if (clamp)
                return Limits::max();

            // Preserve infinities; reject finite overflow rather than
            // relying on platform-specific narrowing behavior.
            if (std::isinf(v) && Limits::has_infinity)
                return Limits::infinity();

            throw std::range_error("Value exceeds the floating-point output range.");
        }

        if (v < -maximum)
        {
            if (clamp)
                return Limits::lowest();

            if (std::isinf(v) && Limits::has_infinity)
                return -Limits::infinity();

            throw std::range_error("Value is below the floating-point output range.");
        }

        return static_cast<To>(value);
    }
}

template<class To, class From>
To convertValue(From value, const Options& options)
{
    if (!options.transform())
        return numericCast<To>(value, options.clamp);

    // Scaling is floating-point arithmetic and can lose integer precision,
    // depending on the platform's long double representation.
    const long double transformed =
        static_cast<long double>(value) *
            static_cast<long double>(options.scale) +
        static_cast<long double>(options.offset);

    return numericCast<To>(transformed, options.clamp);
}

template<class From, class To>
std::string convertNumeric(
    const std::string& input,
    const Options& options)
{
    if (input.size() % sizeof(From) != 0)
        throw std::invalid_argument(
            "Input buffer contains an incomplete numeric element.");

    const size_t count = input.size() / sizeof(From);
    std::string output = allocateBuffer(count, sizeof(To));

    for (size_t i = 0; i < count; ++i)
    {
        const From value = load<From>(input.data() + i * sizeof(From));
        const To converted = convertValue<To>(value, options);
        store(output.data() + i * sizeof(To), converted);
    }

    return output;
}

// Dispatch a runtime format name to a compile-time numeric type.
template<class Function>
bool dispatchNumeric(const std::string& format, Function&& function)
{
#define DISPATCH_NUMERIC(name, type)          \
    if (format == name)                      \
    {                                        \
        function(TypeTag<type>{});            \
        return true;                         \
    }

    DISPATCH_NUMERIC("uint8",  std::uint8_t)
    DISPATCH_NUMERIC("int8",   std::int8_t)
    DISPATCH_NUMERIC("uint16", std::uint16_t)
    DISPATCH_NUMERIC("int16",  std::int16_t)
    DISPATCH_NUMERIC("uint32", std::uint32_t)
    DISPATCH_NUMERIC("int32",  std::int32_t)
    DISPATCH_NUMERIC("uint64", std::uint64_t)
    DISPATCH_NUMERIC("int64",  std::int64_t)
    DISPATCH_NUMERIC("float",  float)
    DISPATCH_NUMERIC("double", double)

#undef DISPATCH_NUMERIC

    return false;
}

struct PixelFormat
{
    size_t size = 0;
    int red = 0;
    int green = 1;
    int blue = 2;
    int alpha = -1;
};

inline PixelFormat pixelFormat(const std::string& format)
{
    if (format == "rgb")  return {3, 0, 1, 2, -1};
    if (format == "bgr")  return {3, 2, 1, 0, -1};
    if (format == "rgba") return {4, 0, 1, 2,  3};
    if (format == "bgra") return {4, 2, 1, 0,  3};
    if (format == "argb") return {4, 1, 2, 3,  0};

    return {};
}

inline std::string convertPixels(
    const std::string& input,
    PixelFormat from,
    PixelFormat to,
    const Options& options)
{
    if (input.size() % from.size != 0)
        throw std::invalid_argument(
            "Input buffer contains an incomplete pixel.");

    const size_t count = input.size() / from.size;
    std::string output = allocateBuffer(count, to.size);

    for (size_t i = 0; i < count; ++i)
    {
        const char* source = input.data() + i * from.size;
        char* destination = output.data() + i * to.size;

        const auto copyChannel = [&](int sourceIndex, int destinationIndex)
        {
            const auto value = load<std::uint8_t>(source + sourceIndex);
            store(destination + destinationIndex,
                  convertValue<std::uint8_t>(value, options));
        };

        copyChannel(from.red, to.red);
        copyChannel(from.green, to.green);
        copyChannel(from.blue, to.blue);

        if (to.alpha >= 0)
        {
            if (from.alpha >= 0)
                copyChannel(from.alpha, to.alpha);
            else
                store(destination + to.alpha, std::uint8_t{255});
        }
    }

    return output;
}

inline std::string expandGrayscale(const std::string& input)
{
    std::string output = allocateBuffer(input.size(), 3);

    for (size_t i = 0; i < input.size(); ++i)
    {
        output[3 * i] = input[i];
        output[3 * i + 1] = input[i];
        output[3 * i + 2] = input[i];
    }

    return output;
}

inline std::string pixelsToGrayscale(
    const std::string& input,
    PixelFormat from,
    const Options& options)
{
    if (input.size() % from.size != 0)
        throw std::invalid_argument(
            "Input buffer contains an incomplete pixel.");

    const size_t count = input.size() / from.size;
    std::string output = allocateBuffer(count, 1);

    for (size_t i = 0; i < count; ++i)
    {
        const char* source = input.data() + i * from.size;

        const unsigned sum =
            unsigned(load<std::uint8_t>(source + from.red)) +
            unsigned(load<std::uint8_t>(source + from.green)) +
            unsigned(load<std::uint8_t>(source + from.blue));

        const auto gray =
            convertValue<std::uint8_t>(
                static_cast<long double>(sum) / 3.0L, options);

        store(output.data() + i, gray);
    }

    return output;
}

inline std::string transform(
    const std::string& input,
    const std::string& from,
    const std::string& to,
    const Options& options)
{
    // Base64 operates on raw bytes, not numeric values.
    // Validation behavior is delegated to your existing codec.
    if (from == "uint8" && to == "base64")
        return utils::encode64(input);

    if (from == "base64" && to == "uint8")
        return utils::decode64(input);

    const PixelFormat sourcePixel = pixelFormat(from);
    const PixelFormat destinationPixel = pixelFormat(to);

    // Supports all requested pixel pairs, plus other combinations
    // of the recognized pixel formats.
    if (sourcePixel.size != 0 && destinationPixel.size != 0)
        return convertPixels(input, sourcePixel, destinationPixel, options);

    // Preserve existing RGB/BGR -> uint8 grayscale conversion.
    if ((from == "rgb" || from == "bgr") && to == "uint8")
        return pixelsToGrayscale(input, sourcePixel, options);

    std::string output;
    bool converted = false;

    dispatchNumeric(from, [&](auto sourceTag)
    {
        using From = typename decltype(sourceTag)::type;

        converted = dispatchNumeric(to, [&](auto destinationTag)
        {
            using To = typename decltype(destinationTag)::type;
            output = convertNumeric<From, To>(input, options);
        });

        // Preserve float/double/uint8 -> RGB grayscale expansion.
        // This also permits the other numeric input types.
        if (!converted && to == "rgb")
        {
            output = expandGrayscale(
                convertNumeric<From, std::uint8_t>(input, options));
            converted = true;
        }
    });

    if (!converted)
        throw std::invalid_argument(
            "Unsupported conversion: " + from + " -> " + to + ".");

    return output;
}
} // namespace bufferTransform
