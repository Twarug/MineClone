#pragma once

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

namespace mc
{
    // ReSharper disable CppInconsistentNaming
    using u8 = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;

    using i8 = int8_t;
    using i16 = int16_t;
    using i32 = int32_t;
    using i64 = int64_t;

    using f32 = float_t;
    using f64 = double_t;

    template <typename T, int D>
    using Vec = glm::vec<D, T>;

    template <typename T>
    using Vec2 = Vec<T, 2>;
    template <typename T>
    using Vec3 = Vec<T, 3>;
    template <typename T>
    using Vec4 = Vec<T, 4>;

    using float2 = Vec2<f32>;
    using float3 = Vec3<f32>;
    using float4 = Vec4<f32>;

    using double2 = Vec2<f64>;
    using double3 = Vec3<f64>;
    using double4 = Vec4<f64>;


    using short2 = Vec2<i16>;
    using short3 = Vec3<i16>;
    using short4 = Vec4<i16>;

    using ushort2 = Vec2<u16>;
    using ushort3 = Vec3<u16>;
    using ushort4 = Vec4<u16>;

    using int2 = Vec2<i32>;
    using int3 = Vec3<i32>;
    using int4 = Vec4<i32>;

    using uint2 = Vec2<u32>;
    using uint3 = Vec3<u32>;
    using uint4 = Vec4<u32>;

    using long2 = Vec2<i64>;
    using long3 = Vec3<i64>;
    using long4 = Vec4<i64>;

    using ulong2 = Vec2<u64>;
    using ulong3 = Vec3<u64>;
    using ulong4 = Vec4<u64>;
    // ReSharper restore CppInconsistentNaming

    template <int W, int H>
    using Mat = glm::mat<W, H, f32>;

    using Mat3 = Mat<3, 3>;
    using Mat4 = Mat<4, 4>;


    using Color = float4;
    using Color32 = Vec4<u8>;
}


// ReSharper disable once CppInconsistentNaming
template <typename T, int D, typename CharT>
struct std::formatter<mc::Vec<T, D>, CharT> : std::formatter<T, CharT>
{
    template <typename FormatContext>
    constexpr format_context::iterator format(const mc::Vec<T, D>& val, FormatContext& ctx) const {
        auto&& out = ctx.out();
        format_to(out, "(");
        if(D > 0)
            formatter<T, CharT>::format(val[0], ctx);
        for(int i = 1; i < D; ++i) {
            format_to(ctx.out(), ", ");
            formatter<T, CharT>::format(val[i], ctx);
        }
        return format_to(out, ")");
    }
};

// ReSharper disable once CppInconsistentNaming
template <int W, int H, typename CharT>
struct std::formatter<mc::Mat<W, H>, CharT> : std::formatter<mc::Vec<mc::f32, H>, CharT>
{
    template <typename FormatContext>
    constexpr format_context::iterator format(const mc::Mat<W, H>& val, FormatContext& ctx) const {
        auto&& out = ctx.out();
        format_to(out, "[");

        if(W > 0)
            formatter<mc::Vec<mc::f32, H>, CharT>::format(val[0], ctx);
        for(int x = 1; x < W; x++) {
            format_to(out, ", ");
            formatter<mc::Vec<mc::f32, H>, CharT>::format(val[x], ctx);
        }

        return format_to(out, "]");
    }
};
