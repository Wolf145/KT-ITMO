grammar Grammar;

options {
    caseInsensitive = true;
}

program
    : programHeading block DOT EOF
    ;

programHeading
    : PROGRAM identifier SEMI
    ;

identifier
    : IDENT
    ;

block
    : (
        constantDefinitionPart
        | typeDefinitionPart
        | variableDeclarationPart
        | procedureAndFunctionDeclarationPart
    )* compoundStatement
    ;

blockInside
    : (
        constantDefinitionPart
        | variableDeclarationPart
    )* compoundStatement
    ;

constantDefinitionPart
    : CONST (constantDefinition SEMI)+
    ;

constantDefinition
    : identifier EQUAL constant
    ;

constantChr
    : CHR LPAREN unsignedInteger RPAREN
    ;

constant
    : unsignedNumber
    | sign unsignedNumber
    | identifier
    | sign identifier
    | string
    | constantChr
    | bool_
    ;

unsignedNumber
    : unsignedInteger
    | unsignedReal
    ;

unsignedInteger
    : NUM_INT
    ;

unsignedReal
    : NUM_REAL
    ;

sign
    : PLUS
    | MINUS
    ;

bool_
    : TRUE
    | FALSE
    ;

string
    : STRING_LITERAL
    ;

typeDefinitionPart
    : TYPE (typeDefinition SEMI)+
    ;

typeDefinition
    : identifier EQUAL (arrayType | recordType | scalarType)
    ;

type_
    : simpleType
    | arrayType
    | recordType
    | pointerType
    ;

simpleType
    : scalarType
    | typeIdentifier
    | stringType
    ;

scalarType
    : LPAREN identifierList RPAREN
    ;

typeIdentifier
    : identifier
    | CHAR
    | BOOLEAN
    | INTEGER
    | REAL
    | STRING
    ;

stringType
    : STRING LBRACK (identifier | unsignedInteger) RBRACK
    ;

arrayType
    : ARRAY LBRACK typeList RBRACK OF type_
    ;

typeList
    : rangeList (COMMA rangeList)*
    ;

rangeList
    : unsignedInteger DOTDOT unsignedInteger
    ;

recordType
    : RECORD fixedPart END
    ;

fixedPart
    : (recordSection SEMI)+
    ;

recordSection
    : identifierList COLON type_
    ;

pointerType
    : (POINTER)+ typeIdentifier
    ;

variableDeclarationPart
    : VAR (variableDeclaration SEMI)+
    ;

variableDeclaration
    : identifierList COLON type_
    ;

procedureAndFunctionDeclarationPart
    : procedureOrFunctionDeclaration SEMI
    ;

procedureOrFunctionDeclaration
    : procedureDeclaration
    | functionDeclaration
    ;

procedureDeclaration
    : PROCEDURE identifier LPAREN (formalParameterList)? RPAREN SEMI blockInside
    ;

formalParameterList
    : parameterGroup (SEMI parameterGroup)*
    ;

parameterGroup
    : identifierList COLON typeIdentifier
    ;

identifierList
    : VAR? identifier (COMMA identifier)*
    ;

constList
    : constant (COMMA constant)*
    ;

functionDeclaration
    : FUNCTION identifier LPAREN (formalParameterList)? RPAREN COLON resultType SEMI blockInside
    ;

resultType
    : typeIdentifier
    ;

statement
    : simpleStatement
    | structuredStatement
    ;

simpleStatement
    : assignmentStatement
    | procedureStatement
    | emptyStatement_
    | BREAK
    | CONTINUE
    ;

assignmentStatement
    : variable ASSIGN expression
    ;

variable
    : (AT identifier | identifier) (access)*
    ;

access
    : arrayAccess
    | structAccess
    | ptrAccess
    ;

arrayAccess
    : LBRACK expression (COMMA expression)* RBRACK
    ;

structAccess
    : DOT identifier
    ;

ptrAccess
    : POINTER
    ;


expression
    : simpleExpression (relationaloperator expression)?
    ;

relationaloperator
    : EQUAL
    | NOT_EQUAL
    | LT
    | LE
    | GE
    | GT
    ;

simpleExpression
    : term (additiveoperator simpleExpression)?
    ;

additiveoperator
    : PLUS
    | MINUS
    | OR
    ;

term
    : signedFactor (multiplicativeoperator term)?
    ;

multiplicativeoperator
    : STAR
    | SLASH
    | DIV
    | MOD
    | AND
    ;

signedFactor
    : (PLUS | MINUS)? factor
    ;

factor
    : variable
    | LPAREN expression RPAREN
    | functionDesignator
    | unsignedConstant
    | NOT factor
    | bool_
    | string
    ;

unsignedConstant
    : unsignedNumber
    | constantChr
    | string
    | NIL
    ;

functionDesignator
    : identifier LPAREN parameterList? RPAREN
    ;

procedureStatement
    : identifier LPAREN parameterList? RPAREN
    ;

parameterList
    : expression (COMMA expression)*
    ;

emptyStatement_
    :
    ;

structuredStatement
    : compoundStatement
    | conditionalStatement
    | repetetiveStatement
    ;

compoundStatement
    : BEGIN statements END
    ;

statements
    : statement (SEMI statement)*
    ;

conditionalStatement
    : ifStatement
    | switchStatement
    ;

ifStatement
    : IF expression THEN statement (elseIfStatement)* elseStatement?
    ;

elseIfStatement
    : ELSE IF expression THEN statement
    ;

elseStatement
    : ELSE statement
    ;

switchStatement
    : CASE expression OF caseListElement (SEMI caseListElement)* (defaultStatement)? END
    ;

defaultStatement
    : SEMI ELSE statement
    ;

caseListElement
    : constList COLON statement
    ;

repetetiveStatement
    : whileStatement
    | repeatStatement
    | forStatement
    ;

whileStatement
    : WHILE expression DO statement
    ;

repeatStatement
    : REPEAT statements UNTIL expression
    ;

forStatement
    : FOR forList DO statement
    ;

forList
    : identifier ASSIGN initialValue (TO | DOWNTO) finalValue
    ;

initialValue
    : expression
    ;

finalValue
    : expression
    ;

AND
    : 'AND'
    ;

ARRAY
    : 'ARRAY'
    ;

BEGIN
    : 'BEGIN'
    ;

BOOLEAN
    : 'BOOLEAN'
    ;

CASE
    : 'CASE'
    ;

CHAR
    : 'CHAR'
    ;

CHR
    : 'CHR'
    ;

CONST
    : 'CONST'
    ;

DIV
    : 'DIV'
    ;

DO
    : 'DO'
    ;

DOWNTO
    : 'DOWNTO'
    ;

ELSE
    : 'ELSE'
    ;

END
    : 'END'
    ;

FOR
    : 'FOR'
    ;

FUNCTION
    : 'FUNCTION'
    ;

IF
    : 'IF'
    ;

INTEGER
    : 'INTEGER'
    ;

MOD
    : 'MOD'
    ;

NIL
    : 'NIL'
    ;

NOT
    : 'NOT'
    ;

OF
    : 'OF'
    ;

OR
    : 'OR'
    ;

PROCEDURE
    : 'PROCEDURE'
    ;

PROGRAM
    : 'PROGRAM'
    ;

REAL
    : 'REAL'
    ;

RECORD
    : 'RECORD'
    ;

REPEAT
    : 'REPEAT'
    ;

THEN
    : 'THEN'
    ;

TO
    : 'TO'
    ;

TYPE
    : 'TYPE'
    ;

UNTIL
    : 'UNTIL'
    ;

VAR
    : 'VAR'
    ;

WHILE
    : 'WHILE'
    ;

PLUS
    : '+'
    ;

MINUS
    : '-'
    ;

STAR
    : '*'
    ;

SLASH
    : '/'
    ;

ASSIGN
    : ':='
    ;

COMMA
    : ','
    ;

SEMI
    : ';'
    ;

COLON
    : ':'
    ;

EQUAL
    : '='
    ;

NOT_EQUAL
    : '<>'
    ;

LT
    : '<'
    ;

LE
    : '<='
    ;

GE
    : '>='
    ;

GT
    : '>'
    ;

LPAREN
    : '('
    ;

RPAREN
    : ')'
    ;

LBRACK
    : '['
    ;

RBRACK
    : ']'
    ;

POINTER
    : '^'
    ;

AT
    : '@'
    ;

DOT
    : '.'
    ;

DOTDOT
    : '..'
    ;

STRING
    : 'STRING'
    ;

TRUE
    : 'TRUE'
    ;

FALSE
    : 'FALSE'
    ;

WS
    : [ \t\r\n] -> skip
    ;

BREAK
    : 'BREAK'
    ;
CONTINUE
    : 'CONTINUE'
    ;

COMMENT_1
    : '(*' .*? '*)' -> skip
    ;

COMMENT_2
    : '{' .*? '}' -> skip
    ;

IDENT
    : ('A' .. 'Z') ('A' .. 'Z' | '0' .. '9' | '_')*
    ;

STRING_LITERAL
    : '\'' ('\'\'' | ~ ('\''))* '\''
    ;

NUM_INT
    : ('0' .. '9')+
    ;

NUM_REAL
    : ('0' .. '9')+ (('.' ('0' .. '9')+ (EXPONENT)?)? | EXPONENT)
    ;

fragment EXPONENT
    : ('E') ('+' | '-')? ('0' .. '9')+
    ;