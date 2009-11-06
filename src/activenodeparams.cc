#include "clusterlibinternal.h"
#include "activenodeparams.h"

using namespace std;
using namespace boost;

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
        {"output_type", 1, NULL, 'o'},
        {0,0,0,0}
    };

    /* Index of current long option into opt_lng array */
    int32_t option_index = 0;
    int32_t err = -1;
    int32_t ret = 0;
    const char *optstring = ":hn:z:p:o:";

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
                split(m_groupsVec, optarg, is_any_of(";"));
                if (m_groupsVec.begin()->empty()) {
                    m_groupsVec.erase(m_groupsVec.begin());
                }
                if (m_groupsVec.back().empty()) {
                    m_groupsVec.pop_back();
                }
                break;
            case 'z':
                m_zkServerPortList = optarg;
                break;
            case 'p':
                m_numProcs = ::atoi(optarg);
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
                cout << "Option -" 
                     << static_cast<char>(optopt) 
                     << " is invalid" << endl;
                exit(-1);
        }
    }

    if (m_groupsVec.empty()) {
        cout << "Option -n needs to be set" << endl;
        printUsage(argv[0]);
        ::exit(-1);
    }
    if (m_zkServerPortList.empty()) {
        cout << "Option -z needs to be set" << endl;;
        printUsage(argv[0]);
        ::exit(-1);
    }
    if (m_numProcs <= 0) {
        cout << "Option -p needs to be set and be greater than 0" << endl;;
        printUsage(argv[0]);
        ::exit(-1);
    }
}

