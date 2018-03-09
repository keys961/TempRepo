%{
	unsigned long wordCount = 0;
	unsigned long letterCount = 0;
	unsigned long lineCount = 0;
%}
%%
\n { lineCount++; letterCount++; }
[A-Za-z]+ { wordCount++; letterCount += yyleng; }
[0-9]+ { wordCount++; letterCount += yyleng; }
. { letterCount++; }
%%
int yywrap()
{
	return 1;
}

int main(int argc, char* argv[])
{
	yyin = fopen(argv[1], "r");
	yylex();
	fclose(yyin);

	printf("Total words: %ld\n", wordCount);
	printf("Total chars: %ld\n", letterCount);
	printf("Total lines: %ld\n", lineCount);	

	return 0;
}