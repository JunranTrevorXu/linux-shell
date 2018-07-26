
/*
 * CS-252
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%token	<string_val> WORD

%token 	NOTOKEN GREAT NEWLINE LESS A GG GA GGA PIPE EX

%union	{
		char   *string_val;
	}

%{
//#define yylex yylex
#include <stdio.h>
#include <string.h>
#include "command.h"
void yyerror(const char * s);
int yylex();

%}

%%

goal:	
	commands
	;

commands: 
	command
	| commands command 
	;

command: simple_command
        ;

simple_command:	
	pipe_list iomodifier_opts background NEWLINE {
		/*execute*/
		
		Command::_currentCommand.execute();
	}
	| NEWLINE 
	| error NEWLINE { yyerrok; }
	| EXIT
	;

background:
	A {
		/*printf("   Yacc: background set\n");*/
		Command::_currentCommand._background = 1;
	}
	|
	;

EXIT:
	EX {
			
		exit(1);
	};

pipe_list:
	command_and_args {
		/*printf("   Yacc: Pipe created\n");*/
	}
	| pipe_list pipe command_and_args {
		/*printf("   Yacc: complex pipe created\n");*/
	}
	;

command_and_args:
	command_word argument_list {
		/*printf("   Yacc: command_and_args created\n");*/
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	;

argument_list:
	argument_list argument {
		/*printf("   Yacc: argument_list created\n");*/
	}
	| /* can be empty */
	;

pipe:
	PIPE {
		/*printf("   Yacc: insert pipe\n");*/
	}
	;

argument:
	WORD {
               /*printf("   Yacc: insert argument \"%s\"\n", $1);*/

	       char * s = strdup($1);
	       char dest[1024];
		if (strcmp(s, "~") == 0) strcpy(s, "/homes/xu473");
		else if (s[0] == '~') {
			
			strcpy(dest, "/homes/");
			s++;
			strcat(dest, s);
			s = strdup(dest);
		}
	       Command::_currentSimpleCommand->expandWhileNecessary( s );
	       
	       
	}
	;

command_word:
	WORD {
               /*printf("   Yacc: insert command \"%s\"\n", $1);*/
	       
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	    
	}
	;

iomodifier_opts:
	iomodifier_opts iomodifier_opt /*{printf("io combined\n");}*/
	|
	;

iomodifier_opt:
	GREAT WORD {
		/*printf("   Yacc: insert output \"%s\"\n", $2);*/
		if (Command::_currentCommand._outFile) Command::_currentCommand._ambi = 1;
		else Command::_currentCommand._outFile = $2;
	}
	| LESS WORD {
		/*printf("   Yacc: insert input \"%s\"\n", $2);*/
		Command::_currentCommand._inFile = $2;
	}
	| GG WORD {
		/*printf("   Yacc: insert output \"%s\"\n", $2);*/
		if (Command::_currentCommand._outFile) Command::_currentCommand._ambi = 1;
		else {
			Command::_currentCommand._outFile = $2;
			Command::_currentCommand._append = 1;
		}
	}
	| GA WORD {
		/*printf("   Yacc: insert output with error \"%s\"\n", $2);*/
		if (Command::_currentCommand._outFile) Command::_currentCommand._ambi = 1;
		else {
			Command::_currentCommand._outFile = $2;
			Command::_currentCommand._errFile = strdup($2);
		}
	}
	| GGA WORD {
		/*printf("   Yacc: insert output with error \"%s\"\n", $2);*/
		if (Command::_currentCommand._outFile) Command::_currentCommand._ambi = 1;
		else {
			Command::_currentCommand._outFile = $2;
			Command::_currentCommand._errFile = strdup($2);
			Command::_currentCommand._append = 1;
		}
	}
	| /* can be empty */ 
	;

%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif
