%{
#include <string.h>
#include "util.h"
#include "symbol.h"
#include "errormsg.h"
#include "absyn.h"
#include "y.tab.h"

int charPos=1;

int yywrap(void)
{
 charPos=1;
 return 1;
}

void adjust(void)
{
 EM_tokPos=charPos;
 charPos+=yyleng;
}
/*
* Please don't modify the lines above.
* You can add C declarations of your own below.
*/

%}
  /* You can add lex definitions here. */
%{
char str[500]={""};
int string_index=0;//means this char should be at the string_index place; 
int string_length=0;//means the string's length showed in the origin file;like"\n"has two char in origin file;
int comment_num=0;
%}
%Start STR COMMENT
%%


<INITIAL>","		{adjust(); return COMMA;}
<INITIAL>":"		{adjust(); return COLON;} 
<INITIAL>";"		{adjust(); return SEMICOLON;} 
<INITIAL>"("		{adjust(); return LPAREN;} 
<INITIAL>")"		{adjust(); return RPAREN;} 
<INITIAL>"["		{adjust(); return LBRACK;} 
<INITIAL>"]"		{adjust(); return RBRACK;} 
<INITIAL>"{"		{adjust(); return LBRACE;} 
<INITIAL>"}"		{adjust(); return RBRACE;} 
<INITIAL>"."		{adjust(); return DOT;} 
<INITIAL>"+"		{adjust(); return PLUS;} 
<INITIAL>"-"		{adjust(); return MINUS;} 
<INITIAL>"*"		{adjust(); return TIMES;} 
<INITIAL>"/"		{adjust(); return DIVIDE;} 
<INITIAL>"="		{adjust(); return EQ;} 
<INITIAL>"<>"		{adjust(); return NEQ;} 
<INITIAL>"<"		{adjust(); return LT;} 
<INITIAL>"<="		{adjust(); return LE;} 
<INITIAL>">"		{adjust(); return GT;} 
<INITIAL>">="		{adjust(); return GE;} 
<INITIAL>"&"		{adjust(); return AND;} 
<INITIAL>"|"		{adjust(); return OR;} 
<INITIAL>":="		{adjust(); return ASSIGN;} 
<INITIAL>array 		{adjust();return ARRAY;}
<INITIAL>if		{adjust(); return IF;} 
<INITIAL>then		{adjust(); return THEN;} 
<INITIAL>else		{adjust(); return ELSE;} 
<INITIAL>while		{adjust(); return WHILE;} 
<INITIAL>for		{adjust(); return FOR;} 
<INITIAL>to		{adjust(); return TO;} 
<INITIAL>do		{adjust(); return DO;} 
<INITIAL>let		{adjust(); return LET;} 
<INITIAL>in		{adjust(); return IN;} 
<INITIAL>end		{adjust(); return END;} 
<INITIAL>of		{adjust(); return OF;} 
<INITIAL>break		{adjust(); return BREAK;} 
<INITIAL>nil		{adjust(); return NIL;} 
<INITIAL>function	{adjust(); return FUNCTION;} 
<INITIAL>var		{adjust(); return VAR;} 
<INITIAL>type		{adjust(); return TYPE;} 
<INITIAL>\t		{adjust();}
<INITIAL>\n		{adjust();}
<INITIAL>" "		{adjust();}
<INITIAL>[0-9]+		{adjust();yylval.ival=atoi(yytext);return INT;}
<INITIAL>[a-zA-Z][a-zA-Z0-9_]*  {adjust();yylval.sval=String(yytext);return ID;}
<INITIAL>\"		{adjust();str[0]='\0';BEGIN STR;}
<INITIAL>"/*"		{adjust();BEGIN COMMENT;comment_num++;}
<INITIAL><<EOF>>	{adjust();yyterminate();}
<INITIAL>.		{adjust();EM_error(EM_tokPos,"invalid expression");}

<STR>\\n		{adjust();str[string_index]='\n';string_index++;string_length+=2;}
<STR>\\t		{adjust();str[string_index]='\t';string_index++;string_length+=2;}
<STR>\\\"		{adjust();str[string_index]='\"';string_index++;string_length+=2;}
<STR>\\\\		{adjust();str[string_index]='\\';string_index++;string_length+=2;}
<STR>\"			{charPos-=(string_length+1);adjust();charPos+=(string_length+1);str[string_index]='\0';if(!string_index)yylval.sval="";else yylval.sval=String(str);string_index=0;string_length=0;BEGIN INITIAL;return STRING;}
<STR>\\[\t\n" "]+\\	{adjust();} 
<STR>\\			{adjust();EM_error(EM_tokPos,"invalid \\");}
<STR>.			{adjust();str[string_index]=*yytext;string_index++;string_length+=1;}


<COMMENT>"/*"		{adjust();comment_num++;}
<COMMENT>"*/"		{adjust();comment_num--;if(!comment_num)BEGIN INITIAL;}
<COMMENT>.		{adjust();}
<COMMENT>\n		{adjust();}
