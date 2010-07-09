/*
 * zookeeperperiodiccheck.h --
 *
 * Definition of the ZookeeperPeriodicCheck.
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_ZOOKEEPERPERIODIC_CHECK_H_
#define	_CL_ZOOKEEPERPERIODIC_CHECK_H_

namespace clusterlib
{

/**
 * Checks a Zookeeper instance and reports its data into Clusterlib.
 * The data is stored in a PropertyList called 'Zookeeper instance -
 * <server,port string>'.
 */
class ZookeeperPeriodicCheck : public Periodic
{
  public:
    virtual void run();

    /**
     * Constructor.
     *
     * @param msecsFrequency How many msecs to wait between checks
     * @param registry The comma-separated list of Zookeeper servers
     *        and ports.
     * @param root The Root underwhich the Group will be created.  If NULL, 
     *        then no Notifyable objects will be manipulated.
     */
    ZookeeperPeriodicCheck(int64_t msecsFrequency,
                           const std::string &registry, 
                           Root *root);

    /**
     * Virtual destructor.
     */
    virtual ~ZookeeperPeriodicCheck();

    /**
     * Get the aggregate node state string (should have run run() first).
     *
     * @return A copy of the aggregate node state string
     */
    json::JSONValue::JSONObject getAggNodeState();

  private:
    /**
     * Send a command to a Zookeeper Service and gets back a response.
     *
     * @param host The host to send the command to
     * @param port The port of the host to send the command to
     * @param command The command to be issued
     * @param maxMsecs The maximum amount of msecs to wait for do this command
     * @return The response
     */
    std::string telnetCommand(const std::string &host, 
                              int32_t port, 
                              const std::string &command,
                              int64_t maxMsecs) const;

  private:
    /**
     * Maintains a sock and cleans up during stack unwinding.
     */
    class Sock
    {
      public:
        explicit Sock(int32_t fd)
            : m_fd(fd) {}
        
        int32_t getFd()
        {
            return m_fd;
        }

        void close()
        {
            if (m_fd != -1) {
                ::close(m_fd);
                m_fd = -1;
            }
        }

        ~Sock()
        {
            close();
        }

      private:
        Sock(const Sock &sock);
        Sock &operator= (const Sock &sock);

        /**
         * The file descriptor.
         */
        int32_t m_fd;
    };

    /**
     * The host and ports parsed out of the server string.
     */
    std::vector<std::pair<std::string, int32_t> > m_hostPortVec;

    /**
     * Application for storing the information.
     */
    Application *m_application;

    /**
     * The Zookeeper aggregate service state.
     */
    json::JSONValue::JSONObject m_aggNodeStateObj;

    /**
     * Nodes that are being watched and owned.
     */
    std::vector<Node *> m_nodeVec;

    /**
     * Make this object thread-safe.
     */
    Mutex m_lock;
};
    
}

#endif	/* !_CL_ZOOKEEPERPERIODIC_CHECK_H_ */
