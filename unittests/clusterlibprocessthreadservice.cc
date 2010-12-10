#include "clusterlib.h"
#include "testparams.h"
#include "MPITestFixture.h"

extern TestParams globalTestParams;

using namespace clusterlib;
using namespace std;
using namespace boost;

class ClusterlibProcessThreadService : public MPITestFixture
{
    CPPUNIT_TEST_SUITE(ClusterlibProcessThreadService);
    CPPUNIT_TEST(testProcessThreadService1);
    CPPUNIT_TEST(testProcessThreadService2);
    CPPUNIT_TEST(testProcessThreadService3);
    CPPUNIT_TEST_SUITE_END();

  public:
    
    ClusterlibProcessThreadService() 
        : MPITestFixture(globalTestParams) {}

    /* Runs prior to each test */
    virtual void setUp() 
    {
    }

    /* Runs after each test */
    virtual void tearDown() 
    {
        cleanAndBarrierMPITest(NULL, false);
    }

    /**
     * Simple test to execute 'ls' when not capturing output.  Can't
     * do current path, since this can cause a deadlock since files
     * are being created in this path.
     */
    void testProcessThreadService1()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    NULL,
                                    false, 
                                    "testProcessThreadService1");
        vector<string> addEnv;
        string path;
        string cmd = "/bin/ls -al / > /dev/null";
        pid_t processId = -1;
        int32_t returnCode = -1;

        bool success = ProcessThreadService::forkExecWait(addEnv, 
                                                          path, 
                                                          cmd,
                                                          processId, 
                                                          returnCode);
        MPI_CPPUNIT_ASSERT(success == true);
        MPI_CPPUNIT_ASSERT(processId != -1);
        MPI_CPPUNIT_ASSERT(returnCode == 0);
    }


    /**
     * Simple test to execute 'ls' when capturing output.  Can't
     * do current path, since this can cause a deadlock since files
     * are being created in this path.
     */
    void testProcessThreadService2()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    NULL,
                                    false, 
                                    "testProcessThreadService2");
        vector<string> addEnv;
        string path;
        string cmd = "/bin/ls -al /";
        pid_t processId = -1;
        int32_t returnCode = -1;
        string stdoutOutput, stderrOutput;

        bool success = ProcessThreadService::forkExecWait(addEnv, 
                                                          path, 
                                                          cmd, 
                                                          processId, 
                                                          returnCode, 
                                                          stdoutOutput, 
                                                          stderrOutput);
        cerr << "stdout size=" << stdoutOutput.size() << ", data='" 
             <<  stdoutOutput << "'" << endl;
        cerr << "stderr size=" << stderrOutput.size() << ", data='" 
             <<  stderrOutput << "'" << endl;

        MPI_CPPUNIT_ASSERT(success == true);
        MPI_CPPUNIT_ASSERT(processId != -1);
        MPI_CPPUNIT_ASSERT(returnCode == 0);
        MPI_CPPUNIT_ASSERT(!stdoutOutput.empty());
        MPI_CPPUNIT_ASSERT(stderrOutput.empty());
    }

    /**
     * Exhaust the 4k buffer size.
     */
    void testProcessThreadService3()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    NULL,
                                    false, 
                                    "testProcessThreadService3");
        vector<string> addEnv;
        string path;
        string cmd = "/bin/dd if=/dev/zero bs=1k count=64";
        pid_t processId = -1;
        int32_t returnCode = -1;
        string stdoutOutput, stderrOutput;

        bool success = ProcessThreadService::forkExecWait(addEnv, 
                                                          path, 
                                                          cmd, 
                                                          processId, 
                                                          returnCode, 
                                                          stdoutOutput, 
                                                          stderrOutput);
        cerr << "stdout size=" << stdoutOutput.size() << ", data='" 
             <<  stdoutOutput << "'" << endl;
        cerr << "stderr size=" << stderrOutput.size() << ", data='" 
             <<  stderrOutput << "'" << endl;

        MPI_CPPUNIT_ASSERT(success == true);
        MPI_CPPUNIT_ASSERT(processId != -1);
        MPI_CPPUNIT_ASSERT(returnCode == 0);
        MPI_CPPUNIT_ASSERT(stdoutOutput.size() == 64*1024);
        MPI_CPPUNIT_ASSERT(!stderrOutput.empty());
    }
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibProcessThreadService);

