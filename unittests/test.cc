#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <mpi.h>
#include "testparams.h"

using namespace std;

const string debugExt = ".out";

/* Global test parameters to be used in all tests */
TestParams globalTestParams;

int main(int argc, char* argv[]) {
    /* MPI initialization */
    MPI::Init(argc, argv);

    /* Parse arguments */
    globalTestParams.setMyId(MPI::COMM_WORLD.Get_rank());
    globalTestParams.setNumProcs(MPI::COMM_WORLD.Get_size());
    globalTestParams.rootParseArgs(argc, argv);
    if (globalTestParams.scatterArgs() != 0) {
        MPI::Finalize();
        return -1;
    }

    /* Redirect all stderr log4cxxx messaging to the same files */
    stringstream fileStringStream;
    fileStringStream << MPI::COMM_WORLD.Get_rank();
    fileStringStream << debugExt;
    remove(fileStringStream.str().c_str());
    FILE *stderrFile = freopen(fileStringStream.str().c_str(), "a", stderr);
    if (!stderrFile) {
        cerr << "freopen to redirect stderr failed" << endl;
        MPI::Finalize();
        return -1;
    }

    /* MPI set error handler to throw exceptions */
    MPI::COMM_WORLD.Set_errhandler(MPI::ERRORS_THROW_EXCEPTIONS);
    
    /* Get the top level suite from the registry */
    CppUnit::Test *suite = 
	CppUnit::TestFactoryRegistry::getRegistry().makeTest();

    /* Adds the test to the list of test to run */
    CppUnit::TextUi::TestRunner runner;
    runner.addTest(suite);

    /* Change the default outputter to stderr */
    runner.setOutputter(
        new CppUnit::CompilerOutputter(&runner.result(), std::cerr));

    /* Run the tests. */
    int wasSuccessful = runner.run();

    /* Check if all processes were successful */
    int *successArr = NULL;
    if (MPI::COMM_WORLD.Get_rank() == 0) {
        successArr = new int[MPI::COMM_WORLD.Get_size()];
    }
    MPI::COMM_WORLD.Gather(
        &wasSuccessful, 1, MPI::INT, successArr, 1, MPI::INT, 0);
    if (MPI::COMM_WORLD.Get_rank() == 0) {
        for (int i = 0; i < MPI::COMM_WORLD.Get_size(); i++) {
            if (successArr[i] == 0) {
                cerr << "Process " << i << " failed" << endl;
                wasSuccessful = successArr[i];
            }
        }
        delete [] successArr;
        successArr = NULL;

        cout << (wasSuccessful ? "SUCCESS" : "FAILURE")
             << " - See details in files (0-"
             << MPI::COMM_WORLD.Get_size() - 1 << ")"
             << debugExt<< endl;
    }

    if (fclose(stderrFile)) {
        cerr << "fclose stderrFile failed.";
    }

    /* MPI Finalization */
    MPI::Finalize();

    /* Return error code 1 if the one of test failed. */
    return wasSuccessful ? 0 : 1;
}
