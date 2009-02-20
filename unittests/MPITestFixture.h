#ifndef MPITESTFIXTURE_H
#define MPITESTFIXTURE_H

#include <mpi.h>
#include <cppunit/extensions/HelperMacros.h>
#include <clusterlib.h>

#define MPI_TAG 1000

class MPITestFixture : public CppUnit::TestFixture {
  public:
    MPITestFixture() : 
	_rank(MPI::COMM_WORLD.Get_rank()),
	_size(MPI::COMM_WORLD.Get_size()) {}
    /** 
     * Ensure that all processes are synchronized.  Doesn't guarantee
     * clusterlib event synchorization. 
     */
    void barrier()
    {
	/* Synchronize */
	MPI::COMM_WORLD.Barrier();
    }
    /** 
     * Get the rank of your process.  If MPI was started with n
     * processes, the rank will be in the range of 0 to n-1.
     */
    int getRank() { return _rank; }
    /** Get the number of processes started */
    int getSize() { return _size; }
    /** 
     * Ensures that all clusterlib events that have been seen by
     * process procFirst have been seen by procNext.
     */
    void waitsForOrder(clusterlib::Factory *factory,
		       int procFirst, int procNext)
    {
	assert(procFirst < getSize());
	assert(procNext < getSize());
	assert(procFirst != procNext);

	if (getRank() == procFirst) {
	    factory->synchronize();
	    MPI::COMM_WORLD.Ssend(NULL, 0, MPI::BYTE, procNext, MPI_TAG);
	}
	else if (getRank() == procNext) {
	    MPI::COMM_WORLD.Recv(NULL, 0, MPI::BYTE, procFirst, MPI_TAG);
	    factory->synchronize();
	}
    }
    /** 
     * Ensures that all clusterlib events that have been seen by
     * process procFirst have been seen by all processes.
     */
    void allWaitsForOrder(clusterlib::Factory *factory,
                          int procFirst) 
    {
        assert(procFirst < getSize());
        if (getRank() == procFirst) {
	    factory->synchronize();
        }
        MPI::COMM_WORLD.Bcast(NULL, 0, MPI::BYTE, procFirst);
        if (getRank() != procFirst) {
            factory->synchronize();
        }
    }

  private:
    int _rank;
    int _size;
};

#endif
