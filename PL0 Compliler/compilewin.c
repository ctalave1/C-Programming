//Chris Talavera & Kyle Reynolds
//10/19/14
//System Software
//Compiler Driver

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Keeps track of which program needs
//to print their output to the console.
int printFlag[] = {0,0,1};

//Prototypes
void parseDirectives(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    parseDirectives(argc, argv);

    int status = EXIT_SUCCESS;

	if(printFlag[0] == 0 && status == EXIT_SUCCESS)
		status = system("scanner");
	else if(printFlag[0] == 1 && status == EXIT_SUCCESS)
		status = system("scanner -l");

    if(printFlag[1] == 0 && status == EXIT_SUCCESS)
        status = system("parser");
    else if(printFlag[1] == 1 && status == EXIT_SUCCESS)
        status = system("parser -a");

    if(printFlag[2] == 0 && status == EXIT_SUCCESS)
        system("pmachine");
    else if(printFlag[2] == 1 && status == EXIT_SUCCESS)
		system("pmachine -v");

    if(status == EXIT_FAILURE)
        exit(EXIT_FAILURE);

    return 0;
}

//Grabs the directives passed to the compile driver and
//sends the directives to its corresponding program. The
//directives control which program must print its output
//to the console.
void parseDirectives(int argc, char* argv[])
{
    int i;

	for(i = 1; i < argc; i++)
    {

		if( strcmp(argv[i],"-l") == 0)
			printFlag[0] = 1;

		else if( strcmp(argv[i],"-a") == 0)
			printFlag[1] = 1;

		else if( strcmp(argv[i],"-v") == 0)
			printFlag[2] = 1;

        //Throw this error, if any directive is not
        //"-l", "-a", or "-v".
        else
            goto argerr;
	}

    return;

//Bad directive
argerr:
    printf("Bad directive: %s", argv[i]);
    exit(EXIT_FAILURE);
}
