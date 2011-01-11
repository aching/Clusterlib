/*
 * Copyright (c) 2010 Yahoo! Inc. All rights reserved. Licensed under
 * the Apache License, Version 2.0 (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License. See accompanying
 * LICENSE file.
 * 
 * $Id$
 */

#ifndef MPITESTFIXTURE_H
#define MPITESTFIXTURE_H

#include <mpi.h>
#include <cppunit/extensions/HelperMacros.h>
#include <clusterlib.h>
#include <iomanip>

#define MPI_TAG 1000

/**
 * Replacement for CPPUNIT_ASSERT() that will record error points but
 * not exit the function it is called in.  You can call this anywhere
 * in the test and it will record the error point and move foward.
 * Finally, in cleanAndBarrierMPITest() the errors will be printed.
 * Therefore, do not do any group communication after
 * cleanAndBarrierMPITest() is called.  This macro solves the problem
 * where group communication is expected by CPPUNIT_ASSERT() but
 * CPPUNIT_ASSERT exits the test function without executing the
 * communication expected.
 */

#define MPI_CPPUNIT_ASSERT(_condition) \
{ \
    if (!(_condition)) { \
        addErrorFileLine(__FILE__, __LINE__); \
    } \
} 

/**
 * MPI-enhanced test fixture.
 *
 * All tests fall into 3 categories.
 * 1) Run with exactly N processes
 *    - To guarantee that only these processes are interacting, wrap all 
 *      logic with isMyRank() code blocks.
 *    - When not enough processes are present, all isMyRank() will return false
 *      and waitsForOrder() and allWaitsForOrder() will return immediately.
 *    - Make sure to set singleProcessMode to false.
 * 2) Run with either N or 1 proccess
 *    - If N processes are not met, the all logic will only match process 0.
 *    - Make sure to set singleProcessMode to true.
 *    - Wrap all logic with isMyRank() 
 * 3) Run with any number of processes
 *    - Can use isMyRank() but isn't required as it is intended for all 
 *      available processes
 *    - Make sure to set minSize to -1.  singleProcessMode is irrelevant.
 */
class MPITestFixture : public CppUnit::TestFixture
{
  public:
    /**
     * Constructor.
     *
     * @param testParams the test parameters used for this test
     */
    MPITestFixture(TestParams &testParams) : 
	m_rank(MPI::COMM_WORLD.Get_rank()),
	m_size(MPI::COMM_WORLD.Get_size()),
        m_testSingleProcessMode(false),
        m_testMinSize(-1),
        m_clPropertyList(testParams.getClPropertyList()),
        m_updateClPropertyList(testParams.getUpdateClPropertyList()),
        m_testCount(testParams.getTestCount())
    {
        testParams.incrTestCount();
        const int32_t bufLen = 256;
        char tmp[bufLen + 1];
        tmp[bufLen] = '\0';
        if (gethostname(tmp, bufLen) == 0) {
            m_hostname = tmp;
        }
    }

    virtual ~MPITestFixture() {}

    /**
     * Generate a test key that keep tracks of the count.
     * 
     * @return the test key
     */
    std::string genTestKey()
    {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(3) << m_testCount 
           <<  ":" << m_testName;
        return ss.str();
    }

    /**
     * Make a unique ID to represent this process in the clusterlib
     * property list.
     */
    std::string genPropertyListId()
    {
        std::stringstream ss;
        ss << m_rank << ":" << m_hostname;
        return ss.str();
    }

    /**
     * Must be called prior to the beginning of any test.  It barriers
     * and sets up the arguments to be used for the remainder of the
     * test.
     *
     * @param minSize If == -1, it runs with any number of processes. If 
     *        != -1, it requires at least this many processes.
     * @param singleProcessMode supports running when only less than minSize 
     *        processes exist.  Process 0 is the only process that meets all
     *        isMyRank() and allWaitsForOrder() conditions.
     * @param factory the factory pointer (if NULL, barrier is called without 
     *        the clusterlib sync)
     * @param clusterlibSync use the clusterlib sync with the barrier?
     * @param testName if not empty, prints a "testName: initialized"
     */
    void initializeAndBarrierMPITest(int32_t minSize, 
                                     bool singleProcessMode, 
                                     clusterlib::Factory *factory,
                                     bool clusterlibSync,
                                     std::string testName)
    {
        MPI_CPPUNIT_ASSERT(minSize >= -1);
        m_testMinSize = minSize;
        m_testSingleProcessMode = singleProcessMode;
        m_testName = testName;

        if (m_testName.empty() == false) {
            std::cerr << "===== " << m_testName << ": initialized =====" 
                      << std::endl;
        }

        if (factory) {
            if (m_updateClPropertyList) {
                /* Make sure all updates have been seen! */
                factory->synchronize();
                boost::shared_ptr<clusterlib::Root> rootSP = 
                    factory->createClient()->getRoot();
                boost::shared_ptr<clusterlib::PropertyList> propertyListSP =
                    rootSP->getPropertyList(
                        m_clPropertyList, 
                        clusterlib::CREATE_IF_NOT_FOUND);
                bool done = false;
                /* 
                 * publish() can fail if another process publishes at
                 * the same time.
                 */
                while (!done) {
                    json::JSONValue jsonValue;
                    
                    clusterlib::NotifyableLocker l(
                        propertyListSP,
                        clusterlib::CLString::NOTIFYABLE_LOCK,
                        clusterlib::DIST_LOCK_EXCL);
                    
                    bool exists = propertyListSP->cachedKeyValues().get(
                        genTestKey(), jsonValue);
                    std::string value;
                    if (exists) {
                        value = 
                            jsonValue.get<json::JSONValue::JSONString>();
                    }
                    value.append(" ");
                    value.append(genPropertyListId());
                    propertyListSP->cachedKeyValues().set(
                        genTestKey(), value);
                    propertyListSP->cachedKeyValues().publish();
                    done = true;
                }
            }
            barrier(factory, true);
        }
        else {
            barrier(NULL, false);
        }
    }

    /**
     * Some tests must end prior to the cleanAndBarrierMPITest().
     * Therefore, calling this will clean up their clusterlib property
     * list data.  The preferred way however is through
     * cleanAndBarrierMPITest().
     */
    void finishedClTest(clusterlib::Factory *factory)
    {
        assert(factory);

        if (m_updateClPropertyList) {
            boost::shared_ptr<clusterlib::Root> rootSP = 
                factory->createClient()->getRoot();
            boost::shared_ptr<clusterlib::PropertyList> propertyListSP =
                rootSP->getPropertyList(m_clPropertyList, 
                                        clusterlib::CREATE_IF_NOT_FOUND);
            std::stringstream ss;
            ss << " " << m_rank << ":" << m_hostname;
            bool done = false;
            /* 
             * publishProperties() can fail if another process publishes at
             * the same time.
             */
            while (!done) {
                json::JSONValue jsonValue;

                clusterlib::NotifyableLocker l(
                    propertyListSP,
                    clusterlib::CLString::NOTIFYABLE_LOCK,
                    clusterlib::DIST_LOCK_EXCL);
                
                bool exists = propertyListSP->cachedKeyValues().get(
                    genTestKey(), jsonValue);
                std::string value;
                if (exists) {
                    value = jsonValue.get<json::JSONValue::JSONString>();
                }
                size_t index = value.find(genPropertyListId());
                if (index != std::string::npos) {
                    value.erase(index, genPropertyListId().size());
                    propertyListSP->cachedKeyValues().set(
                        genTestKey(), value);
                    propertyListSP->cachedKeyValues().publish();
                }
                done = true;
            }
        }
    }
    
    /**
     * Must be called prior to the ending of any test.  It barriers so
     * that all outstanding clusterlib events are propagated before
     * the end of the test.  It also resets the minSize and testName.
     * It should be called in the tearDown() method.  It prints the
     * test completed output if set in initializeAndBarrierMPITest().
     * No function should be called after it in the tearDown() method.
     *
     * @param factory the factory pointer (if NULL, barrier is called without 
     *        the clusterlib sync)
     * @param clusterlibSync use the clusterlib sync with the barrier?
     */
    void cleanAndBarrierMPITest(clusterlib::Factory *factory,
                                bool clusterlibSync)
    {
        if (factory) {
            finishedClTest(factory);
            barrier(factory, true);
        }
        else {
            barrier(NULL, false);
        }
        
        if (m_testName.empty() == false) {
            std::cerr << "===== " << m_testName << ": finished =====" 
                      << std::endl;
        }

        /*
         * Reset test parameters
         */
        m_testMinSize = -1;
        m_testName.clear();

        /*
         * Print out the errors if there were any with
         * CPPUNIT_ASSERT_MESSAGE.
         */
        if (m_testError.empty() == false) {
            std::string testError(m_testError);
            m_testError.clear();
            CPPUNIT_ASSERT_MESSAGE(testError, false);
        }
    }

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
        if (clusterlibSync && factory) {
            factory->synchronize();
            MPI::COMM_WORLD.Barrier();
            factory->synchronize();
            MPI::COMM_WORLD.Barrier();
        }
    }

    /**
     * Determine if my rank matches the rank parameter.  It returns
     * true only if the ranks match or (this is rank 0 and
     * singleProcessMode is set and we don't have enough processes).
     *
     * @param rank check to see if my rank matches this
     */
    bool isMyRank(int rank) 
    {
        /*
         * Return true if singleTestProcessMode is set and there
         * aren't enough processes and this is process 0.
         */
        if (getTestSingleProcessMode() &&
            (getTestMinSize() != -1) &&
            (getTestMinSize() > getSize()) &&
            (getRank() == 0)) {
            return true;
        }
        else if (getRank() == rank) {
            return true;
        }
        
        return false;
    }

    /** 
     * Ensures that one process waits until another has signaled it.
     * If clusterlibSync is set, all clusterlib events that have been
     * seen by process procFirst have been seen by procSecond.  If
     * singleProcessMode was set and there aren't enough processes, it
     * simply bypasses the messaging.
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
         * Don't do anything messaging for single process mode when
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

        std::cerr << "waitsForOrder: proper waiting done with " 
                  << getTestMinSize()
                  << " processes exist and need " << getSize() << std::endl;
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

    /**
     * Do not use this function directly, it helps
     * INIT_BARRIER_MPI_TEST_OR_DONE
     */
    bool getTestSingleProcessMode() const 
    {
        return m_testSingleProcessMode; 
    }

    /**
     * Do not use this function directly, it helps
     * INIT_BARRIER_MPI_TEST_OR_DONE
     */
    void setTestSingleProcessMode(bool testSingleProcessMode)
    { 
        m_testSingleProcessMode = testSingleProcessMode;
    }

    /**
     * Do not use this function directly, it helps
     * INIT_BARRIER_MPI_TEST_OR_DONE
     */
    void setTestMinSize(int testMinSize)
    { 
        m_testMinSize = testMinSize;
    }

    /**
     * Do not use this function directly, it helps
     * INIT_BARRIER_MPI_TEST_OR_DONE
     */
    int getSize() const { return m_size; }

    /** 
     * Get the rank of your process.  If MPI was started with n
     * processes, the rank will be in the range of 0 to n-1.
     */
    int getRank() const { return m_rank; }

    /**
     * Add an error message for this test for file and line number.
     * Used by MPI_CPPUNIT_ASSERT.
     *
     * @param file name of the file the error happened in
     * @param line the line number where the error happened.
     */
    void addErrorFileLine(const std::string &file, int32_t line)
    {
        std::stringstream ss;
        ss << "Assert failed in file: " << file << ", line: " 
           << line << std::endl;
        if (m_testError.empty()) {
            m_testError.append("\n");
        }
        std::cerr << ss.str();
        m_testError.append(ss.str());
    }

  private:

    /**
     * Get the minimum test size
     * 
     * @return the minimum test size
     */
    int getTestMinSize() const { return m_testMinSize; }

    /**
     * My process rank
     */
    int m_rank;

    /**
     * Total number of processes available
     */
    int m_size;

    /**
     * Should be initialized by each test trhough
     * INIT_BARRIER_MPI_TEST_OR_DONE.  Does this test support single
     * process mode?
     */
    bool m_testSingleProcessMode;

    /**
     * Should be initialized by each test trhough
     * INIT_BARRIER_MPI_TEST_OR_DONE.  How many processes are required
     * for this test to run?
     */     
    int m_testMinSize;

    /**
     * Should be initialized by each test through
     * INIT_BARRIER_MPI_TEST_OR_DONE. Name of this test.
     */
    std::string m_testName;

    /**
     * This saves the first error that this test ran into.  It should
     * be outputted by the cleanAndBarrierMPITest() in the tearDown().
     */
    std::string m_testError;

    /**
     * Special clusterlib output property list
     */
    std::string m_clPropertyList;

    /**
     * Update the clusterlib output property list
     */
    bool m_updateClPropertyList;

    /**
     * What test is this?
     */
    int32_t m_testCount;

    /**
     * My hostname.
     */
    std::string m_hostname;
};

#endif
