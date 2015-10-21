//Chris Talavera
//9/21/14
//System Software
//Scanner

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//Constants
#define RESERVEDWORDS 14
#define MAXRESERVEDWORDSIZE 10
#define MAXIDENTLENGTH 11
#define MAXDIGITS 5
#define MAXTOKENLENGTH 501

//Enumerations
typedef enum
{
nulsym = 1,
identsym,
numbersym,
plussym,
minussym,
multsym,
slashsym,
oddsym,
eqlsym,
neqsym,
lessym,
leqsym,
gtrsym,
geqsym,
lparentsym,
rparentsym,
commasym,
semicolonsym,
periodsym,
becomessym,
beginsym,
endsym,
ifsym,
thensym,
whilesym,
dosym,
callsym,
constsym,
varsym,
procsym,
writesym,
readsym,
elsesym
}token_type;

//Global variables
char* reservedWords[] =
{
"begin",
"end",
"if",
"then",
"while",
"do",
"call",
"const",
"var",
"procedure",
"write",
"read",
"else",
"odd"
};

//Prototypes
void cleanInput(char input[], char cleaninput[], int inputSize);
void formLexemes(char input[]);

int main(int argc, const char *argv[])
{
    int i;
    int charCount = 0;//Holds the number of characters in the input file
    int endOfFileCheck; //Used to check the end of the input file

    char temp;

    FILE* ifp, *ofp;

    ifp = fopen("input.txt", "r");

    //Gets the number of characters in the input file
    while((endOfFileCheck = fgetc(ifp)) != EOF)
        charCount++;

    //Creates two arrays, one for the input text and the other for the cleaned up input text
    char input[charCount];
    char newInput[charCount];

    //Rewinds the file pointer back to the beginning of the file
    rewind(ifp);

    //Puts the input text in the input array
    for(i = 0; i < charCount; i++)
    {
        fscanf(ifp, "%c", &temp);
        input[i] = temp;
    }

    input[i] = '\0';

    fclose(ifp);

    //Creates cleaninput.txt
    cleanInput(input, newInput, charCount);

    //Creates lexemetable.txt and lexemelist.txt
    formLexemes(newInput);

    //If the "-l" directive is passed to the scanner, print
    //both the lexeme table and lexeme list to the console.
    if(argc > 1 && strcmp(argv[1], "-l") == 0)
    {
        charCount = 0;

        ifp = fopen("lexemelist.txt", "r");

        while((endOfFileCheck = fgetc(ifp)) != EOF)
            charCount++;

        rewind(ifp);

        printf("Lexeme List\n-----------------\n");

        for(i = 0; i < charCount; i++)
        {
            fscanf(ifp, "%c", &temp);
            printf("%c", temp);
        }

        fclose(ifp);

        charCount = 0;

        printf("\n\n");

        /*ifp = fopen("lexemetable.txt", "r");

        while((endOfFileCheck = fgetc(ifp)) != EOF)
            charCount++;

        rewind(ifp);

        printf("Lexeme Table\n-----------------\n");

        for(i = 0; i < charCount; i++)
        {
            fscanf(ifp, "%c", &temp);
            printf("%c", temp);
        }

        fclose(ifp);

        printf("\n\n");*/
    }

    ofp = fopen("fullcompileroutput.txt", "a");

    charCount = 0;

    ifp = fopen("lexemelist.txt", "r");

    while((endOfFileCheck = fgetc(ifp)) != EOF)
        charCount++;

    rewind(ifp);

    fprintf(ofp, "lexemelist.txt\n-----------------\n");

    for(i = 0; i < charCount; i++)
    {
        fscanf(ifp, "%c", &temp);
        fprintf(ofp, "%c", temp);
    }

    fclose(ifp);

    charCount = 0;

    fprintf(ofp, "\n\n");

    ifp = fopen("lexemetable.txt", "r");

    while((endOfFileCheck = fgetc(ifp)) != EOF)
        charCount++;

    rewind(ifp);

    fprintf(ofp,"lexemetable.txt\n-----------------\n");

    for(i = 0; i < charCount; i++)
    {
        fscanf(ifp, "%c", &temp);
        fprintf(ofp, "%c", temp);
    }

    fclose(ifp);

    fprintf(ofp, "\n\n");

    fclose(ofp);

    return 0;
}

//Cleans up the input from input file
void cleanInput(char input[], char cleaninput[], int inputSize)
{
    int i, j;
    int isComment = 0;

    FILE* ofp1, *ofp2;

    i = j = 0;

    //While not the end of the input, check for text that isn't commented and place it in the clean array.
    while(i < inputSize)
    {
        if(input[i] == '/')
        {
            if(input[i + 1] == '*')
            {
                isComment = 1;
                i += 2;
                continue;
            }
        }

        else if(input[i] == '*')
        {
            if(input[i + 1] == '/')
            {
                isComment = 0;
                i += 2;
                continue;
            }
        }

        //While in a comment, don't place current char in the clean array
        if(isComment)
        {
            i++;
            continue;
        }

        cleaninput[j] = input[i];
        i++;
        j++;
    }

    cleaninput[j] = '\0';

    ofp1 = fopen("cleaninput.txt", "w");
    ofp2 = fopen("fullcompileroutput.txt", "w");

    fprintf(ofp1, "%s", cleaninput);

    fprintf(ofp2, "cleaninput.txt\n------------------------\n");
    fprintf(ofp2, "%s\n\n", cleaninput);

    fclose(ofp1);
    fclose(ofp2);

    return;
}

//Tokenizes and categorizes the input from the input file, so that a lexeme table and lexeme list can be made.
void formLexemes(char input[])
{
    int i, j, k;
    int error;
    int reserved = 0;

    char temp[MAXTOKENLENGTH];
    token_type tokenType;

    k = 0;

    FILE* ofp1;
    FILE* ofp2;
    FILE* ofp3;

    ofp1 = fopen("lexemetable.txt", "w");
    ofp2 = fopen("lexemelist.txt", "w");

    fprintf(ofp1, "lexeme\ttoken type\n");

    //While the end of the input array isn't reached, check for lexemes.
    for(i = 0; input[i] != '\0'; i++)
    {
        //Skip over white spaces and control characters.
        if(iscntrl(input[i]) || isspace(input[i]))
            continue;

        //If the character is alphanumeric, check if it is a part of a identifier or a digit.
        //One of three errors may be found during this process: 1)digit too long,
        //2)name too long, or 3)variable doesn't start with a letter.
        if(isalnum(input[i]))
        {
            if(isalpha(input[i]))
            {
                do
                {
                    temp[k] = input[i];
                    k++;
                    i++;
                }
                while(isalnum(input[i]));

                i--;

                temp[k] = '\0';

                reserved = 0;

                for(j = 0; j < RESERVEDWORDS; j++)
                {
                    if(strcmp(reservedWords[j], temp) == 0)
                    {
                        if(strcmp("odd", temp) == 0)
                        {
                            reserved = 1;
                            tokenType = oddsym;
                            fprintf(ofp1, "%s\t%d\n", temp, tokenType);
                            fprintf(ofp2, "%d ", tokenType);
                            break;
                        }

                        reserved = 1;
                        tokenType = j + 21;
                        fprintf(ofp1, "%s\t%d\n", temp, tokenType);
                        fprintf(ofp2, "%d ", tokenType);
                        break;
                    }
                }

                if(!reserved)
                {
                    if(k > MAXIDENTLENGTH)
                        goto lexerror3;

                    tokenType = identsym;

                    fprintf(ofp1, "%s\t%d\n", temp, tokenType);
                    fprintf(ofp2, "%d %s ", tokenType, temp);
                }

                k = 0;
            }

            else if(isdigit(input[i]))
            {
                do
                {
                    if(isalpha(input[i + 1]))
                        goto lexerror1;

                    temp[k] = input[i];
                    k++;
                    i++;
                }
                while(isdigit(input[i]));

                i--;

                temp[k] = '\0';

                if(k > MAXDIGITS)
                    goto lexerror2;

                tokenType = numbersym;

                fprintf(ofp1, "%s\t%d\n", temp, tokenType);
                fprintf(ofp2, "%d %s ", tokenType, temp);

                k = 0;
            }
        }

        //If the character isn't alphanumeric, check if it is a part of a special symbol. If the lexeme isn't a special symbol,
        //the "invalid symbol" error occurs
        else
        {
            switch(input[i])
            {
					case '+':
					    temp[k] = input[i];
					    temp[k + 1] = '\0';
					    tokenType = plussym;
					    fprintf(ofp1, "%s\t%d\n", temp, tokenType);
					    fprintf(ofp2, "%d ", tokenType);
						break;

					case '-':
						temp[k] = input[i];
					    temp[k + 1] = '\0';
					    tokenType = minussym;
					    fprintf(ofp1, "%s\t%d\n", temp, tokenType);
					    fprintf(ofp2, "%d ", tokenType);
						break;

					case '*':
						temp[k] = input[i];
					    temp[k + 1] = '\0';
					    tokenType = multsym;
					    fprintf(ofp1, "%s\t%d\n", temp, tokenType);
					    fprintf(ofp2, "%d ", tokenType);
						break;

					case '/':
						temp[k] = input[i];
					    temp[k + 1] = '\0';
					    tokenType = slashsym;
					    fprintf(ofp1, "%s\t%d\n", temp, tokenType);
					    fprintf(ofp2, "%d ", tokenType);
						break;

					case '(':
						temp[k] = input[i];
					    temp[k + 1] = '\0';
					    tokenType = lparentsym;
					    fprintf(ofp1, "%s\t%d\n", temp, tokenType);
					    fprintf(ofp2, "%d ", tokenType);
						break;

					case ')':
						temp[k] = input[i];
					    temp[k + 1] = '\0';
					    tokenType = rparentsym;
					    fprintf(ofp1, "%s\t%d\n", temp, tokenType);
					    fprintf(ofp2, "%d ", tokenType);
						break;

					case '=':
						temp[k] = input[i];
					    temp[k + 1] = '\0';
					    tokenType = eqlsym;
					    fprintf(ofp1, "%s\t%d\n", temp, tokenType);
					    fprintf(ofp2, "%d ", tokenType);
						break;

					case ',':
						temp[k] = input[i];
					    temp[k + 1] = '\0';
					    tokenType = commasym;
					    fprintf(ofp1, "%s\t%d\n", temp, tokenType);
					    fprintf(ofp2, "%d ", tokenType);
						break;

					case '.':
						temp[k] = input[i];
					    temp[k + 1] = '\0';
					    tokenType = periodsym;
					    fprintf(ofp1, "%s\t%d\n", temp, tokenType);
					    fprintf(ofp2, "%d ", tokenType);
						break;

					case '<':
						if(input[i + 1] == '=')
                        {
							for(j = i; j < i + 2; j++)
                            {
                                temp[k] = input[j];
                                k++;
                            }

                            temp[k] = '\0';

                            k = 0;

                            i++;

                            tokenType = leqsym;

                            fprintf(ofp1, "%s\t%d\n", temp, tokenType);
                            fprintf(ofp2, "%d ", tokenType);
						}
						else if(input[i + 1] == '>')
                        {
                            for(j = i; j < i + 2; j++)
                            {
                                temp[k] = input[j];
                                k++;
                            }

                            temp[k] = '\0';

                            k = 0;

                            i++;

                            tokenType = neqsym;

                            fprintf(ofp1, "%s\t%d\n", temp, tokenType);
                            fprintf(ofp2, "%d ", tokenType);
                        }
						else
                        {
							temp[k] = input[i];
                            temp[k + 1] = '\0';
                            tokenType = lessym;
                            fprintf(ofp1, "%s\t%d\n", temp, tokenType);
                            fprintf(ofp2, "%d ", tokenType);
						}

						break;

					case '>':
						if(input[i + 1] == '=')
                        {
							for(j = i; j < i + 2; j++)
                            {
                                temp[k] = input[j];
                                k++;
                            }

                            temp[k] = '\0';

                            k = 0;

                            i++;

                            tokenType = geqsym;

                            fprintf(ofp1, "%s\t%d\n", temp, tokenType);
                            fprintf(ofp2, "%d ", tokenType);
						}
						else
                        {
							temp[k] = input[i];
                            temp[k + 1] = '\0';
                            tokenType = gtrsym;
                            fprintf(ofp1, "%s\t%d\n", temp, tokenType);
                            fprintf(ofp2, "%d ", tokenType);
						}

						break;

					case ';':
						temp[k] = input[i];
                        temp[k + 1] = '\0';
                        tokenType = semicolonsym;
                        fprintf(ofp1, "%s\t%d\n", temp, tokenType);
                        fprintf(ofp2, "%d ", tokenType);
						break;

					case ':':
						if(input[i + 1] == '=')
                        {
							for(j = i; j < i + 2; j++)
                            {
                                temp[k] = input[j];
                                k++;
                            }

                            temp[k] = '\0';

                            k = 0;

                            i++;

                            tokenType = becomessym;

                            fprintf(ofp1, "%s\t%d\n", temp, tokenType);
                            fprintf(ofp2, "%d ", tokenType);
						}
						else
							goto lexerror4;

						break;

					default:
						goto lexerror4;
						break;
				}
        }
    }

    fclose(ofp1);
    fclose(ofp2);

    return;

lexerror1:
    printf("Variable does not start with a letter.\n");
    fclose(ofp1);
    fclose(ofp2);
    exit(EXIT_FAILURE);

lexerror2:
    printf("Number too long.\n");
    fclose(ofp1);
    fclose(ofp2);
    exit(EXIT_FAILURE);

lexerror3:
    printf("Name too long.\n");
    fclose(ofp1);
    fclose(ofp2);
    exit(EXIT_FAILURE);

lexerror4:
    printf("Invalid symbols.\n");
    fclose(ofp1);
    fclose(ofp2);
    exit(EXIT_FAILURE);
}
