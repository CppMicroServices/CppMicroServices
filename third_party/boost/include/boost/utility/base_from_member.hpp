//  boost utility/base_from_member.hpp header file  --------------------------//

//  Copyright 2001, 2003, 2004, 2012 Daryle Walker.  Use, modification, and
//  distribution are subject to the Boost Software License, Version 1.0.  (See
//  accompanying file LICENSE_1_0.txt or a copy at
//  <http://www.boost.org/LICENSE_1_0.txt>.)

//  See <http://www.boost.org/libs/utility/> for the library's home page.

#ifndef BOOST_UTILITY_BASE_FROM_MEMBER_HPP
#define BOOST_UTILITY_BASE_FROM_MEMBER_HPP

#include <boost/config.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/remove_cv.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/utility/enable_if.hpp>


//  Base-from-member arity configuration macro  ------------------------------//

// The following macro determines how many arguments will be in the largest
// constructor template of base_from_member.  Constructor templates will be
// generated from one argument to this maximum.  Code from other files can read
// this number if they need to always match the exact maximum base_from_member
// uses.  The maximum constructor length can be changed by overriding the
// #defined constant.  Make sure to apply the override, if any, for all source
// files during project compiling for consistency.

// Contributed by Jonathan Turkanis

#ifndef BOOST_BASE_FROM_MEMBER_MAX_ARITY
#define BOOST_BASE_FROM_MEMBER_MAX_ARITY  10
#endif


//  An iteration of a constructor template for base_from_member  -------------//

// A macro that should expand to:
//     template < typename T1, ..., typename Tn >
//     base_from_member( T1 x1, ..., Tn xn )
//         : member( x1, ..., xn )
//         {}
// This macro should only persist within this file.

#define BOOST_PRIVATE_CTR_DEF( z, n, data )                   \
    template < BOOST_PP_ENUM_PARAMS(n, typename T) >          \
    base_from_member( BOOST_PP_ENUM_BINARY_PARAMS(n, T, x) )  \
        : member( BOOST_PP_ENUM_PARAMS(n, x) )                \
        {}                                                    \
    /**/


namespace cppmsboost
{

namespace detail
{

//  Type-unmarking class template  -------------------------------------------//

// Type-trait to get the raw type, i.e. the type without top-level reference nor
// cv-qualification, from a type expression.  Mainly for function arguments, any
// reference part is stripped first.

// Contributed by Daryle Walker

template < typename T >
struct remove_cv_ref
{
    typedef typename ::cppmsboost::remove_cv<typename
     ::cppmsboost::remove_reference<T>::type>::type  type;

};  // cppmsboost::detail::remove_cv_ref

//  Unmarked-type comparison class template  ---------------------------------//

// Type-trait to check if two type expressions have the same raw type.

// Contributed by Daryle Walker, based on a work-around by Luc Danton

template < typename T, typename U >
struct is_related
    : public ::cppmsboost::is_same<
     typename ::cppmsboost::detail::remove_cv_ref<T>::type,
     typename ::cppmsboost::detail::remove_cv_ref<U>::type >
{};

//  Enable-if-on-unidentical-unmarked-type class template  -------------------//

// Enable-if on the first two type expressions NOT having the same raw type.

// Contributed by Daryle Walker, based on a work-around by Luc Danton

#ifndef BOOST_NO_CXX11_VARIADIC_TEMPLATES
template<typename ...T>
struct enable_if_unrelated
    : public ::cppmsboost::enable_if_c<true>
{};

template<typename T, typename U, typename ...U2>
struct enable_if_unrelated<T, U, U2...>
    : public ::cppmsboost::disable_if< ::cppmsboost::detail::is_related<T, U> >
{};
#endif

}  // namespace cppmsboost::detail


//  Base-from-member class template  -----------------------------------------//

// Helper to initialize a base object so a derived class can use this
// object in the initialization of another base class.  Used by
// Dietmar Kuehl from ideas by Ron Klatcho to solve the problem of a
// base class needing to be initialized by a member.

// Contributed by Daryle Walker

template < typename MemberType, int UniqueID = 0 >
class base_from_member
{
protected:
    MemberType  member;

#if !defined(BOOST_NO_CXX11_RVALUE_REFERENCES) && \
    !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES) && \
    !defined(BOOST_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS) && \
    !(defined(__GNUC__) && (__GNUC__ == 4) && (__GNUC_MINOR__ < 4))
    template <typename ...T, typename EnableIf = typename
     ::cppmsboost::detail::enable_if_unrelated<base_from_member, T...>::type>
    explicit BOOST_CONSTEXPR base_from_member( T&& ...x )
        BOOST_NOEXCEPT_IF( BOOST_NOEXCEPT_EXPR(::new ((void*) 0) MemberType(
         static_cast<T&&>(x)... )) )  // no std::is_nothrow_constructible...
        : member( static_cast<T&&>(x)... )     // ...nor std::forward needed
        {}
#else
    base_from_member()
        : member()
        {}

    template < typename T0 > explicit base_from_member( T0 x0 ) : member( x0 ) {}
    BOOST_PP_REPEAT_FROM_TO( 2, BOOST_PP_INC(BOOST_BASE_FROM_MEMBER_MAX_ARITY),
     BOOST_PRIVATE_CTR_DEF, _ )
#endif

};  // cppmsboost::base_from_member

template < typename MemberType, int UniqueID >
class base_from_member<MemberType&, UniqueID>
{
protected:
    MemberType& member;

    explicit BOOST_CONSTEXPR base_from_member( MemberType& x )
        BOOST_NOEXCEPT
        : member( x )
        {}

};  // cppmsboost::base_from_member

}  // namespace cppmsboost


// Undo any private macros
#undef BOOST_PRIVATE_CTR_DEF


#endif  // BOOST_UTILITY_BASE_FROM_MEMBER_HPP
