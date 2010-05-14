/*
 * activenodeparams.h --
 *
 * Definition of class ActiveNodeParams; it manages the parameters of
 * the active node process.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_ACTIVENODEPARAMS_H_
#define _CL_ACTIVENODEPARAMS_H_

#include <iostream>
#include <vector>
#include <getopt.h>

namespace activenode {

/**
 * Active node test parameters
 */
class ActiveNodeParams 
{
  public:
    /**
     * Output to console or file
     */
    enum OutputType {
        CONSOLE,
        FILE
    };

    /**
     * Constructor
     */
    ActiveNodeParams() :
        m_numProcs(0),
        m_outputType(FILE) {}

    /**
     * Print the usage
     *
     * @param exec the executable name
     */
    void printUsage(char *exec) const;

    /**
     * Parse the arguments.
     *
     * @param argc the number of arguments
     * @param argv the vector of arguments
     */
    void parseArgs(int argc, char **argv);
    
    /**
     * Get the number of processes to allow on this node.
     *
     * @return the number of processes allowed
     */
    int32_t getNumProcs() const { return m_numProcs; }

    /**
     * Set the number of processes to allow on this node.
     *
     * @param numProcs the number of processes allowed
     */
    void setNumProcs(int32_t numProcs) { m_numProcs = numProcs; }

    /**
     * Get the progression of groups as a vector.
     *
     * @return the groups as a vector
     */
    const std::vector<std::string> &getGroupsVec() const 
    {
        return m_groupsVec; 
    }

    /**
     * Get the zookeeper server port list as a comma separated string.
     *
     * @return the string of zookeeper servers (comma delimited)
     */
    const std::string &getZkServerPortList() const 
    { 
        return m_zkServerPortList; 
    }

    /**
     * Get the output type (console or file)
     *
     * @return the output type
     */
    OutputType getOutputType() const { return m_outputType; }

    /**
     * Get the node name used.
     */
    const std::string &getNodeName() const { return m_nodeName; }

  private:
    /**
     * The name of the node (either defaulting to the hostname or a
     * user-specified name).
     */
    std::string m_nodeName;

    /**
     * The number of processes to run this test with
     */
    int32_t m_numProcs;

    /**
     * Vector of the groups hierarchy that the node belongs in. 
     */
    std::vector<std::string> m_groupsVec;

    /** 
     * The command separated list of ZooKeeper Servers
     * i.e. (wmdev1008:2181,wmdev1007:2181)
     */
    std::string m_zkServerPortList;

    /**
     * Output type 
     */
    OutputType m_outputType;
};

}

#endif	/* !_CL_ACTIVENODEPARAMS_H__H_ */
