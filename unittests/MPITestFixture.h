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
    void barrier()
    {
	/* Synchronize */
	MPI::COMM_WORLD.Barrier();
    }
    int getRank() { return _rank; }
    int getSize() { return _size; }
    void waitsForOrder(clusterlib::Factory *factory,
		       int procFirst, int procNext)
    {
	assert(procFirst < getSize());
	assert(procNext < getSize());
	assert(procFirst != procNext);

	if (getRank() == procFirst) {
	    factory->synchronize();
	    MPI::COMM_WORLD.Ssend(NULL, 0, MPI_BYTE, procNext, MPI_TAG);
	}
	else if (getRank() == procNext) {
	    MPI::COMM_WORLD.Recv(NULL, 0, MPI_BYTE, procFirst, MPI_TAG);
	    factory->synchronize();
	}
    }

  private:
    int _rank;
    int _size;
};

#endif
