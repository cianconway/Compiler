/*--------------------------------------------------------------------------*/
/*                                                                          */
/*       parser1                                                            */
/*                                                                          */
/*       Group Member(s)              ID Number(s)                          */
/*       Cian Conway                  10126767                              */
/*       Pa Stapleton                 10122834                              */
/*       Ailbe O'Donoghue             10124705                                                                   */
/*                                                                          */
/*       Skeleton parser file for parser1.  This is essentially just a      */
/*       copy of smallparser.c, here as a placeholder for the Qt project.   */
/*       You will need to edit this file to generate your version of        */
/*       parser1.c,                                                         */
/*       a program that can *parse* (not compile) all aspects               */
/*       of a CPL program fed into it and report if that CPL program        */
/*       contains a syntax error or is syntactically correct.               */
/*                                                                          */
/*                                                                          */
/*       Replace the smallparser recursive-descent parser given here with   */
/*       a set of recursive-descent procedures implementing the CPL         */
/*       grammar.                                                           */
/*                                                                          */
/*       Although the listing file generator has to be initialised in       */
/*       this program, full listing files cannot be generated in the        */
/*       presence of errors because of the "crash and burn" error-          */
/*       handling policy adopted in parser1. Only the first error is        */
/*       reported, the remainder of the input is simply copied to the       */
/*       (using the routine "ReadToEndOfFile") without further comment.     */
/*       Note that this is appropriate behaviour for parser1.c but not      */
/*       for its successors, parser2.c, comp1.c and comp2.c.                */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "scanner.h"
#include "line.h"

PRIVATE FILE *InputFile;
PRIVATE FILE *ListFile;

PRIVATE TOKEN  CurrentToken;

PRIVATE int  OpenFiles( int argc, char *argv[] );
PRIVATE void ParseProgram( void );
PRIVATE void ParseStatement( void );
PRIVATE void ParseExpression( void );
PRIVATE void Accept( int code );
PRIVATE void ReadToEndOfFile( void );
PRIVATE void ParseDeclaration( void );
PRIVATE void ParseProcDeclaration( void );
PRIVATE void ParseParameterList( void );
PRIVATE void ParseFormalParameter( void );
PRIVATE void ParseBlock( void );
PRIVATE void ParseSimpleStatement( void );
PRIVATE void ParseRestOfStatement( void );
PRIVATE void ParseProcCallList( void );
PRIVATE void ParseAssignment( void );
PRIVATE void ParseActualParameter( void );
PRIVATE void ParseWhileStatement( void );
PRIVATE void ParseIfStatement( void );
PRIVATE void ParseReadStatement( void );
PRIVATE void ParseWriteStatement( void );
PRIVATE void ParseCompoundTerm( void );
PRIVATE void ParseTerm( void );
PRIVATE void ParseSubTerm( void );
PRIVATE void ParseBooleanExpression( void );
PRIVATE void ParseAddOp( void );
PRIVATE void ParseMultOp( void );
PRIVATE void ParseRelOp( void );
PRIVATE void ParseVariable( void );
/*PRIVATE void ParseIntConst( void );
PRIVATE void ParseIdentifier( void );
PRIVATE void ParseAlphaNum( void );
PRIVATE void ParseAlpha( void );
PRIVATE void ParseDigit( void );
*/

PUBLIC int main ( int argc, char *argv[] )
{
    if ( OpenFiles( argc, argv ) )  {
        InitCharProcessor( InputFile, ListFile );
        CurrentToken = GetToken();
        ParseProgram();
        fclose( InputFile );
        fclose( ListFile );
        printf("valid\n");
        return  EXIT_SUCCESS;
    }
    else 
        return EXIT_FAILURE;
}


PRIVATE void ParseProgram( void )
{
    while ( CurrentToken.code == PROGRAM )  {
        Accept(PROGRAM);
        Accept (IDENTIFIER);
        Accept (SEMICOLON);
        if(CurrentToken.code == VAR) ParseDeclaration();
        while(CurrentToken.code == PROCEDURE) ParseProcDeclaration();
        ParseBlock();
        Accept( ENDOFPROGRAM );
    }

}

PRIVATE void ParseDeclaration( void )
{
    Accept(VAR);
    ParseVariable();
    while ( CurrentToken.code == COMMA )  {
        Accept (COMMA);
        ParseVariable();
    }
    Accept( SEMICOLON );
}

PRIVATE void ParseProcDeclaration( void )
{
    Accept( PROCEDURE );
    Accept( IDENTIFIER );
    while (CurrentToken.code == PROCEDURE)  {
        ParseProcDeclaration();
        }
    if (CurrentToken.code == LEFTPARENTHESIS)
    {
        ParseParameterList();
    }
    Accept( SEMICOLON );

    if (CurrentToken.code == VAR)
    {
        ParseDeclaration();
    }
    ParseBlock();
    Accept( SEMICOLON );
}

PRIVATE void ParseParameterList( void )
{
    Accept(LEFTPARENTHESIS);
    ParseFormalParameter();
    while(CurrentToken.code == COMMA) {
        ParseFormalParameter();
        Accept(COMMA);
    }
    Accept(RIGHTPARENTHESIS);
}

PRIVATE void ParseFormalParameter( void ){
    if(CurrentToken.code == REF) {
        ParseVariable();
        Accept(REF);
    }
    else{
        ParseVariable();
    }
}


PRIVATE void ParseBlock( void ){
    Accept( BEGIN );
    while (CurrentToken.code == WHILE || CurrentToken.code == IF ||
           CurrentToken.code == READ || CurrentToken.code == WRITE ||
           CurrentToken.code ==IDENTIFIER )  {
            ParseStatement();
        Accept( SEMICOLON );
    }
    Accept( END );

}

PRIVATE void ParseStatement( void ){

    if(CurrentToken.code == IDENTIFIER){
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

PRIVATE void ParseSimpleStatement(void){
    AcceptIdentifier();
    ParseVariable();
    ParseRestOfStatement();
}

PRIVATE void ParseRestOfStatement(void){

    if(CurrentToken.code == LEFTPARENTHESIS || IDENTIFIER){
        ParseProcCallList();
    }
    else if(CurrentToken.code == ASSIGNMENT){
      ParseAssignment();
    }
    else if (CurrentToken.code == SEMICOLON) {
            Accept( SEMICOLON );
        }

}

PRIVATE void ParseProcCallList(void){
    Accept(LEFTPARENTHESIS);
    ParseActualParameter();
    while(CurrentToken.code == COMMA){
        Accept(COMMA);
        ParseActualParameter();
    }
    Accept(RIGHTPARENTHESIS);
}

PRIVATE void ParseAssignment(void){
    Accept(ASSIGNMENT);
    ParseExpression();
}

PRIVATE void ParseActualParameter(void){
    if(CurrentToken.code == IDENTIFIER){
        ParseVariable();
    }
    else{
        ParseExpression();
    }
}

PRIVATE void ParseWhileStatement(void){
    Accept(WHILE);
    ParseBooleanExpression();
    Accept(DO);
    ParseBlock();
}

PRIVATE void ParseIfStatement(void){
    Accept(IF);
    ParseBooleanExpression();
    Accept(THEN);
    ParseBlock();
    if(CurrentToken.code == ELSE){
        Accept(ELSE);
        ParseBlock();
    }
}


PRIVATE void ParseReadStatement(void){
    Accept(READ);
    ParseProcCallList();
}

PRIVATE void ParseWriteStatement(void){
    Accept(WRITE);
    ParseProcCallList();
}


PRIVATE void ParseExpression( void )
{
    ParseCompoundTerm();
        while (CurrentToken.code == SUBTRACT || CurrentToken.code == ADD) {
            ParseAddOp();
            ParseCompoundTerm();
        }
    }

PRIVATE void ParseCompoundTerm( void )
{
   ParseTerm();
   while(CurrentToken.code == MULTIPLY || CurrentToken.code ==  DIVIDE){
       ParseMultOp();
       ParseTerm();
   }
}


PRIVATE void ParseTerm( void ){
    if ( CurrentToken.code == SUBTRACT ) {
            Accept( SUBTRACT);
            ParseSubTerm();
        }
        else{
            ParseSubTerm();
    }
}


PRIVATE void ParseSubTerm(void){
    if(CurrentToken.code == LEFTPARENTHESIS){
        ParseExpression();
        Accept( RIGHTPARENTHESIS );
    }
    else if(CurrentToken.code == IDENTIFIER){
        ParseVariable();
    }
    else if (CurrentToken.code == INTCONST){
        Accept(INTCONST);
    }
}


PRIVATE void ParseBooleanExpression(void){
    ParseExpression();
    ParseRelOp();
    ParseExpression();
}


PRIVATE void ParseAddOp(void){
    if(CurrentToken.code == ADD){
        Accept(ADD);
    }
    else if (CurrentToken.code == SUBTRACT){
        Accept(SUBTRACT);
    }
}

PRIVATE void ParseMultOp(void){
    if(CurrentToken.code == MULTIPLY){
        Accept(MULTIPLY);
    }
    else if (CurrentToken.code == DIVIDE){
        Accept(DIVIDE);
    }
}

PRIVATE void ParseRelOp(void){
    if(CurrentToken.code == EQUALITY){
        Accept(EQUALITY);
    }
    else if (CurrentToken.code == LESSEQUAL){
        Accept(LESSEQUAL);
    }
    else if (CurrentToken.code == GREATEREQUAL){
        Accept(GREATEREQUAL);
    }
    else if (CurrentToken.code == LESS){
        Accept(LESS);
    }
    else if (CurrentToken.code == GREATER){
        Accept(GREATER);
    }

}


PRIVATE void ParseVariable(void){
    Accept(IDENTIFIER);
}


PRIVATE void Accept( int ExpectedToken )
{
    if ( CurrentToken.code != ExpectedToken )  {
        SyntaxError( ExpectedToken, CurrentToken );
        ReadToEndOfFile();
        fclose( InputFile );
        fclose( ListFile );
        exit( EXIT_FAILURE );
    }
    else  CurrentToken = GetToken();
}

PRIVATE int  OpenFiles( int argc, char *argv[] )
{

    if ( argc != 3 )  {
        fprintf( stderr, "%s <inputfile> <listfile>\n", argv[0] );
        return 0;
    }

    if ( NULL == ( InputFile = fopen( argv[1], "r" ) ) )  {
        fprintf( stderr, "cannot open \"%s\" for input\n", argv[1] );
        return 0;
    }

    if ( NULL == ( ListFile = fopen( argv[2], "w" ) ) )  {
        fprintf( stderr, "cannot open \"%s\" for output\n", argv[2] );
        fclose( InputFile );
        return 0;
    }

    return 1;
}

PRIVATE void ReadToEndOfFile( void ){
    if ( CurrentToken.code != ENDOFINPUT )  {
        Error( "Parsing ends here in this program\n", CurrentToken.pos );
        while ( CurrentToken.code != ENDOFINPUT )  CurrentToken = GetToken();
    }
}
