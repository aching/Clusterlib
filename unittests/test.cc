#include <cppunit/CompilerOutputter.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestResultCollector.h>
#include <mpi.h>
#include "clusterlib.h"
#include "testparams.h"
#include <iomanip>
#include <time.h>

using namespace std;
using namespace boost;

/* Global test parameters to be used in all tests */
TestParams globalTestParams;

int main(int argc, char* argv[]) 
{
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

    /* 
     * Inform users about the output files in case the test causes
     * failure before the results.
     */
    const int32_t timeBufferSize = 1024;
    char timeBuffer[timeBufferSize];
    time_t curTime;
    struct tm *localTime;
    curTime = time (NULL);       
    localTime = localtime(&curTime);
    if (!strftime(timeBuffer, 
                  timeBufferSize, 
                  "cppunit.%Y-%m-%d-%H-%M-%S.mpi.", 
                  localTime)) {
        cerr << "Cannot parse time " << endl;
        MPI::Finalize();
        return -1;
    }
    string prefix(timeBuffer);
    uint32_t *pidArr = NULL;
    if (MPI::COMM_WORLD.Get_rank() == 0) {
        pidArr = new uint32_t[MPI::COMM_WORLD.Get_size()];
    }
    unsigned pid = static_cast<unsigned>(getpid());
    MPI::COMM_WORLD.Gather(
        &pid, 1, MPI::UNSIGNED, pidArr, 1, MPI::UNSIGNED, 0);

    if ((MPI::COMM_WORLD.Get_rank() == 0) && 
        (globalTestParams.getOutputType() == TestParams::FILE)) {
        cout << endl << "Output files:" << endl;
        for (int i = 0; i < MPI::COMM_WORLD.Get_size(); i++) {
            cout << setfill('0') << " " << prefix << setw(3) << i 
                 << ".pid." << setw(6) << pidArr[i] << endl;
        }
        cout << endl;
    }

    /* 
     * Redirect all stderr log4cxxx messaging to the same files if the
     * file out put type is chosen. 
     */
    FILE *stderrFile = NULL;
    if (globalTestParams.getOutputType() == TestParams::FILE) {
        stringstream fileStringStream;
        fileStringStream << setfill('0');
        fileStringStream << prefix;
        fileStringStream << setw(3);
        fileStringStream << MPI::COMM_WORLD.Get_rank();
        fileStringStream << ".pid.";
        fileStringStream << setw(6);
        fileStringStream << getpid();
        remove(fileStringStream.str().c_str());
        stderrFile = freopen(fileStringStream.str().c_str(), 
                             "a", 
                             stderr);
        if (!stderrFile) {
            cerr << "freopen to redirect stderr failed" << endl;
            MPI::Finalize();
            return -1;
        }
    }

    /* MPI set error handler to throw exceptions */
    MPI::COMM_WORLD.Set_errhandler(MPI::ERRORS_THROW_EXCEPTIONS);
    
    /* Get the top level suite from the registry. */
    CppUnit::Test *suite = 
        CppUnit::TestFactoryRegistry::getRegistry().makeTest();

    /* Adds the test to the list of test to run */
    CppUnit::TextUi::TestRunner runner;
    runner.addTest(suite);

    /* Change the default outputter to stderr */
    runner.setOutputter(
        new CppUnit::CompilerOutputter(&runner.result(), std::cerr));

    /* Set an XML outputter if desired */
    std::ofstream xmlFileOut(globalTestParams.getXmlOutputFile().c_str());
    if (!globalTestParams.getXmlOutputFile().empty()) {
        CppUnit::XmlOutputter *xmlOutputter = 
            new CppUnit::XmlOutputter(&runner.result(), xmlFileOut);
        runner.setOutputter(xmlOutputter);
    }
    
    /* Cleanup the old property list */
    if (MPI::COMM_WORLD.Get_rank() == 0) {
        globalTestParams.resetClPropertyList();
    }
    MPI::COMM_WORLD.Barrier();

    /* Run one or more test fixtures. */
    double startTime = MPI_Wtime();
    int wasSuccessful = runner.run(globalTestParams.getTestFixtureName());

    /* Check if all processes were successful */
    /* [wasSuccessful, total tests, test errors, test failures]  */
    const int resultArrLen = 4;
    int resultArr[resultArrLen];
    resultArr[0] = wasSuccessful;
    resultArr[1] = runner.result().runTests();
    resultArr[2] = runner.result().testErrors();
    resultArr[3] = runner.result().testFailures();
    int *allResultArr = NULL;
    if (MPI::COMM_WORLD.Get_rank() == 0) {
        allResultArr = new int[MPI::COMM_WORLD.Get_size() * resultArrLen];
    }
    MPI::COMM_WORLD.Gather(
        &resultArr, 4, MPI::INT, allResultArr, 4, MPI::INT, 0);

    if (MPI::COMM_WORLD.Get_rank() == 0) {
        for (int i = 0; i < MPI::COMM_WORLD.Get_size(); i++) {
            if (allResultArr[i * resultArrLen] == 0) {
                cerr << "Process " << i << " failed" << endl;
                wasSuccessful = allResultArr[i * resultArrLen];
            }
        }
 
        /* Print out test results */
        for (int i = 0; i < resultArrLen; i ++) {
            switch (i) {
                case 0:
                    cout << "      MPI process id ";
                    break;
                case 1: 
                    cout << "         total tests ";
                    break;
                case 2:
                    cout << " uncaught exceptions ";
                    break;
                case 3:
                    cout << "          assertions ";
                    break;
                default:
                    cout << "Error: shouldn't have gotten here";
            }
            cout << setfill(' ');
            for (int j = 0; j < MPI::COMM_WORLD.Get_size(); j++) {
                if (i == 0) {
                    cout << setw(3) << dec << j << " ";
                }
                else {
                    cout << setw(3) << dec
                         << allResultArr[i + (j * resultArrLen)] << " ";
                }
            }
            cout << endl;
        } 
        delete [] allResultArr;
        allResultArr = NULL;
       
        cout << endl << globalTestParams.getNumProcs();
        if (globalTestParams.getNumProcs() == 1) {
            cout << " process took ";
        }
        else {
            cout << " processes took ";
        }
        cout << MPI_Wtime() - startTime 
             << " seconds for all tests" << endl << endl;

        cout << (wasSuccessful ? "SUCCESS" : "FAILURE") << endl;
        delete [] pidArr;
        pidArr = NULL;
    }

    if (stderrFile != NULL) {
        if (fclose(stderrFile)) {
            cerr << "fclose stderrFile failed.";
        }
    }

    /* MPI Finalization */
    MPI::Finalize();

    /* Return error code -1 if the one of test failed. */
    return wasSuccessful ? 0 : -1;
}
