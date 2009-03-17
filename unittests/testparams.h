#include <iostream>
#include <mpi.h>
#include <getopt.h>

/**
 * Global test parameters class to be shared with all tests
 */
class TestParams {
  public:
    TestParams() :
        m_myId(-1),
        m_numProcs(-1),
        m_zkServerPortList("localhost:2221"),
        m_parseArgsState(-1) {}
    TestParams(int32_t myId, int32_t numProcs) :
        m_myId(myId),
        m_numProcs(numProcs),
        m_zkServerPortList("localhost:2221"),
        m_parseArgsState(-1) {}
    void printUsage(char *exec) const;
    /**
     * Only the root process will actually parse the arguments
     */
    int32_t rootParseArgs(int argc, char **argv);
    /**
     * The root process will scatter the arguments to the other
     * processes.
     */
    int32_t scatterArgs();

    int32_t getMyId() const { return m_myId; }
    void setMyId(int32_t myId) { m_myId = myId; }
    int32_t getNumProcs() const { return m_numProcs; }
    void setNumProcs(int32_t numProcs) { m_numProcs = numProcs; }
    bool root() const { return m_myId == 0; }
    const std::string &getZkServerPortList() const 
    { 
        return m_zkServerPortList; 
    }
  private:
    int32_t m_myId;
    int32_t m_numProcs;
    /** 
     * The command separated list of ZooKeeper Servers
     * i.e. (wmdev1008:2181,wmdev1007:2181)
     */
    std::string m_zkServerPortList;
    /**
     * -1 for failure, 0 for success
     */
    int32_t m_parseArgsState;
};
