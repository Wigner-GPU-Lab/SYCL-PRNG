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

    std::vector<prng::tinymt_64> engines{ prng::tinymt_64{} }; // default CTOR

    {
        std::random_device rd;
        std::generate_n(std::back_inserter(engines),
                        10,
                        [&]() { return prng::tinymt_64{ rd() }; }); // random seeding
    }

    auto it = std::find(engines.cbegin(), engines.cend(), prng::tinymt_64{}); // operator==

    prng::tinymt_64 a;
    std::uniform_real_distribution<> d;

    std::generate_n(std::ostream_iterator<double>(std::cout, "\n"),
                    40,
                    [&]() { return d(a); });

    return 0;
}
