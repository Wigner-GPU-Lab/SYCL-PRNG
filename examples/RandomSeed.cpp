// SYCL-PRNG includes
#include <PRNG/TinyMT.hpp>
#include <PRNG/MWC64X.hpp>

// Standard C++ includes
#include <random>
#include <vector>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <typeinfo>


int main()
{
    static_assert(std::is_standard_layout<prng::tinymt_64>::value, "TinyMT 64 is not standard layout.");
    static_assert(std::is_standard_layout<prng::tinymt_32>::value, "TinyMT 32 is not standard layout.");
    static_assert(std::is_standard_layout<prng::mwc64x_32>::value, "MWC64X 32 is not standard layout.");

    std::vector<prng::tinymt_64> tmt64{ prng::tinymt_64{} }; // default CTOR
    std::vector<prng::tinymt_32> tmt32{ prng::tinymt_32{} }; // default CTOR
    std::vector<prng::mwc64x_32> mwc32{ prng::mwc64x_32{} }; // default CTOR

    {
        std::random_device rd;
        std::generate_n(std::back_inserter(tmt64), 10, [&]() { return prng::tinymt_64{ rd() }; }); // random seeding
        std::generate_n(std::back_inserter(tmt32), 10, [&]() { return prng::tinymt_32{ rd() }; }); // random seeding
        std::generate_n(std::back_inserter(mwc32), 10, [&]() { return prng::mwc64x_32{ rd() }; }); // random seeding
    }

    std::vector<prng::tinymt_64> tmt64_ref( tmt64.cbegin(), tmt64.cend() ); // copy CTOR
    std::vector<prng::tinymt_32> tmt32_ref( tmt32.cbegin(), tmt32.cend() ); // copy CTOR
    std::vector<prng::mwc64x_32> mwc32_ref( mwc32.cbegin(), mwc32.cend() ); // copy CTOR

    auto skip_function_discard = [](auto prng, const std::int64_t& distance) // copy CTOR
    {
        prng.discard(distance); // discard
        return prng;
    };
    auto next_stepping_discard = [](auto prng, const std::int64_t& distance) // copy CTOR
    {
        for (std::int64_t i = 0; i < distance; ++i) prng(); // operator()
        return prng;
    };

    std::default_random_engine re;
    std::uniform_int_distribution<std::int64_t> dist{ 9'000, 11'000 };

    std::vector<std::int64_t> distances;
    std::generate_n(std::back_inserter(distances), tmt64.size(), [&]() { return dist(re); });

    auto match_skip_vs_step = [&](auto& skip_seq, auto& step_seq)
    {
        std::transform(skip_seq.cbegin(), skip_seq.cend(), distances.cbegin(), skip_seq.begin(), skip_function_discard);
        std::transform(step_seq.cbegin(), step_seq.cend(), distances.cbegin(), step_seq.begin(), next_stepping_discard);

        auto iters = std::mismatch(skip_seq.cbegin(), skip_seq.cend(), step_seq.cbegin()); // operator==

        if (iters.first  != skip_seq.cend() ||
            iters.second != step_seq.cend())
        {
            std::cerr << "Skip vs. step differs for " <<
                typeid(skip_seq.at(0)).name() <<
                " when discarding " <<
                distances.at(std::distance(skip_seq.cbegin(), iters.first)) <<
                " elements." <<
                std::endl;

            std::exit(EXIT_FAILURE);
        }
    };

    match_skip_vs_step(tmt64, tmt64_ref);
    match_skip_vs_step(tmt32, tmt32_ref);
    match_skip_vs_step(mwc32, mwc32_ref);

    return 0;
}
