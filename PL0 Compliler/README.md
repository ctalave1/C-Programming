Chris Talavera
Kyle Reynolds

**To Compile:**

*Compile Driver (Linux):* gcc -o compile compile.c

*Compile Driver (Windows):* gcc -o compile compilewin.c

*Scanner:* gcc -o scanner scanner.c

*Parser/Code Generator:* gcc -o parser parser.c

*P-code Machine:* gcc -o pmachine pmachine.c

**To Run (Linux):**

*Compile Driver:* ./compile [-l] [-a] [-v]

*Scanner:* ./scanner [-l]

*Parser/Code Generator:* ./parser [-a]

*P-code Machine:* ./pmachine [-v]

**To Run (Windows):**

*Compile Driver:* compile [-l] [-a] [-v]

*Scanner:* scanner [-l]

*Parser/Code Generator:* parser [-a]

*P-code Machine:* pmachine [-v]

**Side Note: If the compile driver appears to be frozen before printing the stack trace, it is most likely waiting for input from the user. **
**Check the input file to see what it wants as input.**

*| | denotes information that should not be included in the command*
*[] denotes optional items*
