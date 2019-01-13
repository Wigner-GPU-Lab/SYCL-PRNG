// Copyright(c) 2011, 2013 Mutsuo Saito, Makoto Matsumoto, Hiroshima University and The University of Tokyo.
//
// Copyright(c) 2018 Máté Ferenc Nagy-Egri, Wigner GPU-Laboratory.
//
// All rights reserved.
//
// The 3-clause BSD License is applied to this software, see LICENSE.txt
//

#pragma once

// SYCL-PRNG includes
#include <PRNG/concepts/SeedSequence.hpp>

// Standard C++ includes
#include <cstddef>  // std::size_t
#include <cstdint>  // std::uint64_t
#include <limits>   // std::numeric_limits::min,max

namespace prng
{
    template <std::uint32_t Mat1, std::uint32_t Mat2, std::uint64_t TMat, std::uint64_t Poly1, std::uint64_t Poly2>
    class tiny_mersenne_twister_engine_64
    {
    public:

        using result_type = std::uint64_t;

        static constexpr std::size_t word_size = 64;
        static constexpr std::size_t state_size = 2;
        static constexpr result_type mask = 0x7fffffffffffffff;
        static constexpr std::size_t sh0 = 12;
        static constexpr std::size_t sh1 = 11;
        static constexpr std::size_t sh8 = 8;

        static constexpr result_type default_seed = 5489u;

        tiny_mersenne_twister_engine_64(result_type value) { seed(value); }

        //template <typename Sseq, typename std::enable_if<concepts::SeedSequence<Sseq> and not concepts::ConvertibleTo<Sseq, result_type>, tiny_mersenne_twister_engine_64>::type = 0>
        //explicit tiny_mersenne_twister_engine_64(Sseq& s) { seed(s); }

        tiny_mersenne_twister_engine_64() : tiny_mersenne_twister_engine_64(default_seed) {}
        tiny_mersenne_twister_engine_64(const tiny_mersenne_twister_engine_64&) = default;

        void seed(result_type value = default_seed)
        {
            state_[0] = value ^ ((result_type)mat1 << 32);
            state_[1] = mat2 ^ tmat;

            for (int i = 1; i < min_loop; i++)
            {
                state_[i & 1] ^=
                    i + static_cast<result_type>(6364136223846793005) *
                    (state_[(i - 1) & 1] ^ (state_[(i - 1) & 1] >> 62));
            }
        }
        //template <typename Sseq, typename std::enable_if<concepts::SeedSequence<Sseq> and not concepts::ConvertibleTo<Sseq, result_type>, tiny_mersenne_twister_engine_64>::type = 0>
        //void seed(Sseq& s) { s.generate(state_, state_ + state_size); }

        result_type operator()()
        {
            next_state();

            return temper();
        }

        void discard(unsigned long long z) { for (; 0 < z; --z) (*this)(); }

        friend bool operator==(const tiny_mersenne_twister_engine_64<Mat1, Mat2, TMat>& lhs,
            const tiny_mersenne_twister_engine_64<Mat1, Mat2, TMat>& rhs)
        {
            return (lhs.state_[0] == rhs.state_[0]) &&
                   (lhs.state_[1] == rhs.state_[1]);
        }

        friend bool operator!=(const tiny_mersenne_twister_engine_64<Mat1, Mat2, TMat>& lhs,
            const tiny_mersenne_twister_engine_64<Mat1, Mat2, TMat>& rhs)
        {
            return (lhs.state_[0] != rhs.state_[0]) ||
                   (lhs.state_[1] != rhs.state_[1]);
        }

        static constexpr result_type min() { return std::numeric_limits<result_type>::min(); }
        static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }

    private:

        result_type state_[state_size];

        // Convenience renames
        static constexpr auto mat1 = Mat1;
        static constexpr auto mat2 = Mat2;
        static constexpr auto tmat = TMat;
        static constexpr auto poly1 = Poly1;
        static constexpr auto poly2 = Poly2;

        // Non-customizable params
        static constexpr int min_loop = 8;

        inline void next_state()
        {
            state_[0] &= mask;

            result_type x = state_[0] ^ state_[1];
            x ^= x << sh0;
            x ^= x >> 32;
            x ^= x << 32;
            x ^= x << sh1;

            state_[0] = state_[1];
            state_[1] = x;

            state_[0] ^= -((std::int64_t)(x & 1)) & mat1;
            state_[1] ^= -((std::int64_t)(x & 1)) & (((result_type)mat2) << 32);
        }
        inline result_type temper()
        {
            uint64_t x = state_[0] + state_[1];

            x ^= state_[0] >> sh8;
            x ^= -((std::int64_t)(x & 1)) & tmat;

            return x;
        }
        
        /// <summary>Polynomial over F<sub>2</sub>, whose degree is equal to or less than 128.</summary>
        /// <note>LSB of ar[0], i.e. ar[0] & 1, represent constant</note>
        ///
        class f2_polynomial_128
        {
        public:

            f2_polynomial_128(std::uint64_t low, std::uint64_t high = 0)
            {
                f2_polynomial_128 tee{ 2, 0 };

                polynomial_power_mod(tee,
                                     lower_step,
                                     upper_step);
            }

            f2_polynomial_128(std::initializer_list<std::uint64_t> init) : ar_{ init } {}

        private:

            std::array<std::uint64_t, 2> ar_;

            /// <summary>Polynomial over F<sub>2</sub>, whose degree is equal to or less than 256.</summary>
            /// <note>LSB of ar[0], i.e. ar[0] & 1, represent constant</note>
            ///
            class f2_polynomial_256
            {
            public:

                f2_polynomial_256(const f2_polynomial_128 poly) : ar_{ poly[0], poly[1], 0, 0 } {}

            private:

                std::array<std::uint64_t, 4> ar_;
            };

            void polynomial_power_mod(const f2_polynomial_128 x, std::uint64_t low, std::uint64_t high)
            {
                f2_polynomial_256 tmp{ x };
                f2_polynomial_256 lmod{ *this };
                f2_polynomial_256 result{ 1, 0, 0, 0 };

                for (std::int_fast8_t i = 0; i < 64; i++) {
                    if ((low & 1) != 0) {
                        mul_pol(result, tmp);
                        mod_f2_polynomial_256(result, lmod);
                    }
                    square_f2_polynomial_256(tmp);
                    mod_f2_polynomial_256(tmp, lmod);
                    lower_power = lower_power >> 1;
                    if ((lower_power == 0) && (upper_power == 0)) {
                        break;
                    }
                }
                while (upper_power != 0) {
                    if ((upper_power & 1) != 0) {
                        mul_pol(result, tmp);
                        mod_f2_polynomial_256(result, lmod);
                    }
                    square_f2_polynomial_256(tmp);
                    mod_f2_polynomial_256(tmp, lmod);
                    upper_power = upper_power >> 1;
                }
                topol(dest, result);
                return;
            }
        };
        
        void tinymt64_jump(std::uint64_t lower_step,
                           std::uint64_t upper_step,
                           const char * poly_str);
        void tinymt64_jump_by_polynomial(f2_polynomial * jump_poly);
    };

    template <std::uint32_t Mat1, std::uint32_t Mat2, std::uint32_t TMat>
    class tiny_mersenne_twister_engine_32
    {
    public:

        using result_type = std::uint32_t;

        static constexpr std::size_t word_size = 32;
        static constexpr std::size_t state_size = 4;
        static constexpr result_type mask = 0x7fffffff;
        static constexpr std::size_t sh0 = 1;
        static constexpr std::size_t sh1 = 10;
        static constexpr std::size_t sh8 = 8;

        static constexpr result_type default_seed = 5489u;

        tiny_mersenne_twister_engine_32(result_type value)
        {
            state_[0] = value;
            state_[1] = mat1;
            state_[2] = mat2;
            state_[3] = tmat;

            for (int i = 1; i < min_loop; i++)
            {
                state_[i & 3] ^= i + UINT32_C(1812433253) *
                    (state_[(i - 1) & 3] ^ (state_[(i - 1) & 3] >> 30));
            }

            for (int i = 0; i < pre_loop; i++) next_state();
        }

        //template <typename Sseq> explicit tiny_mersenne_twister_engine_32(Sseq& s);

        tiny_mersenne_twister_engine_32() : tiny_mersenne_twister_engine_32(default_seed) {}
        tiny_mersenne_twister_engine_32(const tiny_mersenne_twister_engine_32&) = default;

        void seed(result_type value = default_seed);
        //template <typename Sseq> void seed(Sseq& s);

        result_type operator()()
        {
            next_state();

            return temper();
        }

        void discard(unsigned long long z) { for (; 0 < z; --z) (*this)(); }

        friend bool operator==(const tiny_mersenne_twister_engine_32<Mat1, Mat2, TMat>& lhs,
                               const tiny_mersenne_twister_engine_32<Mat1, Mat2, TMat>& rhs)
        {
            return (lhs.state_[0] == rhs.state_[0]) &&
                   (lhs.state_[1] == rhs.state_[1]) &&
                   (lhs.state_[2] == rhs.state_[2]) &&
                   (lhs.state_[3] == rhs.state_[3]);
        }

        friend bool operator!=(const tiny_mersenne_twister_engine_32<Mat1, Mat2, TMat>& lhs,
                               const tiny_mersenne_twister_engine_32<Mat1, Mat2, TMat>& rhs)
        {
            return (lhs.state_[0] != rhs.state_[0]) ||
                   (lhs.state_[1] != rhs.state_[1]) ||
                   (lhs.state_[2] != rhs.state_[2]) ||
                   (lhs.state_[3] != rhs.state_[3]);
        }

        static constexpr result_type min() { return std::numeric_limits<result_type>::min(); }
        static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }

    private:

        result_type state_[state_size];

        // Convenience renames
        static constexpr auto mat1 = Mat1;
        static constexpr auto mat2 = Mat2;
        static constexpr auto tmat = TMat;

        // Non-customizable params
        static constexpr int min_loop = 8;
        static constexpr int pre_loop = 8;

        inline void next_state()
        {
            result_type x = (state_[0] & mask) ^ state_[1] ^ state_[2],
                        y = state_[3];

            x ^= (x << sh0);
            y ^= (y >> sh0) ^ x;

            state_[0] = state_[1];
            state_[1] = state_[2];
            state_[2] = x ^ (y << sh1);
            state_[3] = y;
            state_[1] ^= -((std::int32_t)(y & 1)) & mat1;
            state_[2] ^= -((std::int32_t)(y & 1)) & mat2;
        }
        inline result_type temper()
        {
            result_type t0 = state_[3],
                        t1 = state_[0] + (state_[2] >> sh8);

            t0 ^= t1;
            t0 ^= -((std::int32_t)(t1 & 1)) & tmat;

            return t0;
        }
    };

    using tinymt_64 = tiny_mersenne_twister_engine_64<0xd02f1a04, 0xfe80ffa0, 0x71126defef7e7ffa>; // tinymt64dc --count 1 1
    using tinymt_32 = tiny_mersenne_twister_engine_32<0xda251b45, 0xfed0ffb5, 0x9b5cf7ff>;         // tinymt32dc --count 1 1
}
