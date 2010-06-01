#include "clusterlibinternal.h"
#include "activenodeparams.h"

using namespace clusterlib;
using namespace std;
using namespace boost;

namespace activenode 
{

/**
 * Default check frequency
 */
static int32_t DefaultCheckMsecs = 15 * 1000;

ActiveNodeParams::ActiveNodeParams() :
    m_checkMsecs(DefaultCheckMsecs),
    m_numProcs(0),
    m_outputType(FILE) 
{
}

void 
ActiveNodeParams::printUsage(char *exec) const
{
    cout <<
"Usage: " << exec <<
" [OPTION]... [VAR=VALUE]...\n\n"
" -h  --help            Display this help and exit.\n"
" -n  --node_location   Select where the active node should be added to.\n"
"                       Use the ';' as a delimeter.  For example, if the\n"
"                       node should be added to 'nodes' group inside the\n"
"                       'test-app' application, pass in 'test-app;nodes\n"
" -z  --zk_server_port  Zookeeper server port list \n"
"                       (i.e. wm301:2181,wm302:2181)\n"
" -p  --num_processes   The number of processes to manage\n"
" -P  --periodic_check  Milliseconds to wait between node checks\n"
"                       (default " << DefaultCheckMsecs << ")\n"
" -N  --node_name       If not set, the node name defaults to the hostname\n"
" -o  --output_type     Choose the output type.\n"
"                       console - console output\n"
"                       file - file output (default)\n";
}

void 
ActiveNodeParams::parseArgs(int argc, char **argv)
{
    static struct option longopts[] =
    {
        {"help", 0, NULL, 'h'},
        {"node_location", 1, NULL, 'n'},
        {"zk_server_port_list", 1, NULL, 'z'},
        {"num_processes", 1, NULL, 'p'},
        {"periodic_check", 1, NULL, 'P'},
        {"node_name", 1, NULL, 'N'},
        {"output_type", 1, NULL, 'o'},
        {0,0,0,0}
    };

    /* Index of current long option into opt_lng array */
    int32_t option_index = 0;
    int32_t err = -1;
    int32_t ret = 0;
    const char *optstring = ":hn:z:p:P:N:o:";

    /* Parse all standard command line arguments */
    while (1) {
        err = getopt_long(argc, argv, optstring, longopts, &option_index);
        if (err == -1) {
            break;
        }
        switch(err) {
            case 'h':
                printUsage(argv[0]);
                exit(-1);
            case 'n':
                split(m_groupVec, optarg, is_any_of(";"));
                if (m_groupVec.begin()->empty()) {
                    m_groupVec.erase(m_groupVec.begin());
                }
                if (m_groupVec.back().empty()) {
                    m_groupVec.pop_back();
                }
                break;
            case 'z':
                m_zkServerPortList = optarg;
                break;
            case 'p':
                m_numProcs = ::atoi(optarg);
                break;
            case 'P':
                m_checkMsecs = ::atoi(optarg);
                if (m_checkMsecs <= 0) {
                    cout << "Invalid -P option (must be > 0)" << endl;
                    exit(-1);
                }
                break;
            case 'N':
                m_nodeName = optarg;
                break;
            case 'o':
                {
                    string arg(optarg);
                    if (arg.compare("console") == 0) {
                        m_outputType = CONSOLE;
                    }
                    else if (arg.compare("file") == 0) {
                        m_outputType = FILE;
                    }
                    else {
                        cout << optarg << " is an unknown output type" << endl;
                        ret = -1;
                    }
                }
                break;
            default:
                cout << "Option -" << static_cast<char>(optopt)
                     << " is invalid" << endl;
                exit(-1);
        }
    }

    if (m_groupVec.empty()) {
        cout << "Option -n needs to be set" << endl;
        printUsage(argv[0]);
        ::exit(-1);
    }
    if (m_zkServerPortList.empty()) {
        cout << "Option -z needs to be set" << endl;
        printUsage(argv[0]);
        ::exit(-1);
    }
    if (m_numProcs <= 0) {
        cout << "Option -p needs to be set and be greater than 0" << endl;
        printUsage(argv[0]);
        ::exit(-1);
    }
    if (m_nodeName.empty()) {
        m_nodeName = ProcessThreadService::getHostname();
    }

    if (m_nodeName.empty()) {
        cout << "Node name is impossibly empty." << endl;
        printUsage(argv[0]);
        ::exit(-1);
    }
}

}
