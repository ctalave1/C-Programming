//Chris Talavera
//9/7/14
//System Software
//P-machine

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_STACK_HEIGHT 2000
#define MAX_CODE_LENGTH 500
#define MAX_LEXI_LEVELS 3

typedef struct instruction
{
    int line; //line number
    int op; //opcode
    int l;  //L
    int m;  //M
}instruction;

typedef struct stackstate
{
    int line;
    int op;
    int l;
    int m;
    int pc;
    int bp;
    int sp;
    int stack[MAX_STACK_HEIGHT];
    int activeAR[MAX_LEXI_LEVELS];
    struct stackstate *next;
}stackstate;

//prototypes
int base(int l, int bp);
void fetch();
void execute();
void oprIdentify();
stackstate* updateStackTrace(stackstate* root);

//global variables
int sp = 0;
int bp = 1;
int pc = 0;
int halt = 0;
int currStackState = 0;
int currAR = -1;

int stack[MAX_STACK_HEIGHT];
int ar[MAX_LEXI_LEVELS];

instruction ir;

instruction code[MAX_CODE_LENGTH];

int main(int argc, const char *argv[])
{
    char* opcodes[] = {"LIT", "OPR", "LOD", "STO", "CAL", "INC", "JMP", "JPC", "SIO", "SIO", "SIO"};

    FILE* ifp;
    FILE* ofp1, *ofp2;


    //Initialization
    stack[1] = 0;
    stack[2] = 0;
    stack[3] = 0;

    ar[0] = -1;
    ar[1] = -1;
    ar[2] = -1;

    ir.op = 0;
    ir.l = 0;
    ir.m = 0;

    int codeCount = 0;
    int i;

    int pcInit = pc;
    int bpInit = bp;
    int spInit = sp;

    stackstate* rootStackState = NULL;

    ifp = fopen("mcode.txt", "r");
    ofp1 = fopen("stacktrace.txt", "w");
    ofp2 = fopen("fullcompileroutput.txt", "a");

    fprintf(ofp2, "stacktrace.txt\n---------------\n");

    //Writes the headings for the instruction section to stacktrace.txt
    fprintf(ofp1, "LINE\tOP\tL\tM\n");
    fprintf(ofp2, "LINE\tOP\tL\tM\n");

    //Reads in the instructions from mcode.txt
    while(fscanf(ifp, "%d %d %d", &code[codeCount].op, &code[codeCount].l, &code[codeCount].m) != EOF)
    {
        code[codeCount].line = codeCount;
        fprintf(ofp1, "%d\t%s\t%d\t%d\n", codeCount, opcodes[code[codeCount].op - 1], code[codeCount].l, code[codeCount].m);
        fprintf(ofp2, "%d\t%s\t%d\t%d\n", codeCount, opcodes[code[codeCount].op - 1], code[codeCount].l, code[codeCount].m);
        codeCount++;
    }

    fclose(ifp);

    //Writes the headings for the stack section to stacktrace.txt
    fprintf(ofp1, "\t\t\t\tPC\tBP\tSP\tstack\n");
    fprintf(ofp1, "Initial values\t\t\t%d\t%d\t%d\n", pc, bp, sp);
    fprintf(ofp2, "\t\t\t\tPC\tBP\tSP\tstack\n");
    fprintf(ofp2, "Initial values\t\t\t%d\t%d\t%d\n", pc, bp, sp);

    //Fetches and executes each instruction until the base pointer is zero
    //or a halt instruction is reached. It also writes the state of the
    //stack after each instruction to stacktrace.txt
    while(1)
    {
        if(halt)
            break;

        if(bp == 0)
            break;

        if(ir.op == 5)
        {
            for(i = 0; i < MAX_LEXI_LEVELS; i++)
            {
                if(ar[i] == -1)
                {
                    ar[i] = sp;
                    currAR = i;
                    break;
                }
            }
        }

        if(ir.op == 2 && ir.m == 0)
        {
            ar[currAR] = -1;
            currAR--;
        }

        fetch();
        execute();

        rootStackState = updateStackTrace(rootStackState);

        fprintf(ofp1, "%d\t%s\t%d\t%d\t%d\t%d\t%d\t", ir.line, opcodes[ir.op - 1], ir.l, ir.m, pc, bp, sp);
        fprintf(ofp2, "%d\t%s\t%d\t%d\t%d\t%d\t%d\t", ir.line, opcodes[ir.op - 1], ir.l, ir.m, pc, bp, sp);

        for(i = 0; i < sp; i++)
        {
            if(i == ar[0] || i == ar[1] || i == ar[2])
            {
                fprintf(ofp1, "| ");
                fprintf(ofp2, "| ");
            }

            fprintf(ofp1, "%d ", stack[i + 1]);
            fprintf(ofp2, "%d ", stack[i + 1]);
        }

        fprintf(ofp1, "\n");
        fprintf(ofp2, "\n");
    }

    fclose(ofp1);
    fclose(ofp2);

    //If the "-v" directive is passed to the p-code machine,
    //print the stack trace to the console.
    if(argc > 1 && strcmp(argv[1], "-v") == 0)
    {
        int j;

        printf("\n");

        printf("Stack Trace\n-----------------\n");

        printf("\t\t\t\tPC\tBP\tSP\tstack\n");
        printf("Initial values\t\t\t%d\t%d\t%d\n", pcInit, bpInit, spInit);

        stackstate *currState = rootStackState;

        for(i = 0; i < currStackState; i++)
        {
            printf("%d\t%s\t%d\t%d\t%d\t%d\t%d\t", currState->line, opcodes[currState->op - 1], currState->l, currState->m, currState->pc, currState->bp, currState->sp);

            for(j = 0; j < currState->sp; j++)
            {
                if(j == currState->activeAR[0] || j == currState->activeAR[1] || j == currState->activeAR[2])
                    printf("| ");

                printf("%d ", currState->stack[j]);
            }

            currState = currState->next;

            printf("\n");
        }
    }

    return 0;
}

stackstate* updateStackTrace(stackstate* root)
{
    int i;

    stackstate* temp = malloc(sizeof(stackstate));
    stackstate* currState = root;

    temp->line = ir.line;
    temp->op = ir.op;
    temp->l = ir.l;
    temp->m = ir.m;
    temp->pc = pc;
    temp->bp = bp;
    temp->sp = sp;

    for(i = 0; i < MAX_LEXI_LEVELS; i++)
    {
        temp->activeAR[i] = ar[i];
    }

    temp->next = 0;

    for(i = 0; i < sp; i++)
        temp->stack[i] = stack[i + 1];

    if(root == 0)
    {
        root = temp;

        currStackState++;

        return root;
    }

    if(root->next == 0)
    {
        root->next = temp;
    }

    else
    {
        while(1)
        {
            if(currState->next == 0)
            {
                currState->next = temp;
                break;
            }

            currState = currState->next;
        }
    }

    currStackState++;

    return root;
}

//Helps find a variable in a different Activation Record some L levels down
int base(l, bp)
{
    int b1; //find base L levels down

    b1 = bp;

    while(l > 0)
    {
        b1 = stack[b1 + 1];
        l--;
    }

    return b1;
}

//Fetches the next instruction from the code store and places it in the IR
//register. Then, it increments the program counter(pc) so that it points
//to the next instruction to be fetched.
void fetch()
{
    ir = code[pc];
    pc++;
}

//Executes the instruction that is currently in the IR register.
void execute()
{
    switch(ir.op)
    {
        //LIT
        case 1:
            sp++;
            stack[sp] = ir.m;
            break;

        //OPR
        case 2:
            oprIdentify();
            break;

        //LOD
        case 3:
            sp++;
            stack[sp] = stack[base(ir.l, bp) + ir.m];
            break;

        //STO
        case 4:
            stack[base(ir.l, bp) + ir.m] = stack[sp];
            sp--;
            break;

        //CAL
        case 5:
            stack[sp + 1] = 0;
            stack[sp + 2] = base(ir.l, bp);
            stack[sp + 3] = bp;
            stack[sp + 4] = pc;
            bp = sp +1;
            pc = ir.m;
            break;

        //INC
        case 6:
            sp += ir.m;
            break;

        //JMP
        case 7:
            pc = ir.m;
            break;

        //JPC
        case 8:
            if(stack[sp] == 0)
                pc = ir.m;

            sp--;
            break;

        //SIO 1
        case 9:
            printf("%d\n", stack[sp]);
            sp--;
            break;

        //SIO 2
        case 10:
            sp++;
            scanf("%d", &stack[sp]);
            break;

        //SIO 3
        case 11:
            halt = 1;
            break;

        default:
            break;
    }
}

//Executes the appropriate operation from a OPR instruction based on the m
//component of the instruction.
void oprIdentify()
{
    switch(ir.m)
    {
        //RET
        case 0:
            sp = bp - 1;
            pc = stack[sp + 4];
            bp = stack[sp + 3];
            break;

        //NEG
        case 1:
            stack[sp] = -stack[sp];
            break;

        //ADD
        case 2:
            sp--;
            stack[sp] = stack[sp] + stack[sp + 1];
            break;

        //SUB
        case 3:
            sp--;
            stack[sp] = stack[sp] - stack[sp + 1];
            break;

        //MUL
        case 4:
            sp--;
            stack[sp] = stack[sp] * stack[sp + 1];
            break;

        //DIV
        case 5:
            sp--;
            stack[sp] = stack[sp] / stack[sp + 1];
            break;

        //ODD
        case 6:
            stack[sp] = stack[sp]%2;
            break;

        //MOD
        case 7:
            sp--;
            stack[sp] = stack[sp] % stack[sp + 1];
            break;

        //EQL
        case 8:
            sp--;
            stack[sp] = stack[sp] == stack[sp + 1] ? 1:0;
            break;

        //NEQ
        case 9:
            sp--;
            stack[sp] = stack[sp] != stack[sp + 1] ? 1:0;
            break;

        //LSS
        case 10:
            sp--;
            stack[sp] = stack[sp] < stack[sp + 1] ? 1:0;
            break;

        //LEQ
        case 11:
            sp--;
            stack[sp] = stack[sp] <= stack[sp + 1] ? 1:0;
            break;

        //GTR
        case 12:
            sp--;
            stack[sp] = stack[sp] > stack[sp + 1] ? 1:0;
            break;

        //GEQ
        case 13:
            sp--;
            stack[sp] = stack[sp] >= stack[sp + 1] ? 1:0;
            break;

        default:
            break;
    }
}
