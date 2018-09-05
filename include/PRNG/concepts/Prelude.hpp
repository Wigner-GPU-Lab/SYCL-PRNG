#pragma once

#ifdef _WIN32
#define not !
#define and &&
#define or ||
#endif

// Standard C++ includes
#include <type_traits>		// LOTS
#include <utility>


namespace prng
{
    namespace meta
    {
        ///////////////////////////////
        // Detection idiom foot work //
        ///////////////////////////////

        template <bool B> using bool_constant = std::integral_constant<bool, B>;

        template <class...> using void_t = void;
        template <class T> struct identity { using type = T; };

        template <class T, class Void, template <class...> class, class...>
        struct detector :
            identity<T>
        {
            using value_t = std::false_type;
        };

        template <class T, template <class...> class U, class... Args>
        struct detector<T, void_t<U<Args...>>, U, Args...> :
            identity<U<Args...>>
        {
            using value_t = std::true_type;
        };

        struct nonesuch final {
            nonesuch(nonesuch const&) = delete;
            nonesuch() = delete;
            ~nonesuch() = delete;

            void operator = (nonesuch const&) = delete;
        };

        template <class T, template <class...> class U, class... Args>
        using detected_or = detector<T, void, U, Args...>;

        template <template <class...> class T, class... Args>
        using detected_t = typename detected_or<nonesuch, T, Args...>::type;

        template <class T, template <class...> class U, class... Args>
        using detected_or_t = typename detected_or<T, U, Args...>::type;

        template <class To, template <class...> class T, class... Args>
        using is_detected_convertible = std::is_convertible<
            detected_t<T, Args...>,
            To
        >;

        template <class T, template <class...> class U, class... Args>
        using is_detected_exact = std::is_same<T, detected_t<U, Args...>>;

        template <template <class...> class T, class... Args>
        using is_detected = typename detected_or<nonesuch, T, Args...>::value_t;

        template <class...> struct conjunction;
        template <class...> struct disjunction;
        template <class B> using negation = bool_constant<not B::value>;

        template <class T, class... Ts>
        struct conjunction<T, Ts...> :
            bool_constant<T::value and conjunction<Ts...>::value> {};
        template <> struct conjunction<> : std::true_type {};

        template <class T, class... Ts>
        struct disjunction<T, Ts...> :
            bool_constant<T::value or disjunction<Ts...>::value> {};

        template <> struct disjunction<> : std::false_type {};

        template <class From, class To>
        using explicit_cast = decltype(static_cast<To>(std::declval<From>()));

        ///////////////////////////////////
        // special names for readability //
        ///////////////////////////////////

        template <bool... Bs>
        constexpr bool require = conjunction<bool_constant<Bs>...>::value;

        template <bool... Bs>
        constexpr bool either = disjunction<bool_constant<Bs>...>::value;

        template <bool... Bs>
        constexpr bool disallow = not require<Bs...>;

        template <template <class...> class Op, class... Args>
        constexpr bool exists = is_detected<Op, Args...>::value;

        template <class To, template <class...> class Op, class... Args>
        constexpr bool casts_to = exists<explicit_cast, detected_t<Op, Args...>, To>;

        template <class To, template <class...> class Op, class... Args>
        constexpr bool converts_to = is_detected_convertible<To, Op, Args...>::value;

        template <class Exact, template <class...> class Op, class... Args>
        constexpr bool identical_to = is_detected_exact<Exact, Op, Args...>::value;


        namespace adl
        {
            template <class T, class U = T>
            using swap_with = decltype(std::swap(std::declval<T>(), std::declval<U>()));
        }

        namespace ops
        {
            template <class T, class U> using equal_to = decltype(std::declval<T>() == std::declval<U>());
            template <class T, class U> using less = decltype(std::declval<T>() < std::declval<U>());
            template <class T> using dereference = decltype(*std::declval<T>());
            template <class T> using arrow = decltype(std::declval<T>().operator->());

            template <class T> using postfix_increment = decltype(std::declval<T>()++);
            template <class T> using prefix_increment = decltype(++std::declval<T>());
        }

        namespace alias
        {
            template <class T> using value_type = typename T::value_type;
            template <class T> using reference = typename T::reference;
            template <class T> using pointer = typename T::pointer;
        }
    }

    namespace concepts
    {
        template <class T, class U> constexpr bool SwappableWith = meta::exists<meta::adl::swap_with, T, U>;


        template <class T> constexpr bool CopyConstructible = std::is_copy_constructible<T>::value;
        template <class T> constexpr bool CopyAssignable = std::is_copy_assignable<T>::value;
        template <class T> constexpr bool Destructible = std::is_destructible<T>::value;
        template <class T> constexpr bool Swappable = SwappableWith<T&, T&>;
        template <class T> constexpr bool Pointer = std::is_pointer<T>::value;

        template <class T, class U = T>
        constexpr bool EqualityComparable = meta::converts_to<bool, meta::ops::equal_to, T, U>;

        template <class T, class... Args> constexpr bool Constructible = Destructible<T> && std::is_constructible_v<T, Args...>;

        template <class T > constexpr bool DefaultConstructible = Constructible<T>;

        template <class From, class To> constexpr bool ConvertibleTo = std::is_convertible<From, To>::value;
    }
}
