#pragma once

// SYCL-PRNG includes
#include <PRNG/concepts/Prelude.hpp> // LOTS

// Standard C++ includes
#include <iterator> // std::input_iterator_tag


namespace prng
{
    namespace meta
    {
        namespace alias
        {
            template <class T>
            using iterator_category = typename std::iterator_traits<T>::iterator_category;
        }
        
    }

    namespace concepts
    {
        template <class T>
        constexpr bool Iterator =
            meta::require<
                CopyConstructible<T>,
                CopyAssignable<T>,
                Destructible<T>,
                Swappable<T>,
                meta::exists<meta::ops::postfix_increment, T>,
                meta::exists<meta::ops::prefix_increment, T>,
                meta::exists<meta::ops::dereference, T>
            >;

        template <class T>
        constexpr bool InputIterator =
            Pointer<T> or
            meta::require<
                EqualityComparable<T>,
                Iterator<T>,
                meta::exists<meta::alias::value_type, T>,
                meta::exists<meta::alias::reference, T>,
                meta::either<
                    meta::identical_to<meta::detected_t<meta::alias::reference, T>, meta::ops::dereference, T>,
                    meta::converts_to<meta::detected_t<meta::alias::value_type, T>, meta::ops::dereference, T>
                >,
                meta::identical_to<meta::detected_t<meta::alias::pointer, T>, meta::ops::arrow, T>,
                meta::converts_to<meta::detected_t<meta::alias::value_type, T>, meta::ops::dereference, T&>,
                meta::converts_to<std::input_iterator_tag, meta::alias::iterator_category, T>
            >;

    } // namespace concepts
}
