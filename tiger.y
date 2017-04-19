%{
#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "symbol.h" 
#include "errormsg.h"
#include "absyn.h"

int position;
int yylex(void); /* function prototype */

A_exp absyn_root;

void yyerror(char *s)
{
 EM_error(EM_tokPos, "%s", s);
 exit(1);
}
%}


%union {
	int pos;
	int ival;
	string sval;
	A_var var;
	A_exp exp;
	A_expList exps;
	A_dec dec;
	A_decList decs;
	A_efieldList efields;
	A_ty ty;
	A_namety name;
	A_nametyList names;
	A_fundec fundec;
	A_fundecList fundecs;
	A_fieldList fields;
	/* et cetera */
	}

%token <sval> ID STRING
%token <ival> INT

%token 
  COMMA COLON SEMICOLON LPAREN RPAREN LBRACK RBRACK 
  LBRACE RBRACE DOT 
  PLUS MINUS TIMES DIVIDE EQ NEQ LT LE GT GE
  AND OR ASSIGN
  ARRAY IF THEN ELSE WHILE FOR TO DO LET IN END OF 
  BREAK NIL
  FUNCTION VAR TYPE 

%right ASSIGN
%left OR
%left AND
%nonassoc EQ NEQ LT LE GT GE
%left  PLUS MINUS
%left  TIMES DIVIDE 


%type <exp> exp program
/* et cetera */
%type <exps> explist arglist
%type <dec> dec vardec 
%type <decs> decs
%type <efields>idlist
%type <var>lvalue
%type <ty> ty
%type <name> tydec
%type <names> tydecs
%type <fundec> fundec
%type <fundecs> fundecs
%type <fields> tyfields tyfields2

%start program

%%

program:   exp    {absyn_root=$1;}


exp:lvalue { $$=A_VarExp(0,$1); }
   |NIL    {position=EM_tokPos;$$=A_NilExp(position); }
   /*|exp SEMICOLON exp {printf("here in\n");}*/ //needed?
   |LPAREN explist RPAREN { position=EM_tokPos;$$=A_SeqExp(position,$2); }
   |LPAREN         RPAREN {position=EM_tokPos;$$=A_SeqExp(position,NULL);}
   |INT {position=EM_tokPos;$$=A_IntExp(position,$1);}
   |STRING {position=EM_tokPos;$$=A_StringExp(position,$1);}
   |MINUS exp  {position=EM_tokPos;$$=A_OpExp(position,A_minusOp,A_IntExp(position,0),$2);}
   |ID LPAREN RPAREN {position=EM_tokPos;$$=A_CallExp(position,S_Symbol($1),NULL);}
   |ID LPAREN arglist RPAREN {position=EM_tokPos;$$=A_CallExp(position,S_Symbol($1),$3);}
   |exp PLUS exp {position=EM_tokPos;$$=A_OpExp(position,A_plusOp,$1,$3);}
   |exp MINUS exp {position=EM_tokPos;$$=A_OpExp(position,A_minusOp,$1,$3);}
   |exp TIMES exp {position=EM_tokPos;$$=A_OpExp(position,A_timesOp,$1,$3);}
   |exp DIVIDE exp {position=EM_tokPos;$$=A_OpExp(position,A_divideOp,$1,$3);}
   |exp EQ exp {position=EM_tokPos;$$=A_OpExp(position,A_eqOp,$1,$3);}
   |exp NEQ exp {position=EM_tokPos;$$=A_OpExp(position,A_neqOp,$1,$3);}
   |exp LT exp {position=EM_tokPos;$$=A_OpExp(position,A_ltOp,$1,$3);}
   |exp LE exp {position=EM_tokPos;$$=A_OpExp(position,A_leOp,$1,$3);}
   |exp GT exp {position=EM_tokPos;$$=A_OpExp(position,A_gtOp,$1,$3);}
   |exp GE exp {position=EM_tokPos;$$=A_OpExp(position,A_geOp,$1,$3);}
   |exp AND exp {position=EM_tokPos;$$=A_IfExp(position,$1,$3,A_IntExp(position,0));}
   |exp OR exp {position=EM_tokPos;$$=A_IfExp(position,$1,A_IntExp(position,1),$3);}
   |ID LBRACE idlist RBRACE {position=EM_tokPos;$$=A_RecordExp(position,S_Symbol($1),$3);}
   |ID LBRACE RBRACE {position=EM_tokPos;$$=A_RecordExp(position,S_Symbol($1),NULL);}
   |ID LBRACK exp RBRACK OF exp {position=EM_tokPos;$$=A_ArrayExp(position,S_Symbol($1),$3,$6);}
   |lvalue ASSIGN exp {position=EM_tokPos;$$=A_AssignExp(position,$1,$3);;}
   |IF exp THEN exp ELSE exp {position=EM_tokPos;$$=A_IfExp(position,$2,$4,$6);}
   |IF exp THEN exp {position=EM_tokPos;$$=A_IfExp(position,$2,$4,NULL);}
   |WHILE exp DO exp {position=EM_tokPos;$$=A_WhileExp(position,$2,$4);}
   |FOR ID ASSIGN exp TO exp DO exp {position=EM_tokPos;$$=A_ForExp(position,S_Symbol($2),$4,$6,$8);}
   |BREAK {position=EM_tokPos;$$=A_BreakExp(position);}
   |LET decs IN END {position=EM_tokPos;$$=A_LetExp(position,$2,NULL);}
   |LET decs IN explist END {position=EM_tokPos;$$=A_LetExp(position,$2,A_SeqExp(position,$4));}//explist?seqexp?
   ;

lvalue: ID {position=EM_tokPos;$$=A_SimpleVar(position,S_Symbol($1));}
      | lvalue DOT ID {position=EM_tokPos;$$=A_FieldVar(position,$1,S_Symbol($3));}
      | ID LBRACK exp RBRACK {position=EM_tokPos;$$=A_SubscriptVar(position,A_SimpleVar(position,S_Symbol($1)),$3);}
      | lvalue LBRACK exp RBRACK {position=EM_tokPos;$$=A_SubscriptVar(position,$1,$3);}
      ;

explist: exp {position=EM_tokPos;$$=A_ExpList($1,NULL);}
       | exp SEMICOLON explist {position=EM_tokPos;$$=A_ExpList($1,$3);}
       ;

arglist:exp {$$=A_ExpList($1,NULL);}
       |exp COMMA arglist {$$=A_ExpList($1,$3);}
       ;

idlist:ID EQ exp {$$=A_EfieldList(A_Efield(S_Symbol($1),$3),NULL);}
      |ID EQ exp COMMA idlist {$$=A_EfieldList(A_Efield(S_Symbol($1),$3),$5);}
      ;

decs:dec {$$=A_DecList($1,NULL);}
    |dec decs {$$=A_DecList($1,$2);}
    ;

dec:tydecs {position=EM_tokPos;$$=A_TypeDec(position,$1);}
   |vardec {$$=$1;}
   |fundecs {position=EM_tokPos;$$=A_FunctionDec(position,$1);}
   ;
tydecs:tydec {$$=A_NametyList($1,NULL);}
      |tydec tydecs {$$=A_NametyList($1,$2);}
      ;

tydec:TYPE ID EQ ty {$$=A_Namety(S_Symbol($2),$4);}
     ;

ty:ID {position=EM_tokPos;$$=A_NameTy(position,S_Symbol($1));}
  |LBRACE tyfields RBRACE {position=EM_tokPos;$$=A_RecordTy(position,$2);}
  |ARRAY OF ID {position=EM_tokPos;$$=A_ArrayTy(position,S_Symbol($3));}
  ;

tyfields:  {$$=NULL;}
        |tyfields2 {$$=$1;}
        ;

tyfields2:ID COLON ID {position=EM_tokPos;$$=A_FieldList(A_Field(position,S_Symbol($1),S_Symbol($3)),NULL);}//position here wrong
         |ID COLON ID COMMA tyfields2 {position=EM_tokPos;$$=A_FieldList(A_Field(position,S_Symbol($1),S_Symbol($3)),$5);}//position here wrong
         ;

vardec:VAR ID ASSIGN exp {position=EM_tokPos;$$=A_VarDec(position,S_Symbol($2),NULL,$4);}
      |VAR ID COLON ID ASSIGN exp {position=EM_tokPos;$$=A_VarDec(position,S_Symbol($2),S_Symbol($4),$6);}
      ;

fundecs:fundec {$$=A_FundecList($1,NULL);}
	|fundec fundecs {$$=A_FundecList($1,$2);}
	;


fundec:FUNCTION ID LPAREN tyfields RPAREN EQ exp {position=EM_tokPos;$$=A_Fundec(position,S_Symbol($2),$4,NULL,$7);}
      |FUNCTION ID LPAREN tyfields RPAREN COLON ID EQ exp {position=EM_tokPos;$$=A_Fundec(position,S_Symbol($2),$4,S_Symbol($7),$9);}
      ;



	
