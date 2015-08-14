/*--------------------------------------------------------------------------*/
/*                                                                          */
/*       comp2                                                              */
/*                                                                          */
/*       Group Member(s)              ID Number(s)                          */
/*       Cian Conway                  10126767                              */
/*       Pa Stapleton                 10122834                              */
/*       Ailbe O'Donoghue             10124705                              */
/*                                                                          */
/*                                                                          */
/*       Skeleton for the comp2.c compiler.  This compiler should be able   */
/*       to compile CPL programs containing all constructs of the CPL       */
/*       language, including procedure definitions and calls.               */
/*       It should output a listing file (like the parsers) and also an     */
/*       assembly-language file (in stack-machine assembly-language).       */
/*       This should contain the compiled version of the input CPL          */
/*       program.   Note that the code here doesn't do this, it is just     */
/*       a copy of the smallparser.c parser, here as a placeholder for      */
/*       the Qt build system.                                               */
/*                                                                          */
/*       comp2 is       													*/
/*       a copy your tested and working version of comp1.c and use it       */
/*       as a base to build comp2.                                          */
/*                                                                          */
/*                                                                          */
/*      														            */
/*                                                                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "strtab.h"
#include "global.h"
#include "scanner.h"
#include "line.h"
#include "code.h"
#include "symbol.h"

PRIVATE FILE *InputFile;
PRIVATE FILE *ListFile;
PRIVATE FILE *AssemblyFile;

PRIVATE TOKEN  CurrentToken;

#define CLASS_PROGRAM		1
#define CLASS_VARIABLE		2
#define CLASS_PROCEDURE		3
#define CLASS_REF			4

int scope = 0;//declare scope

//OpenFiles() was deleted from Parser2.c as we encountered issues, I implemented it directly into the main method

PRIVATE void ParseProgram( void );
PRIVATE void ParseDeclaration( void );
PRIVATE void ParseProcDeclaration( void );
PRIVATE void ParseParameterList( void );
PRIVATE void ParseFormalParameter( void );
PRIVATE void ParseBlock( void );
PRIVATE void ParseStatement( void );
PRIVATE void ParseSimpleStatement( void );
PRIVATE void ParseRestOfStatement();
PRIVATE void ParseProcCallList();
PRIVATE void ParseAssignment( void );
PRIVATE void ParseActualParameter( void );
PRIVATE void ParseWhileStatement( void );
PRIVATE void ParseIfStatement( void );
PRIVATE void ParseReadStatement( void );
PRIVATE void ParseWriteStatement( void );
PRIVATE void ParseExpression( void );
PRIVATE void ParseCompoundTerm( void );
PRIVATE void ParseTerm( void );
PRIVATE void ParseSubTerm( void );
PRIVATE int ParseBooleanExpression( void );
PRIVATE int ParseAddOp( void );
PRIVATE int ParseMultOp( void );
PRIVATE int ParseRelOp( void );
PRIVATE void ParseVariable( void );
PRIVATE void Accept( int code );
/*PRIVATE void ParseIntConst( void );
PRIVATE void ParseIdentifier( void );
PRIVATE void ParseAlphaNum( void );
PRIVATE void ParseAlpha( void );
PRIVATE void ParseDigit( void );
*/

/*S-Algol Error Recovery Variables*/
SET Program1FS;
SET Program2FS;
SET Program1FS_aug;
SET Program2FS_aug;
SET ProgramFBS;
SET DeclarationFS;
SET DeclarationFS_aug;
SET DeclarationFBS;
SET ProcDeclaration1FS;
SET ProcDeclaration2FS;
SET ProcDeclaration1FS_aug;
SET ProcDeclaration2FS_aug;
SET ProcDeclarationFBS;
SET ProgramFS;
SET ProgramFS_aug;
SET ProgramFBS;
SET BlockFS;
SET BlockFBS;
SET BlockFS_aug;
SET ProcCallFS;
SET ProcCallFBS;
SET ProcCallFS_aug;


PRIVATE void Synchronise( SET *F, SET *FB );
PRIVATE void SetupSets( void );
PRIVATE void MakeSymbolTableEntry(int clas);
PRIVATE SYMBOL *LookupSymbol(void);



PUBLIC int main ( int argc, char *argv[] ){
     if ( argc != 4 )  {
        fprintf( stderr, "%s <inputfile> <listfile> <assemblyfile>\n", argv[0] );
        exit( EXIT_FAILURE );
    }
    else {
        if ( NULL == ( InputFile = fopen( argv[1], "r" ) ) )  {
            fprintf( stderr, "cannot open \"%s\" for input\n", argv[1] );
        }
        else if ( NULL == ( ListFile = fopen( argv[2], "w" ) ) )  {
            fprintf( stderr, "cannot open \"%s\" for output\n", argv[2] );
        }
        else if ( NULL == ( AssemblyFile = fopen( argv[3], "w" ) ) )  {
            fprintf( stderr, "cannot open \"%s\" for output\n", argv[3] );
        }
        else  {
            SetupSets();
            InitCharProcessor( InputFile, ListFile );
            InitCodeGenerator( AssemblyFile );
            CurrentToken = GetToken();
            ParseProgram();
            DumpSymbols( -1 );
            WriteCodeFile();
            fclose( InputFile );
            fclose( ListFile );
        }
    }
    return  EXIT_SUCCESS;
}

PRIVATE void ParseProgram( void ){
	int varaddress = 0;
	scope = 0;
	Accept( PROGRAM );
	MakeSymbolTableEntry(CLASS_PROGRAM);
	Accept( IDENTIFIER ); 
	Accept( SEMICOLON ); 
			
	Synchronise( &Program1FS_aug, &ProgramFBS );	
	
	if ( CurrentToken.code == VAR )	{
		ParseDeclaration();
		varaddress++;
	}
	
	Synchronise( &Program2FS_aug, &ProgramFBS );
	
	while ( CurrentToken.code == PROCEDURE ){
		ParseProcDeclaration();
		Synchronise( &Program2FS_aug, &ProgramFBS );
	}
	
	if ( varaddress > 0 ) {
		Emit( I_INC, varaddress );
	}
	ParseBlock();
    Accept( ENDOFPROGRAM );
    _Emit( I_HALT );
    ReadToEndOfFile();
}

PRIVATE void ParseDeclaration( void ){
	int count = 1;

	Accept( VAR );
	MakeSymbolTableEntry(CLASS_VARIABLE);
	Accept(IDENTIFIER);
	while (CurrentToken.code== COMMA){
		Accept( COMMA );
		count++;//increment count
		MakeSymbolTableEntry(CLASS_VARIABLE);
		Accept(IDENTIFIER);
	}
	Accept( SEMICOLON );
	Emit(I_INC, count);
}

PRIVATE void ParseProcDeclaration( void ){
	Accept( PROCEDURE );
	MakeSymbolTableEntry(CLASS_PROCEDURE);
	Accept( IDENTIFIER );
	scope++;
	if (CurrentToken.code == LEFTPARENTHESIS){
		ParseParameterList();
	}
	Accept( SEMICOLON );

	Synchronise( &ProcDeclaration1FS_aug, &ProcDeclarationFBS );		
	if ( CurrentToken.code == VAR )	{
		ParseDeclaration();
	}
	
	Synchronise( &ProcDeclaration2FS_aug, &ProcDeclarationFBS );		
	while ( CurrentToken.code == PROCEDURE ){
		ParseProcDeclaration();
		Synchronise( &ProcDeclaration2FS_aug, &ProcDeclarationFBS );
	}
	ParseBlock();
	Accept( SEMICOLON );
	scope--;//decrement scope
}

PRIVATE void ParseParameterList( void )
{
	Accept( LEFTPARENTHESIS );
	ParseFormalParameter();
	while (CurrentToken.code == COMMA)
	{
		Accept(COMMA);
		ParseFormalParameter();
	}
	Accept( RIGHTPARENTHESIS );
}

PRIVATE void ParseFormalParameter( void )
{
	if ( CurrentToken.code == REF ){
		Accept(REF);
		MakeSymbolTableEntry(CLASS_REF);
		ParseVariable();
	}else{
		ParseVariable();
	}
}

PRIVATE void ParseBlock( void ){
	Accept( BEGIN );
	Synchronise( &BlockFS_aug, &BlockFBS );		
	while ( InSet( &BlockFS, CurrentToken.code )){
		ParseStatement();
		Emit(I_LOADI, CurrentToken.value);
		Accept( SEMICOLON );
		Synchronise( &BlockFS_aug, &BlockFBS );
	}
	Accept( END );
}


PRIVATE void ParseStatement( void ){
    if (CurrentToken.code == IDENTIFIER){
		ParseSimpleStatement();
	}
	else if (CurrentToken.code == WHILE){
		ParseWhileStatement();
	}
	else if (CurrentToken.code == IF){
		ParseIfStatement();
	}
	else if (CurrentToken.code == READ){
		ParseReadStatement();
	}
	else if (CurrentToken.code == WRITE){
		ParseWriteStatement();
	}
}

PRIVATE void ParseSimpleStatement( void )
{
	SYMBOL *target;
	target = LookupSymbol();
	ParseVariable();
	ParseRestOfStatement(target);
}

PRIVATE void ParseRestOfStatement( SYMBOL *target )
{
	if (CurrentToken.code == LEFTPARENTHESIS){
		ParseProcCallList(PROCEDURE, target);
	}
	else if (CurrentToken.code == ASSIGNMENT) {
		ParseAssignment();
		if ( target != NULL )
			if ( target->class == CLASS_VARIABLE )
				Emit( I_STOREA, target->address );
			else {
				KillCodeGeneration();
			}
	}
	
	else if (CurrentToken.code == SEMICOLON){
		if ( target != NULL )
			if ( target->class == CLASS_PROCEDURE )
				Emit( I_CALL, target->address );
			else {
				KillCodeGeneration();
			}
	}
}

PRIVATE void ParseProcCallList( void ){
	Accept( LEFTPARENTHESIS );
	ParseActualParameter();
	Synchronise( &ProcCallFS_aug, &ProcCallFBS );		
	while ( InSet( &ProcCallFS, CurrentToken.code ) && CurrentToken.code == COMMA )	{
		Accept( COMMA );
		ParseActualParameter();
		Synchronise( &ProcCallFS_aug, &ProcCallFBS );
	}
	Accept( RIGHTPARENTHESIS );
}

PRIVATE void ParseAssignment( void )
{
	Accept( ASSIGNMENT );
	ParseExpression();
}

PRIVATE void ParseActualParameter( void )
{
	ParseExpression();
}

PRIVATE void ParseWhileStatement( void )
{
	int WhileHeadAddress, WhileBackPatchAddress;

	Accept( WHILE );
	WhileHeadAddress = CurrentCodeAddress();
	WhileBackPatchAddress = ParseBooleanExpression();
	Accept( DO );
	ParseBlock();
	Emit(I_BR, WhileHeadAddress);
	BackPatch(WhileBackPatchAddress, CurrentCodeAddress());
}

PRIVATE void ParseIfStatement( void )
{
	int IfHeaderAddress, IfBackPatchAddress;

	Accept( IF );
	IfHeaderAddress = CurrentCodeAddress();
	IfBackPatchAddress = ParseBooleanExpression();
	Accept( THEN );
	ParseBlock();
	Emit(I_BR, IfHeaderAddress);
	BackPatch(IfBackPatchAddress, CurrentCodeAddress());
	
	if ( CurrentToken.code == ELSE )
	{
		Accept( ELSE );
		ParseBlock();
		Emit(I_BR, IfHeaderAddress);
		BackPatch(IfBackPatchAddress, CurrentCodeAddress());
	}		
}

PRIVATE void ParseReadStatement( void ){
	int instruction;

	Accept( READ );
	instruction = I_READ;
	ParseProcCallList();
}

PRIVATE void ParseWriteStatement( void ){
	int instruction;

	Accept( WRITE );
	instruction = I_WRITE;
	ParseProcCallList();
}

PRIVATE void ParseExpression( void ){
	int op;

    ParseCompoundTerm();
	while (( op = CurrentToken.code ) == SUBTRACT || op == ADD)
	{
		ParseAddOp();
		ParseCompoundTerm();
		if (op == ADD )
		{
			_Emit(I_ADD);
		}
		else
		{
			_Emit(I_SUB);
		}
	}
}

PRIVATE void ParseCompoundTerm( void ){
	int op;

	ParseTerm();
	while (( op = CurrentToken.code ) == MULTIPLY || op == DIVIDE){
		ParseMultOp();
		ParseTerm();
		if (op == DIVIDE ){
			_Emit(I_DIV);
		}
		else{
			_Emit(I_MULT);
		}

	}	
}

PRIVATE void ParseTerm( void ){
	int negateflag = 0;

	if ( CurrentToken.code == SUBTRACT )
	{
		negateflag = 1;
		Accept(SUBTRACT);
	}
	ParseSubTerm();
	if(negateflag)
	{
		_Emit(I_NEG);
	}
}

PRIVATE void ParseSubTerm( void ){
	SYMBOL *var;
	if ( CurrentToken.code == LEFTPARENTHESIS ){
		Accept( LEFTPARENTHESIS );
		ParseExpression();
		Accept( RIGHTPARENTHESIS );
	}else if (CurrentToken.code == IDENTIFIER){
		var = LookupSymbol();
		Accept( IDENTIFIER );
		if ( var != NULL && var->class == CLASS_VARIABLE )
			Emit( I_LOADA, var->address );
		else
			KillCodeGeneration();
	}else if (CurrentToken.code == INTCONST){
		Emit(I_LOADI, CurrentToken.value);
		Accept( INTCONST );
	}
}

PRIVATE int ParseBooleanExpression( void ){
	int BackPatchAddress, RelOpInstruction;
	ParseExpression();
	RelOpInstruction = ParseRelOp();
	ParseExpression();
	_Emit(I_SUB);
	BackPatchAddress = CurrentCodeAddress();
	Emit(RelOpInstruction, 0);
	return BackPatchAddress;
}

PRIVATE int ParseAddOp( void ){
	int instruction;

	if(CurrentToken.code == ADD){
        Accept(ADD);
		instruction = I_ADD;
    } 
			
    else if (CurrentToken.code == SUBTRACT){
        Accept(SUBTRACT);
		instruction = I_SUB;
    } 
	return instruction;
}

PRIVATE int ParseMultOp( void ){
	int instruction;

	if(CurrentToken.code == MULTIPLY){
        Accept(MULTIPLY);
		instruction = I_MULT;
    }
    
	else if (CurrentToken.code == DIVIDE){
        Accept(DIVIDE);
		instruction =I_DIV;
    }
	return instruction;
}

PRIVATE int ParseRelOp( void ){
	int instruction;

	switch(CurrentToken.code)//Changed to switch statement from if else in parser2.c for ease of use and readability
    {
		 case GREATER : 
			 instruction = I_BLZ;
			 Accept( GREATER ); 
			 break;
		 case LESS : 
			 instruction = I_BGZ;
			 Accept( LESS ); 
			 break;
		 case GREATEREQUAL : 
			 instruction = I_BL;
			 Accept( GREATEREQUAL ); 
			 break;
		 case LESSEQUAL : 
			 instruction = I_BG;
			 Accept( LESSEQUAL ); 
			 break;
		case EQUALITY :
		default:
			instruction = I_BNZ;
			Accept( EQUALITY ); 
			break;
	}
	return instruction;
}

PRIVATE void ParseVariable( void ){
	Accept( IDENTIFIER );
}

PRIVATE void Accept( int ExpectedToken ){
	static int recovering = 0;

	if (recovering){
		while (CurrentToken.code != ExpectedToken && 
			CurrentToken.code != ENDOFINPUT)
			CurrentToken = GetToken();
	recovering = 0;
	}

    if ( CurrentToken.code != ExpectedToken ){
		SyntaxError( ExpectedToken, CurrentToken );
		recovering = 1;
    }
    else  
		CurrentToken = GetToken();
}
//Commented out Open Files method
//PRIVATE int  OpenFiles( int argc, char *argv[] )
//{
// if ( argc != 4 )  
//	{
//        fprintf( stderr, "%s <inputfile> <listfile> <assemblyfile>\n", argv[0] );
//        return 0;
//    }
//
//    if ( NULL == ( InputFile = fopen( argv[1], "r" ) ) )  
//	{
//		fprintf( stderr, "cannot open \"%s\" for input\n", argv[1] );
//		return 0;
//    }
//
//    if ( NULL == ( ListFile = fopen( argv[2], "w" ) ) )  
//	{
//		fprintf( stderr, "cannot open \"%s\" for output\n", argv[2] );
//		fclose( InputFile );
//		return 0;
//    }
//
//	if ( NULL == ( AssemblyFile = fopen( argv[3], "w" ) ) )  
//	{
//		fprintf( stderr, "cannot open \"%s\" for output\n", argv[3] );
//		fclose( InputFile );
//		return 0;
//    }
//
//    return 1;
//}

PRIVATE void ReadToEndOfFile( void ){
    if ( CurrentToken.code != ENDOFINPUT ) {
		Error( "Parsing ends here in this program\n", CurrentToken.pos );
		while ( CurrentToken.code != ENDOFINPUT )  CurrentToken = GetToken();
    }
}

PRIVATE void Synchronise( SET *F, SET *FB ){
	SET S;

	S = Union( 2, F, FB );
	if ( !InSet( F, CurrentToken.code ) ){
		SyntaxError2( *F, CurrentToken );
		while ( !InSet( &S, CurrentToken.code ) ){
			CurrentToken = GetToken();
		}
	}
}

PRIVATE void SetupSets( void ){
	//<Program>		
	ClearSet( &Program1FS );
	AddElements( &Program1FS, 3, VAR, PROCEDURE, BEGIN );
	ClearSet( &Program2FS );
	AddElements( &Program2FS, 2, PROCEDURE, BEGIN );
	ClearSet( &ProgramFBS );
	AddElements( &ProgramFBS, 3, ENDOFINPUT, END, ENDOFPROGRAM );
	Program1FS_aug = Program1FS;
	AddElement( &Program1FS_aug, ENDOFPROGRAM );
	Program2FS_aug = Program2FS;
	AddElement( &Program2FS_aug, ENDOFPROGRAM );
	//<Declaration>
	ClearSet( &DeclarationFS );
	AddElements( &DeclarationFS, 2, COMMA, SEMICOLON );
	ClearSet( &DeclarationFBS );
	AddElements( &DeclarationFBS, 4, PROCEDURE, BEGIN, ENDOFINPUT, ENDOFPROGRAM );
	//<ProcDeclaration>
	ClearSet( &ProcDeclaration1FS );
	AddElements( &ProcDeclaration1FS, 3, VAR, PROCEDURE, BEGIN );
	ClearSet( &ProcDeclaration2FS );
	AddElements( &ProcDeclaration2FS, 2, PROCEDURE, BEGIN );
	ClearSet( &ProcDeclarationFBS );
	AddElements( &ProcDeclarationFBS, 3, ENDOFINPUT, END, ENDOFPROGRAM );
	ProcDeclaration1FS_aug = ProcDeclaration1FS;
	AddElement( &ProcDeclaration1FS_aug, END );
	ProcDeclaration2FS_aug = ProcDeclaration2FS;
	AddElement( &ProcDeclaration2FS_aug, END );
	// Used for <ProcCallList>
	ClearSet( &ProcCallFS );
	AddElements( &ProcCallFS, 1, COMMA );
	ClearSet( &ProcCallFBS );
	AddElements( &ProcCallFBS, 3, SEMICOLON, ENDOFINPUT, ENDOFPROGRAM );
	ProcCallFS_aug = ProcCallFS;
	AddElement( &ProcCallFS_aug, RIGHTPARENTHESIS );
	//PRIMARY Used for <Block>
	ClearSet( &BlockFS );
	AddElements( &BlockFS, 5, IDENTIFIER, WHILE, IF, READ, WRITE );
	ClearSet( &BlockFBS );
	AddElements( &BlockFBS, 4, ENDOFPROGRAM, SEMICOLON, ELSE, ENDOFINPUT );
	BlockFS_aug = BlockFS;
	AddElement( &BlockFS_aug, END );
}

PRIVATE void MakeSymbolTableEntry(int class){
	static char errorbuffer[M_LINE_WIDTH+2];
	static int varaddress = 0;
	int hashindex;
	SYMBOL *oldsptr, *newsptr;
	char *cptr;

	if ( CurrentToken.code == IDENTIFIER ) {
		if ( NULL == ( oldsptr = Probe( CurrentToken.s, &hashindex )) || oldsptr->scope < scope ) 
		{
			if ( oldsptr == NULL ) {
				cptr = CurrentToken.s; 
			}
			else {
				cptr = oldsptr->s;
			}
			if ( NULL == ( newsptr = EnterSymbol( cptr, hashindex ))) {
				fprintf( stderr, "Symbol Table Incestion Failed\n");
				exit( EXIT_FAILURE );
			}
			else {
				if ( oldsptr == NULL ) 
				{
					PreserveString();
				}
				newsptr->scope = scope;
				newsptr->class = class;
				newsptr->parameters = -1;
				if ( class == CLASS_VARIABLE ) 
				{
					newsptr->address = varaddress; 
					varaddress++;
				}
				else 
				{
					newsptr->address = -1;
				}
			}
		}else { 
			sprintf(errorbuffer, "Identifier \"%ss\" already declared in scope %1d\n", CurrentToken.s, scope);
			Error(errorbuffer, CurrentToken.pos);
		}
	}
}

PRIVATE SYMBOL *LookupSymbol(void){
	static char errorbuffer[M_LINE_WIDTH+2];
	SYMBOL *sptr;

	if (CurrentToken.code == IDENTIFIER){
		sptr = Probe(CurrentToken.s, NULL);
		if (sptr == NULL){
			sprintf(errorbuffer, "Idintifier \"%s\" not declared\n", CurrentToken.s);
			Error(errorbuffer, CurrentToken.pos);
			KillCodeGeneration();
		}
	}else{
		sptr = NULL;
	}
	return sptr;
}
