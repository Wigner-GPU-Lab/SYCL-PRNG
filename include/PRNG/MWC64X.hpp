// Copyright(c) 2011, 2013 Mutsuo Saito, Makoto Matsumoto, Hiroshima University and The University of Tokyo.
//
// Copyright(c) 2018 Máté Ferenc Nagy-Egri, Wigner GPU-Laboratory.
//
// All rights reserved.
//
// The 3-clause BSD License is applied to this software, see LICENSE.txt
//

#pragma once

#include <cstddef>  // std::size_t
#include <cstdint>  // std::uint64_t
#include <limits>   // std::numeric_limits::min,max

namespace prng
{
    template <std::uint32_t A, std::uint64_t M>
    class multiply_with_carry_engine_32
    {
    public:

        using result_type = std::uint32_t;

        static constexpr std::size_t word_size = 32;
        static constexpr std::size_t state_size = 2;
        static constexpr result_type mask = 0xffffffff;

        static constexpr result_type default_seed = 5489u;

        multiply_with_carry_engine_32(result_type value) { seed(value); }

        template <typename Sseq> explicit multiply_with_carry_engine_32(Sseq& s);

        multiply_with_carry_engine_32() : multiply_with_carry_engine_32(default_seed) {}
        multiply_with_carry_engine_32(const multiply_with_carry_engine_32&) = default;

        void seed(result_type value = default_seed)
        {
        }
        template <typename Sseq> void seed(Sseq& s);

        result_type operator()()
        {
        }

        void discard(unsigned long long z) { for (; 0 < z; --z) (*this)(); }

        friend bool operator==(const multiply_with_carry_engine_32<A, M>& lhs,
                               const multiply_with_carry_engine_32<A, M>& rhs)
        {
            return (lhs.x == rhs.x) &&
                   (lhs.c == rhs.c);
        }

        friend bool operator!=(const multiply_with_carry_engine_32<A, M>& lhs,
                               const multiply_with_carry_engine_32<A, M>& rhs)
        {
            return (lhs.x != rhs.x) ||
                   (lhs.c != rhs.c);
        }

        static constexpr result_type min() { return std::numeric_limits<result_type>::min(); }
        static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }

    private:

        result_type x, c;

        // Convenience renames
        static constexpr auto a = A;
        static constexpr auto m = M;

        inline void next_state()
        {
            std::uint32_t xn = a * x + c;
            std::uint32_t carry = static_cast<std::uint32_t>(xn < c); // The (Xn<C) will be zero or one for scalar
            std::uint32_t cn = mad_hi(a, x, carry);

            x = xn;
            c = cn;
        }
    };

    using mwc64x = multiply_with_carry_engine_32<4294883355u, 18446383549859758079ul>;
}