// SYCL-PRNG includes
#include <PRNG/TinyMT.hpp>

// Standard C++ includes
#include <random>
#include <vector>
#include <iterator>
#include <algorithm>
#include <iostream>


int main()
{
    static_assert(std::is_standard_layout<prng::tinymt_64>::value, "TinyMT 64 is not standard layout.");
    static_assert(std::is_standard_layout<prng::tinymt_32>::value, "TinyMT 32 is not standard layout.");

    std::vector<prng::tinymt_64> engines64{ prng::tinymt_64{} }; // default CTOR
    std::vector<prng::tinymt_32> engines32{ prng::tinymt_32{} }; // default CTOR

    {
        std::random_device rd;
        std::generate_n(std::back_inserter(engines64),
                        10,
                        [&]() { return prng::tinymt_64{ rd() }; }); // random seeding

        std::generate_n(std::back_inserter(engines32),
                        10,
                        [&]() { return prng::tinymt_32{ rd() }; }); // random seeding
    }

    auto it64 = std::find(engines64.cbegin(), engines64.cend(), prng::tinymt_64{}); // operator==
    auto it32 = std::find(engines32.cbegin(), engines32.cend(), prng::tinymt_32{}); // operator==

    prng::tinymt_64 a64;
    prng::tinymt_32 a32;
    std::uniform_real_distribution<> d;

    std::generate_n(std::ostream_iterator<double>(std::cout, "\n"),
                    10,
                    [&]() { return d(a64); }); // operator()()

    std::generate_n(std::ostream_iterator<double>(std::cout, "\n"),
                    10,
                    [&]() { return d(a32); }); // operator()()

    return 0;
}
