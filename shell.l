
/*
 *
 * CS-252 Fall 2013
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include "y.tab.h"

static  void yyunput (int c,char *buf_ptr  );

void myunputc(int c) {
    unput(c);
}

%}

%%

\n 	{
		return NEWLINE;
	}

[ \t] 	{
		/* Discard spaces and tabs */
     	}	

">" 	{
		return GREAT;
	}

"<"	{
		return LESS;
	}

">>"	{
		return GG;
	}

">&"	{
		return GA;
	}

">>&"	{
		return GGA;
	}

"|"	{
		return PIPE;
	}

"&"	{
		return A;
	}

"exit" 	{
		return EX;
	}

['][^\n']*[']	{
		char * str = strdup(yytext);
		strcat(str, "\nexit\n");
	//printf("enter sub\n");	
		int tempin = dup(0);
		int tempout = dup(1);
		
		int fd2[2];
		pipe(fd2);

		int ret = fork();
		if (ret < 0) {
		
			perror("fork");
			exit(1);
		}

		else if (ret == 0) {
			int fd1[2];
			pipe(fd1);
			write(fd1[1], str, strlen(str));
			dup2(fd1[0], 0);
			close(fd1[0]);
			close(fd1[1]);
			dup2(fd2[1], 1);
			close(fd2[1]);
			execvp("/proc/self/exe", NULL);
			perror("execvp");
			exit(1);
		}
		close(fd2[1]);
		while (waitpid(ret, NULL, 0) == 0){}

		free(str);

		dup2(tempin, 0);
		dup2(tempout, 1);
		close(tempin);
		close(tempout);
//printf("leave sub\n");
		char outp[3000];
		char c;
		int i = 0;
		//printf("here\n");
		while(read(fd2[0], &c, 1) != 0) {
		
			//printf("%c\n", c);
			outp[i] = c;
			i++;
		}
		outp[i] = '\0';
		close(fd2[0]);
		//printf("outp: %s\n", outp);
		while (i >= 0) {
		
			if (outp[i] == '\n') outp[i] = ' ';
			unput(outp[i]);
			i--;
		}

		outp[0] = 0;

		}

\$\{[^\$\n\t]*\} {

	
	char * value = strdup(yytext + 2);
	value[yyleng - 3] = '\000';
	yylval.string_val = strdup(getenv(value));
	free(value);
	//printf("lex: %s\n", yylval.string_val);
	return WORD;
}

\\\"[^\n\"]*\\\" {

		yylval.string_val = strdup(yytext + 2);
		yylval.string_val[yyleng - 4] = '\000';
		return WORD;
		}

\"[^\n\"]*\"	{

		yylval.string_val = strdup(yytext + 1);
		yylval.string_val[yyleng - 2] = '\000';
		return WORD;
		}

[^ \$\{\}\t\n'][^ \$\{\}\t\n']*  {
		/* Assume that file names have only alpha chars */
		if (strchr(yytext, '\\') != NULL) {
			
			char * value = (char*)malloc(strlen(yytext));
			int j = 0;
			int i = 0;
			while(yytext[j] != 0) {
			
				if (yytext[j] != '\\') {
				
					value[i] = yytext[j];
					j++;
					i++;
				}
				else {
					
					j++;
					value[i] = yytext[j];
					i++;
					j++;
				}
			}
			value[i] = '\000';
			yylval.string_val = strdup(value);
		}
		else yylval.string_val = strdup(yytext);
		return WORD;
	}

%%

