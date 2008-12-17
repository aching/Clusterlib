#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <mpi.h>

using namespace std;

const string debugExt = ".out";

int main(int argc, char* argv[]) {
    /* MPI initialization */
    MPI::Init(argc, argv);

    /* MPI set error handler to throw exceptions */
    MPI::COMM_WORLD.Set_errhandler(MPI::ERRORS_THROW_EXCEPTIONS);
    
    /* Get the top level suite from the registry */
    CppUnit::Test *suite = 
	CppUnit::TestFactoryRegistry::getRegistry().makeTest();

    /* Adds the test to the list of test to run */
    CppUnit::TextUi::TestRunner runner;
    runner.addTest( suite );

    /* Change the default outputter to a compiler error format outputter */
    fstream output;
    stringstream fileStringStream;
    fileStringStream << MPI::COMM_WORLD.Get_rank();
    fileStringStream << debugExt;
    output.open(fileStringStream.str().c_str(), fstream::trunc | fstream::out);
    runner.setOutputter(
	new CppUnit::CompilerOutputter(&runner.result(), output));

    /* Run the tests. */
    int wasSuccessful = runner.run();

    /* Check if all processes were successful */
    int *successArr;
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
	delete successArr;

	cout << (wasSuccessful ? "SUCCESS" : "FAILURE")
	     << " - See details in files (0-"
	     << MPI::COMM_WORLD.Get_size() - 1 << ")"
	     << debugExt<< endl;
    }

    output.close();

    /* MPI Finalization */
    MPI::Finalize();

    /* Return error code 1 if the one of test failed. */
    return wasSuccessful ? 0 : 1;
}
