#pragma once

#include <cstddef>  // std::size_t
#include <cstdint>  // std::uint64_t
#include <limits>   // std::numeric_limits::min,max

namespace prng
{
    template <std::uint32_t Mat1, std::uint32_t Mat2, std::uint64_t TMat>
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

        tiny_mersenne_twister_engine_64(result_type value)
        {
            state_[0] = value ^ ((result_type)mat1 << 32);
            state_[1] = mat2 ^ tmat;
            for (int i = 1; i < min_loop; i++)
            {
                state_[i & 1] ^=
                    i + UINT64_C(6364136223846793005) *
                    (state_[(i - 1) & 1] ^ (state_[(i - 1) & 1] >> 62));
            }
        }

        template <typename Sseq> explicit tiny_mersenne_twister_engine_64(Sseq& s);

        tiny_mersenne_twister_engine_64() : tiny_mersenne_twister_engine_64(default_seed) {}
        tiny_mersenne_twister_engine_64(const tiny_mersenne_twister_engine_64&) = default;

        void seed(result_type value = default_seed);
        template <typename Sseq> void seed(Sseq& s);

        result_type operator()()
        {
            next_state();

            return temper();
        }

        void discard(unsigned long long z) { for (; 0 < z; --z) this->operator(); }

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
    };

    using tinymt_64 = tiny_mersenne_twister_engine_64<12, 11, 8>;
}
