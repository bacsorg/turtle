//
//  Copyright Mathieu Champlon 2008
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MOCK_MOCK_HPP_INCLUDED
#define MOCK_MOCK_HPP_INCLUDED

#include "config.hpp"
#include "cleanup.hpp"
#include "object.hpp"
#include "function.hpp"
#include "args.hpp"
#include "type_name.hpp"
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/function_type.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/mpl/joint_view.hpp>
#include <boost/mpl/single_view.hpp>
#include <boost/mpl/pop_front.hpp>
#define BOOST_TYPEOF_SILENT
#include <boost/typeof/typeof.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>

namespace mock
{
namespace detail
{
    template< typename M >
    struct signature :
        boost::function_types::function_type<
            boost::mpl::joint_view<
                boost::mpl::single_view<
                    BOOST_DEDUCED_TYPENAME
                        boost::function_types::result_type< M >::type
                >,
                BOOST_DEDUCED_TYPENAME boost::mpl::pop_front<
                    BOOST_DEDUCED_TYPENAME
                        boost::function_types::parameter_types< M >
                >::type
            >
        >
    {};

    template< typename T >
    struct base
    {
        typedef T base_type;
    };

    template< typename S >
    struct functor : mock::function< S >
    {
        functor()
        {
            static functor* f = 0;
            if( f )
            {
                *this = *f;
                f = 0;
            }
            else
                f = this;
        }
    };
}

    // if an error is generated by the line below it probably means either the
    // method does not exist or it is ambiguous : use MOCK_METHOD_EXT instead
    template< typename T >
    T& invalid_pointer_to_member( const T& );
}

#define MOCK_BASE_CLASS(T, I) \
    struct T : I, mock::object, mock::detail::base< I >
#define MOCK_CLASS(T) \
    struct T : mock::object
#define MOCK_FUNCTOR(f, S) \
    mock::detail::functor< S > f, f##_mocker

#define MOCK_MOCKER(t) \
    t##_mocker( mock::detail::root, BOOST_PP_STRINGIZE(t) )
#define MOCK_ANONYMOUS_MOCKER(t) \
    t##_mocker( mock::detail::root, "?." )

#define MOCK_METHOD_EXPECTATION(S, t) \
    mutable mock::function< S > t##_mocker_; \
    mock::function< S >& t##_mocker( const mock::detail::context&, \
        boost::unit_test::const_string instance ) const \
    { \
        mock::detail::configure( *this, t##_mocker_, \
            instance.substr( 0, instance.rfind( BOOST_PP_STRINGIZE(t) ) ), \
            mock::detail::type_name( typeid( *this ) ), \
            BOOST_PP_STRINGIZE(t) ); \
        return t##_mocker_; \
    }

#define MOCK_SIGNATURE(M) \
    mock::detail::signature< \
        BOOST_TYPEOF( mock::invalid_pointer_to_member( &base_type::M ) ) \
    >::type

#define MOCK_METHOD_STUB(M, n, S, t, c, tpn) \
    MOCK_DECL(M, n, S, c, tpn) \
    { \
        return MOCK_ANONYMOUS_MOCKER(t)( \
            BOOST_PP_ENUM_PARAMS(n, p) ); \
    }

#define MOCK_METHOD_EXT(M, n, S, t) \
    MOCK_METHOD_STUB(M, n, S, t,,) \
    MOCK_METHOD_STUB(M, n, S, t, const,) \
    MOCK_METHOD_EXPECTATION(S, t)
#define MOCK_CONST_METHOD_EXT(M, n, S, t) \
    MOCK_METHOD_STUB(M, n, S, t, const,) \
    MOCK_METHOD_EXPECTATION(S, t)
#define MOCK_NON_CONST_METHOD_EXT(M, n, S, t) \
    MOCK_METHOD_STUB(M, n, S, t,,) \
    MOCK_METHOD_EXPECTATION(S, t)
#define MOCK_METHOD(M, n) \
    MOCK_METHOD_EXT(M, n, MOCK_SIGNATURE(M), M)

#define MOCK_METHOD_EXT_TPL(M, n, S, t) \
    MOCK_METHOD_STUB(M, n, S, t,, BOOST_DEDUCED_TYPENAME) \
    MOCK_METHOD_STUB(M, n, S, t, const, BOOST_DEDUCED_TYPENAME) \
    MOCK_METHOD_EXPECTATION(S, t)
#define MOCK_CONST_METHOD_EXT_TPL(M, n, S, t) \
    MOCK_METHOD_STUB(M, n, S, t, const, BOOST_DEDUCED_TYPENAME) \
    MOCK_METHOD_EXPECTATION(S, t)
#define MOCK_NON_CONST_METHOD_EXT_TPL(M, n, S, t) \
    MOCK_METHOD_STUB(M, n, S, t,, BOOST_DEDUCED_TYPENAME) \
    MOCK_METHOD_EXPECTATION(S, t)

#define MOCK_DESTRUCTOR(T, t) \
    ~T() { MOCK_ANONYMOUS_MOCKER(t).test(); } \
    MOCK_METHOD_EXPECTATION(void(), t)

#define MOCK_CONST_CONVERSION_OPERATOR(T, t) \
    operator T() const { return MOCK_ANONYMOUS_MOCKER(t)(); } \
    MOCK_METHOD_EXPECTATION(T(), t)
#define MOCK_NON_CONST_CONVERSION_OPERATOR(T, t) \
    operator T() { return MOCK_ANONYMOUS_MOCKER(t)(); } \
    MOCK_METHOD_EXPECTATION(T(), t)
#define MOCK_CONVERSION_OPERATOR(T, t) \
    operator T() const { return MOCK_ANONYMOUS_MOCKER(t)(); } \
    operator T() { return MOCK_ANONYMOUS_MOCKER(t)(); } \
    MOCK_METHOD_EXPECTATION(T(), t)

#define MOCK_FUNCTION_STUB(F, n, S, t, s) \
    s mock::function< S >& t##_mocker( mock::detail::context& context, \
        boost::unit_test::const_string instance ) \
    { \
        static mock::function< S > f; \
        return f( context, instance ); \
    } \
    s MOCK_DECL(F, n, S,,) \
    { \
        return MOCK_MOCKER(t)( BOOST_PP_ENUM_PARAMS(n, p) ); \
    }
#define MOCK_FUNCTION(F, n, S, t) \
    MOCK_FUNCTION_STUB(F, n, S, t,)
#define MOCK_STATIC_FUNCTION(F, n, S, t) \
    MOCK_FUNCTION_STUB(F, n, S, t, static)

#define MOCK_EXPECT(t) MOCK_MOCKER(t).expect( __FILE__, __LINE__ )
#define MOCK_RESET(t) MOCK_MOCKER(t).reset()
#define MOCK_VERIFY(t) MOCK_MOCKER(t).verify()

#endif // MOCK_MOCK_HPP_INCLUDED
