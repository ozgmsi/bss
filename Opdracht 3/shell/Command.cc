/** @file Command.cc
 * Implementation of class Command.
 */
#include <iostream>
#include <cstdlib>		// for: getenv()
#include <unistd.h>		// for: getcwd(), close(), execv(), access()
#include <limits.h>		// for: PATH_MAX
#include <fcntl.h>		// for: O_RDONLY, O_CREAT, O_WRONLY, O_APPEND
#include <sys/types.h>  // for: umask
#include <sys/stat.h>   // for: umask
#include <algorithm>    // for: count
#include "asserts.h"
#include "unix_error.h"
#include "Command.h"
#include "string.h"
using namespace std;


// Iff PATH_MAX is not defined in limits.h
#ifndef	PATH_MAX
# define	PATH_MAX (4096)	/* i.e. virtual pagesize */
#endif


Command::Command()
	: append(false)
{
}


void	Command::addWord(string& word)
{
	words.push_back(word);
}


void	Command::setInput(std::string& the_input)
{
	require(input.empty());		// catch multiple inputs
	input = the_input;
}

void	Command::setOutput(std::string& the_output)
{
	require(output.empty());	// catch multiple outputs
	output = the_output;
	append = false;
}

void	Command::setAppend(std::string& the_output)
{
	require(output.empty());	// catch multiple outputs
	output = the_output;
	append = true;
}

// A true "no-nothing" command?
bool	Command::isEmpty() const
{
	return input.empty() && output.empty() && words.empty();
}

bool    Command::isBuiltin() const
{
    size_t found;
    for (vector<string>::const_iterator i = words.begin(); i != words.end(); ++i){
        string word = *i;

        found = word.find("exit");
        if (found != string::npos){
            return true;
        }

        found = word.find("logout");
        if (found != string::npos){
            return true;
        }

        found = word.find("cd");
        if (found != string::npos){
            return true;
        }
    }
}
// ===========================================================


// Execute a command
void	Command::execute()
{
	// TODO:	Handle I/O redirections.
	//			Don't blindly assume the open systemcall will always succeed!
	// TODO:	Convert the words vector<string> to: array of (char*) as expected by 'execv'.
	//			Note: In this case it safe to typecast from 'const char*' to 'char *'.
	//			Note: Make sure the last element of that array will be a 0 pointer!
	// TODO:	Determine the path of the program to be executed.
	// 			If the name contains a '/' it already is a path name,
	//				either absolute like "/bin/ls" or relative to the
	//				current directory like "sub/prog".
	// 			Otherwise it is the name of an executable file to be
	// 				searched for using the PATH environment variable.
	// TODO:	Execute the program passing the arguments array.
	// Also see: close(2), open(2), getcwd(3), getenv(3), access(2), execv(2), exit(2)

	// TODO: replace the code below with something that really works
    /// Others have nothing to do with our files.
	umask(S_IWOTH);

	if (hasInput()){
        /// file openen, redirecten en file weer sluiten.
        int inputf = open(input.c_str(), O_RDONLY);
        dup2(inputf, STDIN_FILENO);
        close(inputf);
	}
	if (hasOutput()){
        int outputf;
        if (append) {
            outputf = open(output.c_str(), O_APPEND|O_WRONLY, 0777);
            if (outputf == -1){
                outputf = open(output.c_str(), O_CREAT|O_WRONLY, 0777);
            }
        } else {
            outputf = open(output.c_str(), O_CREAT | O_WRONLY, 0777);
        }

        if (outputf == -1){
            cerr << "Kon bestand niet openen" << endl;
            exit(EXIT_FAILURE);
        }
        /// redirect output.
        dup2(outputf, STDOUT_FILENO);
        close(outputf);
	}

    /// commandos omzetten naar argumenten voor het uitvoeren in execv
	std::vector<char*> commandAttributes(words.size() + 1);
	for (size_t i = 0; i != words.size(); ++i){
        commandAttributes[i] = &words[i][0];
	}

    /// Er mag geen legen invoer zijn!
	if (commandAttributes[0] === '\0'){
        cerr << "\0 is not a valid filename!" << endl;
        exit(EXIT_FAILURE);
	}

    /// controleren op builtins, cd, exit of logout.
	if (isBuiltin()) {
        /// er moet een system call gedaan worden omdat een builtin type is gevonden.
        if (words[0].find("cd") != string:npos){
            if (1 < words.size()) {
                cout << "Changing working directory to: " << words[1] << endl;
                chdir(words[1].c_str());
            }
        } else if (words[0].find("exit") != string::npos || words[0].find("logout") != string::npos) {
            cout << "Exiting!" << endl;
            exit(EXIT_FAILURE);
        }
	} else {
	    /// plaats naar bestand
        if (strstr(commandAttributes[0], "/") != 0) {
            if (commandAttributes[0][0] != '/'){
                char* cwd = get_current_dir_name();
                string path = cwd;
                path = path + '/' + commandAttributes[0];
                commandAttributes[0] = (char*) path.c_str();
            }
        } else {
            string path = getenv("PATH");
            int pathSize = 0;
            for (int i = 0; i < path.size(); i++) {
                if (path[i] == ':'){
                    pathSize++;
                }
            }

            /// Split path
            string pathArray[pathSize];
            int arrayIndex = 0;
            int lastDelimiterIndex = 0;
            for (int i = 0; i < path.size(); i++) {
                if (path[i] == ':') {
                    pathArray[arrayIndex] = path.substr(lastDelimiterIndex, i - lastDelimiterIndex);
                    lastDelimiterIndex = i + 1;
                    arrayIndex++;
                }
            }

            /// Search for binary
            string binaryPath = "";
            for (int i = 0; i < pathSize; i++) {
                binaryPath = pathArray[i] + '/' + commandAttributes[0];
                if (access(binaryPath.c_str(), X_OK) == 0) {
                    /// Binary is found, do not look further.
                    break;
                }

                /// reset binary path.
                binaryPath = "";
            }

            if (binaryPath.empty()) {
                cerr << "command not found: " << commandAttributes[0] << endl;
                exit(EXIT_FAILURE);
            }

            commandAttributes[0] = (char*) binaryPath.c_str();
        }

        /// execv system call replaces the current process image with a new process images
        /// Now execute the PATH passing the arguments array.
        execv(commandAttributes[0], commandAttributes.data());

        /* NOTREACHED */
        perror(commandAttributes[0]);
        exit(EXIT_FAILURE);
	}

#if 1	/* DEBUG code: Set to 0 to turn off the next block of code */
	cerr <<"Command::execute ";
	// Show the I/O redirections first ...
	if (!input.empty())
		cerr << " <"<< input;
	if (!output.empty()) {
		if (append)
			cerr << " >>"<< output;
		else
			cerr << " >"<< output;
	}
	// ... now show the command & parameters to execute
	if (words.empty())
		cerr << "\t(EMPTY_COMMAND)" << endl;
	else {
		cerr << "\t";
		for (vector<string>::iterator  i = words.begin() ; i != words.end() ; ++i)
			cerr << " " << *i;
		cerr << endl;
	}
#endif	/* end DEBUG code */
}


// vim:ai:aw:ts=4:sw=4:
