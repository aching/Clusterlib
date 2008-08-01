/*
 * command.h --
 *
 * Definition of a Command, used to handle an event.
 *
 * $Header:$
 * $Revision:$
 * $Date:$
 */

#ifndef	__COMMAND_H__
#define	__COMMAND_H__

namespace clusterlib
{

/*
 * Interface for event handler command classes.
 */
class Command
{
  public:
    /*
     * Run the command.
     */
    virtual void run(void *payload) = 0;

    /*
     * Clean up.
     */
    virtual void cleanup() = 0;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !__COMMAND_H__ */
