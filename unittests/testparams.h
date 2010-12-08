/*
 * testparams.h --
 *
 * Contains the test parameters.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_TESTPARAMS_H_
#define _CL_TESTPARAMS_H_

/**
 * Global test parameters class to be shared with all tests
 */
class TestParams
{
  public:
    enum OutputType {
        CONSOLE,
        FILE
    };

    TestParams() :
        m_myId(-1),
        m_numProcs(-1),
        m_zkServerPortList("localhost:2221"),
        m_outputType(FILE),
        m_parseArgsState(-1),
        m_clPropertyList("unittests"),
        m_updateClPropertyList(false),
        m_testCount(0) {}

    /**
     * Virtual destructor.
     */
    virtual ~TestParams() {}

    TestParams(int32_t myId, int32_t numProcs) :
        m_myId(myId),
        m_numProcs(numProcs),
        m_zkServerPortList("localhost:2221"),
        m_outputType(FILE),
        m_parseArgsState(-1),
        m_clPropertyList("unittests"),
        m_updateClPropertyList(false),
        m_testCount(0) {}

    /**
     * Default print usage command.  Intended to be overriden and/or
     * used by subclasses.
     *
     * @param exec Executable string
     */
    virtual void printUsage(char *exec) const;

    /**
     * Only the root process will actually parse the arguments.
     * Intended to be overriden and/or used by subclasses.
     *
     * @param argc Number of argument in argv
     * @param argv Argument vector of char *
     * @return 0 if success, otherwise failure
     */
    virtual int32_t rootParseArgs(int argc, char **argv);

    /**
     * The root process will scatter the arguments to the other
     * processes.  Intended to be overriden and/or used by subclasses.
     *
     * @return 0 if success, otherwise failure
     */
    virtual int32_t scatterArgs();

    /**
     * Remove the old property list if around from last test.
     */
    void resetClPropertyList();

    /**
     * Increment the test count.
     */
    void incrTestCount();
    
    /**
     * Get the test count.
     * 
     * @return the test count
     */
    int32_t getTestCount() const { return m_testCount; }

    int32_t getMyId() const { return m_myId; }

    void setMyId(int32_t myId) { m_myId = myId; }

    int32_t getNumProcs() const { return m_numProcs; }

    void setNumProcs(int32_t numProcs) { m_numProcs = numProcs; }

    const std::string &getXmlOutputFile() { return m_xmlOutputFile; }

    bool root() const { return m_myId == 0; }

    const std::string &getZkServerPortList() const 
    { 
        return m_zkServerPortList; 
    }

    const std::string &getTestFixtureName() { return m_testFixtureName; }

    OutputType getOutputType() const { return m_outputType; }

    const std::string & getClPropertyList() const { return m_clPropertyList; }

    bool getUpdateClPropertyList() const { return m_updateClPropertyList; }

  private:
    /**
     * The rank of this proccess.
     */
    int32_t m_myId;

    /**
     * The number of processes to run this test with
     */
    int32_t m_numProcs;

    /**
     * If not empty, write an XML output file of results
     */
    std::string m_xmlOutputFile;

    /** 
     * The command separated list of ZooKeeper Servers
     * i.e. (wmdev1008:2181,wmdev1007:2181)
     */
    std::string m_zkServerPortList;

    /**
     * The name of one test fixture.  It is empty if all tests fixtures
     * are desired.
     */
    std::string m_testFixtureName;

    /**
     *  The name of a single test within a test fixture.  If it is not
     *  empty, then only this test will be run.
     */
    std::string m_testName;

    /**
     * Output type 
     */
    OutputType m_outputType;

    /**
     * -1 for failure, 0 for success
     */
    int32_t m_parseArgsState;

    /**
     * Special clusterlib output property list
     */
    const std::string m_clPropertyList;

    /**
     * Update the clusterlib output property list
     */
    int32_t m_updateClPropertyList;

    /**
     * The test count.
     */
    int32_t m_testCount;
};

#endif
