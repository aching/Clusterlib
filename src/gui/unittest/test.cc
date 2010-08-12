#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>
#include <log4cxx/basicconfigurator.h>

class TestSetup
{
public:
    TestSetup() {
        log4cxx::BasicConfigurator::configure();
    }
};

TestSetup testSetup;
