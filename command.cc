
/*
 * CS252: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <regex.h>
#include <dirent.h>

#include "command.h"

char *array[5000];
int count = 0;

SimpleCommand::SimpleCommand()
{
	// Create available space for 5 arguments
	_numOfAvailableArguments = 5;
	_numOfArguments = 0;
	_arguments = (char **) malloc( _numOfAvailableArguments * sizeof( char * ) );
}

void
SimpleCommand::insertArgument( char * argument )
{
	if ( _numOfAvailableArguments == _numOfArguments  + 1 ) {
		// Double the available space
		_numOfAvailableArguments *= 2;
		_arguments = (char **) realloc( _arguments,
				  _numOfAvailableArguments * sizeof( char * ) );
	}
	
	_arguments[ _numOfArguments ] = argument;

	// Add NULL argument at the end
	_arguments[ _numOfArguments + 1] = NULL;
	
	_numOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc( _numOfSimpleCommands * sizeof( SimpleCommand * ) );

	_numOfSimpleCommands = 0;
	_outFile = 0;
	_inFile = 0;
	_errFile = 0;
	_background = 0;
	_append = 0;
}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
	if ( _numOfAvailableSimpleCommands == _numOfSimpleCommands ) {
		_numOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
			 _numOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
	}
	
	_simpleCommands[ _numOfSimpleCommands ] = simpleCommand;
	_numOfSimpleCommands++;
}

void
Command:: clear()
{
	for ( int i = 0; i < _numOfSimpleCommands; i++ ) {
		for ( int j = 0; j < _simpleCommands[ i ]->_numOfArguments; j ++ ) {
			free ( _simpleCommands[ i ]->_arguments[ j ] );
		}
		
		free ( _simpleCommands[ i ]->_arguments );
		free ( _simpleCommands[ i ] );
	}

	if ( _outFile ) {
		free( _outFile );
	}

	if ( _inFile ) {
		free( _inFile );
	}

	if ( _errFile ) {
		free( _errFile );
	}

	_numOfSimpleCommands = 0;
	_outFile = 0;
	_inFile = 0;
	_errFile = 0;
	_background = 0;
	_append = 0;
	_ambi = 0;

	for (int i = 0; i < 5000; i++) {
	
		array[i] = NULL;
	}
	count = 0;
}

void
Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");
	
	for ( int i = 0; i < _numOfSimpleCommands; i++ ) {
		printf("  %-3d ", i );
		for ( int j = 0; j < _simpleCommands[i]->_numOfArguments; j++ ) {
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
		}
	}

	printf( "\n\n" );
	printf( "  Output       Input        Error        Background\n" );
	printf( "  ------------ ------------ ------------ ------------\n" );
	printf( "  %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",
		_inFile?_inFile:"default", _errFile?_errFile:"default",
		_background?"YES":"NO");
	printf( "\n\n" );
	
}

void
SimpleCommand::expandWhileNecessary(char * arg) {
	
	if (strchr(arg, '*') != NULL || strchr(arg, '?') != NULL) {
		
		Command::_currentSimpleCommand->expandWildcard((char*)"", arg);
		
		for (int i = 0; i < count-1; i++) {
	
			for (int j = 0; j < count-1; j++) {
		
				if (strcmp(array[j], array[j+1]) > 0) {
			
					char * temp = array[j];
					array[j] = array[j+1];
					array[j+1] = temp;
				}
			}
		}
	
		for (int i = 0; i < count; i++) {
	
			Command::_currentSimpleCommand->insertArgument(strdup(array[i]));
		}

	}
	else Command::_currentSimpleCommand->insertArgument(arg);
	return;
}

void
SimpleCommand::expandWildcard(char * prefix, char * suffix) {
//printf("enter wildcard\n");
	if (suffix[0] == 0) {
	
		array[count] = strdup(prefix);
		count++;
		return;
	}

	char * s = strchr(suffix, '/');
	char com[1024];
	com[0] = 0;
	if (s != NULL) {
	
		strncpy(com, suffix, s - suffix);
		suffix = s + 1;
	}
	else {
	
		strcpy(com, suffix);
		suffix = suffix + strlen(suffix);
	}
	//printf("%s\n", com);	
	char newPre[1024];
	if (strchr(com, '*') == NULL && strchr(com, '?') == NULL) {
			
		if (!strcmp(prefix, "") && !strcmp(com, "")) sprintf(newPre, "/\000");
		else if (!strcmp(prefix, "")) sprintf(newPre, "%s\000", com);
		else if (!strcmp(prefix, "/")) sprintf(newPre, "/%s\000", com);
		else sprintf(newPre, "%s/%s\000", prefix, com);
		com[0] = 0;
		expandWildcard(newPre, suffix);
		newPre[0] = 0;
		return;
	}
	
	char * reg = (char*)malloc(strlen(com)*2 + 10);
	char * c = com;
	char * r = reg;
	*r = '^';
	r++;
	while (*c) {
	
		if (*c == '*') { *r = '.'; r++; *r = '*'; r++; }
		else if (*c == '?') { *r = '.'; r++; }
		else if (*c == '.') { *r = '\\'; r++; *r = '.'; r++; }
		else { *r = *c; r++; }
		c++;
	}
	*r = '$';
	r++;
	*r = 0;
//printf("%s, %s\n", reg, prefix);	
	regex_t re;
	int result = regcomp(&re, reg, REG_EXTENDED|REG_NOSUB);
	if (result != 0) { 
		
		perror("regcomp");
		exit(1);
	}
	char dir[1024];
	if (!strcmp(prefix, "")) strcpy(dir, ".");
	else strcpy(dir, prefix);

	DIR * d = opendir(dir);
	if (d == NULL) {
	
		return;
	}
	struct dirent * ent;

	while ((ent = readdir(d)) != NULL) {
	
		regmatch_t match;
		if (regexec(&re, ent->d_name, 1, &match, 0) == 0) {
			//printf("matches\n");	
			if (ent->d_name[0] == '.') {
					
				if (reg[1] == '\\' && reg[2] == '.') {
					
					if (!strcmp(prefix, "")) sprintf(newPre, "%s\000", ent->d_name);
					else if (!strcmp(prefix, "/")) sprintf(newPre, "/%s\000", ent->d_name);
					else sprintf(newPre, "%s/%s\000", prefix, ent->d_name);
					com[0] = 0;
					expandWildcard(newPre, suffix);
					newPre[0] = 0;
				}
			}
			else {
			//printf("make newPre\n");
				if (!strcmp(prefix, "")) sprintf(newPre, "%s\000", ent->d_name);
				else if (!strcmp(prefix, "/")) sprintf(newPre, "/%s\000", ent->d_name);
				else sprintf(newPre, "%s/%s\000", prefix, ent->d_name);
				com[0] = 0;
				expandWildcard(newPre, suffix);
				newPre[0] = 0;
	
			}
		}
	}
	free(reg);
	closedir(d);
	return;
}

void sighandlerr(int sig) {

	while (waitpid(-1, NULL, WNOHANG) > 0);
}

void
Command::execute()
{
	// Don't do anything if there are no simple commands
	if ( _numOfSimpleCommands == 0 ) {
		prompt();
		return;
	}

	// Print contents of Command data structure


	
	//print();
	// Add execution here
	// For every simple command fork a new process
	// Setup i/o redirection
	// and call exec
	
	if (_ambi) {
		printf("Ambiguous output redirect\n");
		return;
	}

	int tempin = dup(0);
	int tempout = dup(1);
	int temperr = dup(2);

	int fdin;
	if (_inFile) fdin = open(_inFile, O_RDONLY|O_CREAT, 0444);
	else fdin = dup(tempin);

	int ret;
	int fdout;
	int fderr;

	for (int i = 0; i < _numOfSimpleCommands; i++) {
	
		dup2(fdin, 0);
		close(fdin);
		
		if (_errFile && _append) fderr = open(_errFile, O_APPEND|O_CREAT|O_WRONLY, 0777);
		else if (_errFile) fderr = open(_outFile, O_CREAT|O_TRUNC|O_WRONLY, 0777);
		else fderr = dup(temperr);
		dup2(fderr, 2);
		close(fderr);

		if (i == _numOfSimpleCommands - 1) {
		
			if (_outFile && _append) fdout = open(_outFile, O_APPEND|O_CREAT|O_WRONLY, 0777);
			else if (_outFile) fdout = open(_outFile, O_CREAT|O_TRUNC|O_WRONLY, 0777);
			else fdout = dup(tempout);
		}

		else {
		
			int fdpipe[2];
			int check = pipe(fdpipe);
			if (check < 0) perror("pipe\n");
			fdout = fdpipe[1];
			fdin = fdpipe[0];
		}
		
		dup2(fdout, 1);
		close(fdout);
		
		ret = fork();
		if (ret == 0) {
		
			if (!strcmp(_simpleCommands[i]->_arguments[0], "printenv")) {
			
				char **p = environ;
				while (*p != NULL) {
				
					printf("%s\n", *p);
					p++;
				}
				exit(0);
			}
			else if (!strcmp(_simpleCommands[i]->_arguments[0], "setenv")) exit(0);
			else if (!strcmp(_simpleCommands[i]->_arguments[0], "unsetenv")) exit(0);
			else if (!strcmp(_simpleCommands[i]->_arguments[0], "cd")) exit(0);
			execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
			perror("execvp");
			exit(1);
		}
		else if (ret > 0 && !_background) {
		
			waitpid(ret, NULL, 0);
			if (!strcmp(_simpleCommands[i]->_arguments[0], "cd")) {
			
				char * path = (char *)".";
				
				if (_simpleCommands[i]->_arguments[1] == NULL) chdir(getenv("HOME"));

				else if (_simpleCommands[i]->_arguments[1] != NULL && _simpleCommands[i]->_arguments[1][0] != 0) 
					path = _simpleCommands[i]->_arguments[1];
				if (chdir(path) != 0) perror("chdir");
			}
			else if (!strcmp(_simpleCommands[i]->_arguments[0], "unsetenv")) {
			
				unsetenv(_simpleCommands[i]->_arguments[1]);
			}
			else if (!strcmp(_simpleCommands[i]->_arguments[0], "setenv")) {
			
				char * var = (char *)_simpleCommands[i]->_arguments[1];
				char * value = (char *)_simpleCommands[i]->_arguments[2];
				char **p = environ;
				while (*p != NULL) {
				
					if (!strncmp(*p, var, strlen(var))) {
					
						sprintf(*p, "%s=%s\000", var, value);
						break;
					}
					p++;
				}
				*p = (char*)malloc(1024*sizeof(char));
				sprintf(*p, "%s=%s\000", var, value);
				p++;
				*p = NULL;
			}
		}
		else if (ret > 0 && _background){\
		
			signal(SIGCHLD, sighandlerr);
		}
		else {
			
			perror("fork");
			exit(1);
		}
	}
	dup2(tempin, 0);
	dup2(tempout, 1);
	dup2(temperr, 2);

	close(tempin);
	close(tempout);
	close(temperr);

	// Clear to prepare for next command
	//printf("executed\n");
	//print();
	if (getenv("ON_ERROR") != NULL) printf("%s\n", getenv("ON_ERROR"));

	clear();
	//printf("\ncleared\n");
	//print();
	// Print new prompt
	prompt();
}

// Shell implementation

void
Command::prompt()
{
	if(isatty(0)) {
		
		if (getenv("PROMPT") != NULL) printf("%s", getenv("PROMPT"));
		else printf("myshell>");
	}
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);

void sighandler(int sig) {

	printf("\n");
	Command::_currentCommand.prompt();
	return;
}

main()
{
	Command::_currentCommand.prompt();

    signal(SIGINT, sighandler);

    yyparse();
}

