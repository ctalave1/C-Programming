/*
 *Chris Talavera & Kyle Reynolds
 *10/19/14
 *System Software
 *Parser/Code Generator
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Constants
#define MAX_SYMBOL_TABLE_SIZE 100
#define MAX_CODE_LENGTH 500
#define MAXTOKENLENGTH 501

typedef struct instruction
{
    int op; //opcode
    int l;  //L
    int m;  //M
}instruction;

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
    eqsym,
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

typedef struct symbol
{
	int kind; 		// const = 1, var = 2, proc = 3
	char name[12];	// name up to 11 chars
	int val; 		// number (ASCII value)
	int level; 	// L level
	int addr; 		// M address
} symbol;

//Global Variables
int currSym = 0; //Keeps track of the current position in the symbol table
int currTokenPos = 0;
int currCode = 0; //Keeps track of the next position in the code table
int currM = 4; //Keeps track of the number of spots opened up on the stack to make room for variables
int currL = -1; //Keeps track of the current lexigrahpical level

symbol symbol_table[MAX_SYMBOL_TABLE_SIZE];

instruction code[MAX_CODE_LENGTH];

int token; //Holds the current token
int value; //Holds the value of an numbersym
char identifier[MAXTOKENLENGTH]; //Holds the identifier of an identsym

FILE* ifp;

//Prototypes
void getToken();
void ungetToken();
void error(int type);

void enter(int kind, char name[], int val, int level, int addr);

void emit(int op, int l, int m);
int find(char ident[]);
int symbolType(int pos);
int symbolLevel(int pos);
int symbolAddress(int pos);

void printCode();

void program();
void block();
void constDeclaration();
int varDeclaration();
void procDeclaration();
void statement();
void condition();
void expression();
void term();
void factor();

int main(int argc, char* argv[])
{
    ifp = fopen("lexemelist.txt", "r");

    program();

    printf("Program is syntactically correct.\n\n");

    fclose(ifp);

    printCode();

    //If the "-a" directive is passed to the parser/code generator,
    //print the generated machine code to the console.
    if(argc > 1 && strcmp(argv[1], "-a") == 0)
    {
        int i;

        printf("Machine Code\n-----------------\n");

        for(i = 0; i < currCode; i++)
        {
            printf( "%d %d %d\n", code[i].op, code[i].l, code[i].m);
        }

        printf("\n");
    }

    return 0;
}

//Searches through the symbol table to find the given identifier and
//returns its position. If it is not found, it returns -1.
int find(char ident[])
{
    int i;

    for(i = 0; i < currSym; i++)
    {
        if(strcmp(ident, symbol_table[i].name) == 0)
            return i;
    }

    return -1;
}

//Searches for the symbol at the given position and returns its type.
//Otherwise, it returns 0;
int symbolType(int pos)
{
    if(pos == -1)
        return 0;

    return symbol_table[pos].kind;
}

//Searches for the symbol at the given position and returns its level.
//Otherwise, it returns 0;
int symbolLevel(int pos)
{
    if(pos == -1)
        return 0;

    return symbol_table[pos].level;
}

//Searches for the symbol at the given position and returns its address.
//Otherwise, it returns 0;
int symbolAddress(int pos)
{
    if(pos == -1)
        return 0;

    return symbol_table[pos].addr;
}

//Writes the fully formed code array to mcode.txt
void printCode()
{
    FILE* ofp1 = fopen("mcode.txt", "w");
    FILE* ofp2 = fopen("fullcompileroutput.txt", "a");

    int i;

    fprintf(ofp2, "mcode.txt\n-----------------\n");

    for(i = 0; i < currCode; i++)
    {
        fprintf(ofp1, "%d %d %d\n", code[i].op, code[i].l, code[i].m);
        fprintf(ofp2, "%d %d %d\n", code[i].op, code[i].l, code[i].m);
    }

    fprintf(ofp2, "\n\n");

    fclose(ofp1);
    fclose(ofp2);
}

//Gets the next token in the lexeme list
void getToken()
{
    fscanf(ifp, "%d", &token);

    //If the current token is an identsym, store its identifier name
    //because it will be used in the near future.
    if(token == identsym)
        fscanf(ifp, "%s", identifier);

    //If the current token is an numbersym, store its value
    //because it will be used in the near future.
    else if(token == numbersym)
        fscanf(ifp, "%d", &value);

    currTokenPos++;
}

//Enters a new entry in the symbol table.
//For constants, you must store kind, name and value.
//For variables, you must store kind, name, L and M.
void enter(int kind, char name[], int val, int level, int addr)
{
    if(kind == 1)
    {
        symbol_table[currSym].kind = kind;
        strcpy(symbol_table[currSym].name, name);
        symbol_table[currSym].val = val;
        currSym++;
    }

    else if(kind == 2 || kind == 3)
    {
        symbol_table[currSym].kind = kind;
        strcpy(symbol_table[currSym].name, name);
        symbol_table[currSym].level = level;
        symbol_table[currSym].addr = addr;
        currSym++;
    }
}

//Enters a new line of code into the machine code
//array that is currently being formed.
void emit(int op, int l, int m)
{
    //If the length of the code array exceeds the
    //maximum code length, throw an error.
    if(currCode > MAX_CODE_LENGTH)
        error(26);

    else
    {
        code[currCode].op = op;
        code[currCode].l = l;
        code[currCode].m = m;
        currCode++;
    }
}

//program ::= block "." .
void program()
{
    getToken();
    block();

    if(token != periodsym)
        error(9); //Period expected

    emit(11, 0, 3); //SIO 0,3 (Halt)
}

//block ::= const-declaration  var-declaration procedure-declaration statement.
void block()
{
    int var = 0;

    currL++;

    int jmpaddr = currCode;

    emit(7, 0, 999);

    if(token == constsym)
        constDeclaration();

    if(token == varsym)
        var = varDeclaration();

    if(token == procsym)
        procDeclaration();

    code[jmpaddr].m = currCode;

    emit(6, 0, currM + var); //INC

    statement();

    emit(2, 0, 0);

    while(symbol_table[currSym - 1].level == currL)
        currSym--;

    currL--;
}

//const-declaration ::= [ “const” ident "=" number {"," ident "=" number} “;"].
void constDeclaration()
{
    do
    {
        getToken();

        if(token != identsym)
            error(4);

        getToken();

        if(token != eqsym)
            error(3);

        getToken();

        if(token != numbersym)
            error(2);

        enter(1, identifier, value, 0, 0);

        getToken();
    }
    while(token == commasym);

    if(token != semicolonsym)
        error(5);

    getToken();
}

//var-declaration  ::= [ "var" ident {"," ident} “;"].
int varDeclaration()
{
    int var = 0;

    do
    {
        getToken();

        if(token != identsym)
            error(4);

        getToken();

        int i = find(identifier);

        if(i == -1 || i == 0 && symbolLevel(i) != currL)
            enter(2, identifier, 0, currL, currM + var);
        else
            error(29);

        var++;
    }
    while(token == commasym);

    if(token != semicolonsym)
        error(5);

    getToken();

    return var;
}

//procedure-declaration ::= { "procedure" ident ";" block ";" }.
void procDeclaration()
{
    getToken();

    if(token != identsym)
        error(4);

    getToken();

    int i = find(identifier);

    if(i == -1)
        enter(3, identifier, 0, currL, currCode);
    else
        error(99);

    if(token != semicolonsym)
        error(5);

    getToken();

    block();

    if(token != semicolonsym)
        error(5);

    getToken();
}

//statement   ::= [ ident ":=" expression
//                | "call" ident
//	      	      | "begin" statement { ";" statement } "end"
//	      	      | "if" condition "then" statement
//		          | "while" condition "do" statement
//                | "read" ident
//		          | "write"  ident
//	      	      | e ] .
void statement()
{
    if(token == identsym)
    {
        int i;

        i = find(identifier);

        if(symbolType(i) == 1 || symbolType(i) == 3)
            error(12);
        if(symbolType(i) == 0)
            error(11);

        getToken();

        if(token != becomessym)
        {
            if(token == eqsym)
                error(1);

            error(13);
        }

        getToken();

        expression();

        emit(4, currL - symbolLevel(i), symbolAddress(i)); //STO
    }

    else if(token == callsym)
    {
        getToken();

        int i;

        if(token != identsym)
            error(4);

        i = find(identifier);

        if(symbolType(i) == 1 || symbolType(i) == 2)
            error(15);
        if(symbolType(i) == 0)
            error(11);
        if(symbolType(i) == 3)
            emit(5, currL - symbolLevel(i), symbolAddress(i));
        else
            error(99);

        getToken();
    }

    else if(token == beginsym)
    {
        getToken();

        statement();

        while(token == semicolonsym)
        {
            getToken();
            statement();
        }

        if(token != endsym)
            error(10);

        getToken();
    }

    else if(token == ifsym)
    {
        int jpcLoc, jmpLoc;

        fpos_t pos;

        getToken();

        condition();

        if(token != thensym)
            error(16);

        getToken();

        jpcLoc = currCode;

        emit(8, 0, 0); //JPC

        statement();

        jmpLoc = currCode;

        emit(7, 0, 0);

        code[jpcLoc].m = currCode;

        fgetpos(ifp, &pos);

        if(token == elsesym)
        {
            getToken();
            statement();
            code[jmpLoc].m = currCode;
        }
        else
        {
            fsetpos(ifp, &pos);
            currCode--;
            code[jpcLoc].m = currCode;
        }
    }

    else if(token == whilesym)
    {
        int conditionLoc, jpcLoc;

        conditionLoc = currCode;

        getToken();

        condition();

        jpcLoc = currCode;

        if(token != dosym)
            error(18);

        emit(8, 0, 0); //JPC

        getToken();

        statement();

        emit(7, 0, conditionLoc); //JMP

        code[jpcLoc].m = currCode;
    }

    else if(token == readsym)
    {
        int i;

        getToken();

        if(token != identsym)
            error(28);

        i = find(identifier);

        if(symbolType(i) == 1)
            error(12);

        emit(10, 0, 2); //SIO 0, 2 (Read input)

        emit(4, currL - symbolLevel(i), symbolAddress(i)); //STO

        getToken();

        statement();
    }

    else if(token == writesym)
    {
        int i;

        getToken();

        if(token != identsym)
            error(28);

        i = find(identifier);

        emit(3, currL - symbolLevel(i), symbolAddress(i)); //LOD

        emit(9, 0, 1); //SIO, 0, 1 (Write to top of stack)

        getToken();

        statement();
    }
}

//condition ::= "odd" expression
void condition()
{
    if(token == oddsym)
    {
        getToken();
        expression();
        emit(2, 0, 6);
    }

    else
    {
        expression();

        if(token < eqsym || token > geqsym)
            error(20);

        int relop = token;

        getToken();
        expression();

        switch(relop)
        {
            case eqsym:
                emit(2, 0, 8);
                break;

            case neqsym:
                emit(2, 0, 9);
                break;

            case lessym:
                emit(2, 0, 10);
                break;

            case leqsym:
                emit(2, 0, 11);
                break;

            case gtrsym:
                emit(2, 0, 12);
                break;

            case geqsym:
                emit(2, 0, 13);
                break;

            default:
                break;
        }
    }
}

//expression ::= [ "+"|"-"] term { ("+"|"-") term}.
void expression()
{
    int addop = 0;

    if(token == plussym)
        getToken();

    else if(token == minussym)
    {
        addop = token;

        getToken();
    }

    else if(token != numbersym && token != identsym)
    {
        error(24);
    }

    term();

    if(addop == minussym)
        emit(2, 0, 1); //NEG

    while(token == plussym || token == minussym)
    {
        addop = token;

        getToken();
        term();

        if(addop == plussym)
            emit(2, 0, 2); //ADD
        else
            emit(2, 0, 3); //SUB
    }
}

//term ::= factor {("*"|"/") factor}.
void term()
{
    int mulop;

    factor();

    while(token == multsym || token == slashsym)
    {
        mulop = token;

        getToken();
        factor();

        if(mulop == multsym)
            emit(2, 0, 4); //MULT
        else
            emit(2, 0, 5); //DIV
    }
}

//factor ::= ident | number | "(" expression ")“.
void factor()
{
    if(token == identsym)
    {
        int i = find(identifier);

        if(symbolType(i) == 1)
            emit(1, currL - symbolLevel(i), symbol_table[i].val); //LIT
        else if(symbolType(i) == 2)
            emit(3, currL - symbolLevel(i), symbolAddress(i)); //LOD
        else
        {
            if(symbolType(i) == 3)
                error(21);
            else if(symbolType(i) == 0)
                error(11);
            else
                error(99); //This shouldn't happen!
        }

        getToken();
    }

    else if(token == numbersym)
    {
        emit(1, 0, value); //LIT
        getToken();
    }

    else if(token == lparentsym)
    {
        getToken();
        expression();

        if(token != rparentsym)
            error(22);

        getToken();
    }

    else
        error(23);
}

//Throws a paticular error depending on the situation.
void error(int type)
{
    printf("Error #%d: ", type);

    switch(type)
    {
        case 1:
            printf("Use = instead of :=.\n");
            exit(EXIT_FAILURE);

        case 2:
            printf("= must be followed by a number.\n");
            exit(EXIT_FAILURE);

        case 3:
            printf("Identifier must be followed by =.\n");
            exit(EXIT_FAILURE);

        case 4:
            printf("const, var, procedure must be followed by an identifier.\n");
            exit(EXIT_FAILURE);

        case 5:
            printf("Semicolon or comma missing.\n");
            exit(EXIT_FAILURE);

        case 6:
            printf("Incorrect symbol after procedure declaration.\n");
            exit(EXIT_FAILURE);


        case 7:
            printf("Statement expected.\n");
            exit(EXIT_FAILURE);

        case 8:
            printf("Incorrect symbol after statement part in block.\n");
            exit(EXIT_FAILURE);

        case 9:
            printf("Period expected.\n");
            exit(EXIT_FAILURE);

        case 10:
            printf("Semicolon between statements missing.\n");
            exit(EXIT_FAILURE);

        case 11:
            printf("Undeclared identifier.\n");
            exit(EXIT_FAILURE);

        case 12:
            printf("Assignment to constant or procedure is not allowed.\n");
            exit(EXIT_FAILURE);

        case 13:
            printf("Assignment operator expected.\n");
            exit(EXIT_FAILURE);

        case 14:
            printf("call must be followed by an identifier.\n");
            exit(EXIT_FAILURE);

        case 15:
            printf("Call of a constant or variable is meaningless.\n");
            exit(EXIT_FAILURE);

        case 16:
            printf("then expected.\n");
            exit(EXIT_FAILURE);

        case 17:
            printf("Semicolon or } expected.\n");
            exit(EXIT_FAILURE);

        case 18:
            printf("do expected.\n");
            exit(EXIT_FAILURE);

        case 19:
            printf("Incorrect symbol following statement.\n");
            exit(EXIT_FAILURE);

        case 20:
            printf("Relational operator expected.\n");
            exit(EXIT_FAILURE);

        case 21:
            printf("Expression must not contain a procedure identifier.\n");
            exit(EXIT_FAILURE);

        case 22:
            printf("Right parenthesis missing.\n");
            exit(EXIT_FAILURE);

        case 23:
            printf("The preceding factor cannot begin with this symbol.\n");
            exit(EXIT_FAILURE);

        case 24:
            printf("An expression cannot begin with this symbol.\n");
            exit(EXIT_FAILURE);

        case 25:
            printf("This number is too large.\n");
            exit(EXIT_FAILURE);

        case 26:
            printf("Code is too long.\n");
            exit(EXIT_FAILURE);

        case 27:
            printf("Procedures have not been implemented.\n");
            exit(EXIT_FAILURE);

        case 28:
            printf("Identifier expected after read/write.\n");
            exit(EXIT_FAILURE);

        case 29:
            printf("Identifier can't be declared more than once in the same lexigraphical level.\n");
            exit(EXIT_FAILURE);

        default:
            exit(EXIT_FAILURE);
    }
}
