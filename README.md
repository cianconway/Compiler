# Compiler

1. A formal EBNF grammar for a computer language (the "Compiler Project Language", CPL).<br>
2. Routines for:<br>
  Character handling<br>
  String table manipulation<br>
  Scanning<br>
  Symbol table manipulation<br>
3. An interpreter for the target machine assembly language ("sim" - command line, or "simgui" - graphical)<br><br>

<h2>Conatains following files</h2><br><br>

<h3>parser1.c</h3><br>
This is a pure parser for the CPL. This program accepts all syntactically valid CPL programs and rejects all invalid ones. 
On acceptance the program prints "valid", on rejection it stops parsing at the first syntax error in the CPL source 
being analysed and prints "syntax error".<br><br>

<h3>parser2.c</h3><br>
This is a pure parser which includes full syntax error detection and recovery. This reports syntax errors by type, 
recovers from them and continues parsing the CPL source being analysed.<br><br>

<h3>comp1.c</h3><br>
This is a compiler performing syntax and semantic error detection and code generation for the CPL language, 
excluding procedure definitions. (comp1 handles READ and WRITE statements fully. 
These look like procedure calls syntactially, but they compile to Read and Write instructions).<br><br>

<h3>comp2.c</h3><br>
This is a full compiler performing syntax and semantic error detection and code generation for all parts of the CPL language, 
including procedure definitions with parameters.
