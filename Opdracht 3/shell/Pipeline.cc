/** @file Pipeline.cc
 * Implementation of class Pipeline.
 */
#include <iostream>
#include <unistd.h>		// for: pipe(), fork(), dup2(), close()
#include "asserts.h"
#include "unix_error.h"
#include "Pipeline.h"
using namespace std;


Pipeline::Pipeline()
	: background(false)
{
}


void	Pipeline::addCommand(Command *cp)
{
	require(cp != 0);
	commands.push_back(cp);
}


Pipeline::~Pipeline()
{
	for (vector<Command*>::iterator  i = commands.begin() ; i != commands.end() ; ++i)
		delete  *i;
}


bool	Pipeline::isEmpty()	const
{
	return commands.empty();
}


// Execute the commands in this pipeline in parallel
void	Pipeline::execute()
{
	//cerr << "Pipeline::execute\n";//DEBUG

	// Because we want the shell to wait on the rightmost process only
	// we must created the various child processes from the right to the left.
	// Also see: pipe(2), fork(2), dup2(2), dup(2), close(2), open(2).
	// And maybe usefull for debugging: getpid(2), getppid(2).
	size_t	 j = commands.size();		// for count-down
	int pipefd[2];

	for (vector<Command*>::reverse_iterator  i = commands.rbegin() ; i != commands.rend() ; ++i, --j)
	{
		Command  *cp = *i;
		if (j == commands.size()) {//DEBUG
			cerr << "Pipeline::RIGHTMOST PROCESS\n";//DEBUG
		}//DEBUG
		if (commands.size() > 1) {
            if (pipe(pipefd) == -1){
                perror("pipe");
                exit(EXIT_FAILURE);
            }

            pid_t cpid = fork();
            if (cpid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            if (cpid == 0) {
                close(pipefd[PIPE_READ]);
                if (dup2(pipefd[PIPE_WRITE], STDOUT_FILENO) < 0) {
                    perror("error dup2");
                    exit(EXIT_FAILURE);
                }
                close(pipefd[PIPE_WRITE]);
            } else {
                close(pipefd[PIPE_WRITE]);
                if (dup2(pipefd[PIPE_READ], STDIN_FILENO) < 0) {
                    perror("error dup2");
                    exit(EXIT_FAILURE);
                }
                close(pipefd[PIPE_READ]);
                cp->execute();
            }
		} else if (commands.size() == 1){
            cp->execute();
		}
		if (j == 1) {//DEBUG
			cerr << "Pipeline::LEFTMOST PROCESS\n";//DEBUG
		} else {//DEBUG
			cerr << "Pipeline::CONNECT TO PROCESS\n";//DEBUG
		}//DEBUG
	}
}

bool Pipeline::isBuiltin() const
{
    for (vector<Command*>::const_iterator i = commands.begin(); i != commands.end(); ++i){
        Command *cp = *i;
        if (cp->isBuiltin()){
            return true;
        }
    }
}
// vim:ai:aw:ts=4:sw=4:
