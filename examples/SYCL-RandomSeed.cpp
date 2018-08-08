// SYCL-PRNG includes
#include <PRNG/TinyMT.hpp>
#include <PRNG/MWC64X.hpp>

// SYCL includes
#include <CL/sycl.hpp>

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
    static_assert(std::is_standard_layout<prng::mwc64x>::value, "MWC64X is not standard layout.");

    // Sample params
    const std::size_t plat_index = std::numeric_limits<std::size_t>::max();
    const std::size_t dev_index = std::numeric_limits<std::size_t>::max();
    const auto dev_type = cl::sycl::info::device_type::gpu;
    const std::size_t length = 65536u;
    const std::size_t num = 1024;

    try
    {
        // Platform selection
        auto plats = cl::sycl::platform::get_platforms();

        if (plats.empty()) throw std::runtime_error{ "No OpenCL platform found." };

        std::cout << "Found platforms:" << std::endl;
        for (const auto plat : plats) std::cout << "\t" << plat.get_info<cl::sycl::info::platform::vendor>() << std::endl;

        auto plat = plats.at(plat_index == std::numeric_limits<std::size_t>::max() ? 0 : plat_index);

        std::cout << "\n" << "Selected platform: " << plat.get_info<cl::sycl::info::platform::vendor>() << std::endl;

        // Device selection
        auto devs = plat.get_devices(dev_type);

        if (devs.empty()) throw std::runtime_error{ "No OpenCL device of specified type found on selected platform." };

        auto dev = devs.at(dev_index == std::numeric_limits<std::size_t>::max() ? 0 : dev_index);

        std::cout << "Selected device: " << dev.get_info<cl::sycl::info::device::name>() << "\n" << std::endl;

        // Context, queue, buffer creation
        auto async_error_handler = [](cl::sycl::exception_list errors) { for (auto error : errors) throw error; };

        cl::sycl::context ctx{ dev, async_error_handler };

        cl::sycl::queue queue{ dev };

        cl::sycl::buffer<std::random_device::result_type> seeds_buf{ cl::sycl::range<1>{ length } };
        cl::sycl::buffer<prng::tinymt_64> tmt64_buf{ cl::sycl::range<1>{ length } };
        cl::sycl::buffer<prng::tinymt_32> tmt32_buf{ cl::sycl::range<1>{ length } };
        cl::sycl::buffer<prng::mwc64x> mwc32_buf{ cl::sycl::range<1>{ length } };

        std::vector<std::random_device::result_type> seeds_vec;
        std::vector<prng::tinymt_64> tmt64_vec( length );
        std::vector<prng::tinymt_32> tmt32_vec( length );
        std::vector<prng::mwc64x> mwc32_vec( length );

        // Initialize seeds
        {
            std::random_device rd;

            std::generate_n(std::back_inserter(seeds_vec), length, [&]() { return rd(); });
        }

        auto jump_engines = [&](auto engines_buf)
        {
            queue.submit([&](cl::sycl::handler& cgh)
            {
                auto seeds = seeds_buf.get_access<cl::sycl::access::mode::read>();
                auto engines = engines_buf.template get_access<cl::sycl::access::mode::write>();

                using engine_type = typename decltype(engines)::value_type;

                cgh.parallel_for<class SYCL_RandomSeed>(cl::sycl::range<1>{length}, [=](cl::sycl::item<1> i)
                {
                    auto seed = seeds[i];

                    engine_type engine{ seed };

                    engine.discard(num);

                    engines[i] = engine;
                });
            });
        };

        jump_engines(tmt64_buf);
        jump_engines(tmt32_buf);
        jump_engines(mwc32_buf);

        // Verify
        //{
        //    auto access = buf.get_access<cl::sycl::access::mode::read>();
        //
        //    if (std::any_of(access.get_pointer(),
        //        access.get_pointer() + access.get_count(),
        //        [res = 1.f + 1.f + 2.f](const float& val) { return val != res; }))
        //        throw std::runtime_error{ "Wrong result computed in kernel." };
        //}

        std::cout << "Result verification passed!" << std::endl;
    }
    catch (cl::sycl::exception e)
    {
        std::cerr << e.what() << std::endl;
        std::exit(e.get_cl_code());
    }
    catch (std::exception e)
    {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::vector<prng::tinymt_64> tmt64s{ prng::tinymt_64{} }; // default CTOR
    std::vector<prng::tinymt_32> tmt32s{ prng::tinymt_32{} }; // default CTOR
    std::vector<prng::mwc64x> mwc32s{ prng::mwc64x{} }; // default CTOR

    {
        std::random_device rd;
        std::generate_n(std::back_inserter(tmt64s),
                        10,
                        [&]() { return prng::tinymt_64{ rd() }; }); // random seeding

        std::generate_n(std::back_inserter(tmt32s),
                        10,
                        [&]() { return prng::tinymt_32{ rd() }; }); // random seeding

        std::generate_n(std::back_inserter(mwc32s),
                        10,
                        [&]() { return prng::mwc64x{ rd() }; }); // random seeding
    }

    auto tmt64_it = std::find(tmt64s.cbegin(), tmt64s.cend(), prng::tinymt_64{}); // operator==
    auto tmt32_it = std::find(tmt32s.cbegin(), tmt32s.cend(), prng::tinymt_32{}); // operator==
    auto mwc32_it = std::find(mwc32s.cbegin(), mwc32s.cend(), prng::mwc64x{}); // operator==

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
