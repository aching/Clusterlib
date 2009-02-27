#include "testparams.h"

using namespace std;

void TestParams::printUsage(char *exec) const
{
    cout <<
"Usage: " << exec << " [OPTION]... [VAR=VALUE]...\n\n"
" -h  --help            Display this help and exit.\n"
" -z  --zk_server_port  Zookeeper server port list \n"
"                       (i.e. wm301:2181,wm302:2181 (default %s).\n"
         << endl;
}

int32_t TestParams::rootParseArgs(int argc, char **argv)
{
    /* Do not execute unless you are the root process */
    if (m_myId != 0) {
        return 0;
    }

    static struct option longopts[] =
    {
        {"help", 0, NULL, 'h'},
        {"zk_server_port_list", 1, NULL, 'z'},
        {0,0,0,0}
    };

    /* Index of current long option into opt_lng array */
    int32_t option_index = 0;
    int32_t err = -1;
    int32_t ret = 0;
    const char *optstring = ":hz:";
    
    /* Parse all standard command line arguments */
    while (1) {
        err = getopt_long(argc, argv, optstring, longopts, &option_index);
        if (err == -1) {
            break;
        }
        switch(err) {
            case 'h':
                printUsage(argv[0]);
                return -1;
            case 'z':
                m_zkServerPortList = optarg;
                break;
            default:
                if (getMyId() == 0) {
                    fprintf(stderr, "Option -%c is invalid\n",
                            (char) optopt);
                }
                ret = -1;
        }
    }
    
    if (!m_zkServerPortList.compare("localhost:2181")) {
        cout << "-z option not used.  Using default " 
             << "localhost:2181"
             << endl;
    }

    m_parseArgsState = ret;
    return ret;
}

int32_t TestParams::scatterArgs()
{
    /* Check for successful argument parsing on the root process */
    MPI::COMM_WORLD.Bcast(&m_parseArgsState, 1, MPI::INT, 0);
    if (m_parseArgsState != 0) {
        return -1;
    }

    /* Broadcast the m_zkServerPortList */
    int32_t zkServerPortListSize;
    if (root()) {
        /* Extra for end of string character */
        zkServerPortListSize = m_zkServerPortList.size() + 1;
    }        
    MPI::COMM_WORLD.Bcast(&zkServerPortListSize, 1, MPI::INT, 0);
    if (root()) {
        MPI::COMM_WORLD.Bcast(
            const_cast<char *>(m_zkServerPortList.c_str()),
            zkServerPortListSize, 
            MPI::CHAR, 
            0);
    }
    else {
        char *tmpZkServerPortList = new char[zkServerPortListSize];
        MPI::COMM_WORLD.Bcast(
            tmpZkServerPortList,
            zkServerPortListSize, 
            MPI::CHAR, 
            0);
        m_zkServerPortList = tmpZkServerPortList;
        delete [] tmpZkServerPortList;
    }

    return 0;
}
