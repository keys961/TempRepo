%{
#define YYSTYPE double
#include <stdio.h>
#include <math.h>
#include <ctype.h>
%}
%token NUM
%left '-' '+'
%left '*' '/'
%left NEGATIVE
%right '^'
%%

input: /*empty string*/
    | input line
    ;
line: '\n'
    |exp '\n' { printf("\tValue = %f\n", $1);}
    ;
exp: NUM {$$=$1;}
    | exp '+' exp {$$=$1+$3;}
    | exp '-' exp {$$=$1-$3;}
    | exp '*' exp {$$=$1*$3;}
    | exp '/' exp {$$=$1/$3;}
    | exp '^' exp {$$=pow($1,$3);}
    | '-' exp %prec NEGATIVE {$$=-$2;}
    | '('exp')' {$$=$2;}
    ;

%%

int main()
{
    printf("\nPlease input your math expression end with ENTER or EOF:\n");
    yyparse();
    return 0;
}

int yyerror(char* msg)
{
    printf("Error: %s\n", msg);
    //return 1;
}

int yylex()
{
    int c;
    while(((c = getchar()) == ' ') 
        || (c == '\t'));
    
    if(c == '.' || isdigit(c))
    {
        ungetc(c, stdin);
        scanf("%lf", &yylval);
        return NUM;
    }

    if(c == EOF)
        return 0;
    
    return c;
}