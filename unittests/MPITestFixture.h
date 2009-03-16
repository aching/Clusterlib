#ifndef MPITESTFIXTURE_H
#define MPITESTFIXTURE_H

#include <mpi.h>
#include <cppunit/extensions/HelperMacros.h>
#include <clusterlib.h>

#define MPI_TAG 1000

/**
 * Must call this prior to any test being run. It synchronizes
 * clusterlib and decides whether this process will execute any code
 * below it.  Every test can be categorized into:
 * 
 * 1) Single process 
 *    - only one process allowed here, all others are don
 *     _minSize = 1, _singleProcessMode doesn't matter
 * 2) Multiple processes 
 *    - N processes required.  If requirement is not met
 *      then test automatically completes.
 *     _minSize = N, _singleProcessMode is false
 * 3) Multiple process falls back to single process
 *    - N processes desired, but if requirement is not met, a single 
 *      process does each step.  This requires that each "stage" of the 
 *      is not dependent on data from a previous stage.
 *     _minSize = N, _singleProcessMode is true
 *
 * If this process does not meet the requirements, it will return from
 * the function and wait for the next test.
 *
 * @param _minSize if not -1, requires at least this many processes.  Otherwise
                   runs with any amount of processes
 * @param _singleProcessMode supports running when only one process exists 
 *                           by having that one process do all work
 * @param _factory the factory pointer (if NULL, barrier is called without 
 *                 clusterlib)
 */
#define INIT_BARRIER_MPI_TEST_OR_DONE(_minSize, _singleProcessMode, _factory) \
{ \
    setTestSingleProcessMode(_singleProcessMode); \
    setTestMinSize(_minSize); \
    if (_factory) { \
        barrier(_factory, true); \
    } \
    else { \
        barrier(NULL, false); \
    } \
    if ((!_singleProcessMode) && \
        (getSize() < _minSize) && \
        (_minSize != -1)) { \
        cerr << "test: done" << endl; \
        return; \
    } \
} \
 
class MPITestFixture : public CppUnit::TestFixture {
  public:
    MPITestFixture() : 
	_rank(MPI::COMM_WORLD.Get_rank()),
	_size(MPI::COMM_WORLD.Get_size()),
        _testSingleProcessMode(false),
        _testMinSize(-1) {}

    /** 
     * Ensure that all processes have reached this point before
     * continuing.  Only guarantees clusterlib event synchronization
     * if parameters are set appropriately.
     *
     * @param factory pointer to the factory
     * @param clusterlibSync ensure that all processes have received 
     *        all cluserlib updates
     */
    void barrier(clusterlib::Factory *factory = NULL, 
                 bool clusterlibSync = false)
    {
	/* Synchronize */
	MPI::COMM_WORLD.Barrier();
        if (clusterlibSync) {
            assert(factory);
            factory->synchronize();
            MPI::COMM_WORLD.Barrier();
            factory->synchronize();
            MPI::COMM_WORLD.Barrier();
        }
    }

    /**
     * Determine if my rank matches the rank parameter.  It returns
     * true only if the ranks match or (singleProcessMode is set and
     * we don't have enough processes).
     *
     * @param rank check to see if my rank matches this
     */
    bool isMyRank(int rank) 
    {
        /*
         * Return true if singleTestProcessMode is set and there
         * aren't enough processes.
         */
        if (getTestSingleProcessMode() &&
            (getTestMinSize() != -1) &&
            (getTestMinSize() > getSize())) {
            return true;
        }

        if (getRank() == rank) {
            return true;
        }
        
        return false;
    }

    /** 
     * Ensures that one process waits until another has signaled it.
     * If clusterlibSync is set, all clusterlib events that have been
     * seen by process procFirst have been seen by procSecond.  If
     * singleProcessMode was set and there aren't enough processes, it
     * simply bypasses this.
     *
     * @param procFirst the first process that sends a message when completed
     * @param procSecond the process that waits for procFirst
     * @param factory the factory object
     * @param clusterlibSync ensure that all processes have received 
     *        all cluserlib updates
     */
    void waitsForOrder(int procFirst, 
                       int procSecond,
                       clusterlib::Factory *factory = NULL,
                       bool clusterlibSync = false)
    {
        /*
         * Don't do anything messasing for single process mode when
         * there aren't enough processes.
         */
        if (getTestSingleProcessMode() &&
            (getTestMinSize() != -1) &&
            (getTestMinSize() > getSize())) {
            if (clusterlibSync) {
                factory->synchronize();
            }

            return;
        }

	assert((procFirst < getSize()) && (procFirst >= 0));
	assert((procSecond < getSize()) && (procSecond >= 0));
	assert(procFirst != procSecond);

	if (getRank() == procFirst) {
            if (clusterlibSync) {
                factory->synchronize();
            }
	    MPI::COMM_WORLD.Ssend(NULL, 0, MPI::BYTE, procSecond, MPI_TAG);
	}
	else if (getRank() == procSecond) {
	    MPI::COMM_WORLD.Recv(NULL, 0, MPI::BYTE, procFirst, MPI_TAG);
            if (clusterlibSync) {
                factory->synchronize();
            }
	}
    }

    /** 
     * Ensures that all processes (except procFirst) wait until
     * another has signaled it.  If clusterlibSync is set, all
     * clusterlib events that have been seen by process procFirst have
     * been seen by all other processes.  If singleProcessMode was set
     * and there aren't enough processes, it simply bypasses this.
     *
     * @param procFirst the first process that sends a message when completed
     * @param factory the factory object
     * @param clusterlibSync ensure that all processes have received 
     *        all cluserlib updates
     * @param singleProcessMode supports running when only one process exists 
     *                          by having one process do all work
     */
    void allWaitsForOrder(int procFirst,
                          clusterlib::Factory *factory,
                          bool clusterlibSync = false)
    {
        /*
         * Don't do anything messaing for single process mode when
         * there aren't enough processes.
         */
        if (getTestSingleProcessMode() &&
            (getTestMinSize() != -1) &&
            (getTestMinSize() > getSize())) {
            if (clusterlibSync) {
                factory->synchronize();
            }

            return;
        }

        assert(procFirst < getSize());
        if (getRank() == procFirst) {
            if (clusterlibSync) {
                factory->synchronize();
            }
        }
        MPI::COMM_WORLD.Bcast(NULL, 0, MPI::BYTE, procFirst);
        if (getRank() != procFirst) {
            if (clusterlibSync) {
                factory->synchronize();
            }
        }
    }

    bool getTestSingleProcessMode() const 
    {
        return _testSingleProcessMode; 
    }

    /*
     * Do not use this function directly, it helps
     * INIT_BARRIER_MPI_TEST_OR_DONE
     */
    void setTestSingleProcessMode(bool testSingleProcessMode)
    { 
        _testSingleProcessMode = testSingleProcessMode;
    }

    /*
     * Do not use this function directly, it helps
     * INIT_BARRIER_MPI_TEST_OR_DONE
     */
    void setTestMinSize(int testMinSize)
    { 
        _testMinSize = testMinSize;
    }

    /*
     * Do not use this function directly, it helps
     * INIT_BARRIER_MPI_TEST_OR_DONE
     */
    int getSize() const { return _size; }

  private:
    /** 
     * Get the rank of your process.  If MPI was started with n
     * processes, the rank will be in the range of 0 to n-1.
     */
    int getRank() const { return _rank; }

    int getTestMinSize() const { return _testMinSize; }

    /**
     * My process rank
     */
    int _rank;

    /**
     * Total number of processes available
     */
    int _size;

    /*
     * Should be initialized by each test trhough
     * INIT_BARRIER_MPI_TEST_OR_DONE.  Does this test support single
     * process mode?
     */
    bool _testSingleProcessMode;

    /*
     * Should be initialized by each test trhough
     * INIT_BARRIER_MPI_TEST_OR_DONE.  How many processes are required
     * for this test to run?
     */     
    int _testMinSize;
};

#endif
