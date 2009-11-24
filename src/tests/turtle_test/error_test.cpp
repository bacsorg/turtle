//
//  Copyright Mathieu Champlon 2008
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include <turtle/error.hpp>

#include <boost/test/auto_unit_test.hpp>
#define BOOST_LIB_NAME boost_unit_test_framework
#include <boost/config/auto_link.hpp>

BOOST_AUTO_TEST_CASE( a_mock_exception_is_not_an_std_exception_to_not_mess_with_user_exceptions )
{
    try
    {
        throw mock::exception();
    }
    catch( std::exception& )
    {
        BOOST_FAIL( "mock::exception must not be an std::exception" );
    }
    catch( mock::exception& )
    {}
}
