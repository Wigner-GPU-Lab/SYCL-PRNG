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

namespace kernels
{
    struct skip;
    struct step;

    template <typename Engine, typename Method>
    struct invocation;
}

int main()
{
    static_assert(std::is_standard_layout<prng::tinymt_64>::value, "TinyMT 64 is not standard layout.");
    static_assert(std::is_standard_layout<prng::tinymt_32>::value, "TinyMT 32 is not standard layout.");
    static_assert(std::is_standard_layout<prng::mwc64x_32>::value, "MWC64X 32 is not standard layout.");

    // Sample params
    const std::size_t plat_index = std::numeric_limits<std::size_t>::max();
    const std::size_t dev_index = std::numeric_limits<std::size_t>::max();
    const auto dev_type = cl::sycl::info::device_type::gpu;
    const std::size_t length = 65536u;
    const std::size_t num = 64;

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
        cl::sycl::buffer<prng::tinymt_64> tmt64{ cl::sycl::range<1>{ length } };
        cl::sycl::buffer<prng::tinymt_32> tmt32{ cl::sycl::range<1>{ length } };
        cl::sycl::buffer<prng::mwc64x_32> mwc32{ cl::sycl::range<1>{ length } };

        cl::sycl::buffer<prng::tinymt_64> tmt64_ref{ cl::sycl::range<1>{ length } };
        cl::sycl::buffer<prng::tinymt_32> tmt32_ref{ cl::sycl::range<1>{ length } };
        cl::sycl::buffer<prng::mwc64x_32> mwc32_ref{ cl::sycl::range<1>{ length } };

        // Initialize seeds
        {
            std::random_device rd;

            auto seeds_acc = seeds_buf.get_access<cl::sycl::access::mode::write>();

            std::generate(seeds_acc.get_pointer(),
                          seeds_acc.get_pointer() + seeds_acc.get_count(),
                          [&]() { return rd(); });
        }

        auto jump_engines = [&, num](auto& skip_seq, auto& step_seq)
        {
            auto skip_function_discard = [n = num](auto& engine) { engine.discard(n); }; // discard
            auto next_stepping_discard = [n = num](auto& engine) { for (std::int32_t i = 0; i < n; ++i) engine(); }; // operator()

            queue.submit([&](cl::sycl::handler& cgh)
            {
                auto seeds = seeds_buf.get_access<cl::sycl::access::mode::read>();
                auto engines = skip_seq.template get_access<cl::sycl::access::mode::write>();

                using engine_type = typename decltype(engines)::value_type;

                cgh.parallel_for<kernels::invocation<engine_type, kernels::skip>>(cl::sycl::range<1>{length}, [=](cl::sycl::item<1> i)
                {
                    auto seed = seeds[i];

                    // Cast required because of ComputeCpp bug
                    engine_type engine{ static_cast<typename engine_type::result_type>(seed) }; // random seeding

                    skip_function_discard(engine); // discard

                    engines[i] = engine; // copy CTOR
                });
            });

            // TOOD: remove code duplication
            queue.submit([&](cl::sycl::handler& cgh)
            {
                auto seeds = seeds_buf.get_access<cl::sycl::access::mode::read>();
                auto engines = skip_seq.template get_access<cl::sycl::access::mode::write>();

                using engine_type = typename decltype(engines)::value_type;

                cgh.parallel_for<kernels::invocation<engine_type, kernels::step>>(cl::sycl::range<1>{length}, [=](cl::sycl::item<1> i)
                {
                    auto seed = seeds[i];

                    // Cast required because of ComputeCpp bug
                    engine_type engine{ static_cast<typename engine_type::result_type>(seed) }; // random seeding

                    next_stepping_discard(engine); // discard

                    engines[i] = engine; // copy CTOR
                });
            });
        };

        auto match_skip_vs_step = [=](auto& skip_seq, auto& step_seq)
        {
            jump_engines(skip_seq, step_seq);

            auto skip_acc = skip_seq.template get_access<cl::sycl::access::mode::read>();
            auto step_acc = step_seq.template get_access<cl::sycl::access::mode::read>();

            auto iters = std::mismatch(skip_acc.get_pointer(), skip_acc.get_pointer() + skip_acc.get_count(), step_acc.get_pointer()); // operator==

            if (iters.first != (skip_acc.get_pointer() + skip_acc.get_count()) ||
                iters.second != (step_acc.get_pointer() + step_acc.get_count()))
            {
                std::cerr << "Skip vs. step differs for " <<
                    typeid(skip_acc[0]).name() <<
                    " when discarding " <<
                    num <<
                    " elements." <<
                    std::endl;

                std::exit(EXIT_FAILURE);
            }
        };

        match_skip_vs_step(tmt64, tmt64_ref);
        match_skip_vs_step(tmt32, tmt32_ref);
        match_skip_vs_step(mwc32, mwc32_ref);

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

    return 0;
}
