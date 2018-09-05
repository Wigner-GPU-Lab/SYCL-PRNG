#pragma once

// SYCL-PRNG includes
#include <PRNG/concepts/Prelude.hpp>
#include <PRNG/concepts/InputIterator.hpp>

// Standard C++ includes
#include <cstdint>  // std::uint_least32_t
#include <cstddef>  // std::size_t

namespace prng
{
    namespace meta
    {
        namespace member
        {
            // TODO: return types and const qualifiers not really tested
            template <class T, class IT> using generate = decltype(std::declval<T>().generate(std::declval<IT>(), std::declval<IT>()));
            template <class T> using size = decltype(std::declval<T>().size());
        }
    }

    namespace concepts
    {
        template <class T>
        constexpr bool SeedSequence =
            meta::require<
                //meta::exists<meta::alias::value_type, std::uint_least32_t>,
                //meta::either<
                //    meta::identical_to<meta::alias::value_type, std::uint_least32_t>,
                //    meta::identical_to<meta::alias::value_type, std::uint_least64_t>
                //>,
                DefaultConstructible<T>//,
                //Constructible<T, std::istreambuf_iterator<std::uint_least32_t>, std::istreambuf_iterator<std::uint_least32_t>>,
                //Constructible<T, std::initializer_list<std::uint_least32_t>>,
                //meta::identical_to<meta::member::generate<T, std::uint_least32_t*>, void>,
                //meta::identical_to<meta::member::size<T>, std::size_t>
            >;
    }
}
