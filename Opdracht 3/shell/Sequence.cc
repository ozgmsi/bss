/** @file Sequence.cc
 * Implementation of class Sequence.
 */
#include <iostream>
#include <sys/wait.h>		// for: wait(), WIF...(status)
#include <unistd.h>			// for: fork(), nice()
#include <fcntl.h>			// for: O_RDONLY, O_CREAT, O_WRONLY, O_APPEND
#include <signal.h>			// for: signal(), SIG*
#include <cstring>			// for: strsignal()
#include "asserts.h"
#include "unix_error.h"
#include "Sequence.h"
using namespace std;


void  Sequence::addPipeline(Pipeline* pp)
{
	require(pp != 0);
	commands.push_back(pp);
}


Sequence::~Sequence()
{
	for(vector<Pipeline*>::iterator  i = commands.begin() ; i != commands.end() ; ++i)
		delete  *i;
}


bool	Sequence::isEmpty()	const
{
	return commands.empty();
}

// Execute the pipelines in this sequence one by one
void	Sequence::execute()
{
	//cerr << "Sequence::execute\n";//DEBUG

	// Execute each pipeline in turn.
	// Also see: fork(2), nice(2), signal(2), wait(2), WIF...(2), strsignal(3)
	size_t  j = commands.size();			// for count-down
	for(vector<Pipeline*>::iterator  i = commands.begin() ; i != commands.end() ; ++i, --j)
	{
		Pipeline*  pp = *i;
		if(!pp->isEmpty())
		{
			if(j == commands.size()) {//DEBUG
				cerr << "Sequence::FIRST PIPELINE\n";//DEBUG
			}//DEBUG

			/// check if pipeline is a built in, if so handle it on the main process.
			if (pp->isBuiltin()){
                pp->execute(); /// execute without forking.
			} else {
                pid_t cpid = fork(); /// fork because, not a built in.
                if (cpid == 0){ /// if we are the child than execute the pipeline
                    signal(SIGINT, SIG_DFL); /// We DON'T want to ignore SIGINT's as a child
                    signal(SIGQUIT, SIG_DFL); /// We DON'T want to ignore SIGQUIT.

                    /// if pipeline has to run in background add increased nice.
                    if (pp->isBackground()){
                        nice(5);
                    }

                    pp->execute();
                } else if (cpid > 0) {
                    /// don't wait for background processes.
                    if (!pp->isBackground()) {
                        int status;
                        pid_t pid = wait(&status);

                        if (WIFSIGNALED(status)) {
                            cout << " Process ID " << pid << " was terminated by signal: " << strsignal(WTERMSIG(status));
                        }
#ifdef WCOREDUMP
                        if (WCOREDUMP(status)) {
                            cout << " (core dumped )";
                        }
#endif // WCOREDUMP
                        cout << endl;
                        /// check if exited with an error.
                        int exitStatus = WEXITSTATUS(status);
                        if (exitStatus > 0){
                            cerr << "Process ID: " << pid << " exited with error status: " << exitStatus << endl;
                        }
                    }
                }
			}
			if(j == 1) {//DEBUG
				cerr << "Sequence::LAST PIPELINE\n";//DEBUG
			} else {//DEBUG
				cerr << "Sequence::WAIT FOR PIPELINE\n";//DEBUG
			}//DEBUG
		}
		// else ignore empty pipeline
	}
}


// vim:ai:aw:ts=4:sw=4:
