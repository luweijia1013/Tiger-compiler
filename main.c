/*
 * main.c
 */

#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "types.h"
#include "absyn.h"
#include "errormsg.h"
#include "temp.h" /* needed by translate.h */
#include "tree.h" /* needed by frame.h */
#include "assem.h"
#include "frame.h" /* needed by translate.h and printfrags prototype */
#include "translate.h"
#include "env.h"
#include "semant.h" /* function prototype for transProg */
#include "canon.h"
#include "prabsyn.h"
#include "printtree.h"
#include "escape.h" /* needed by escape analysis */
#include "parse.h"
#include "codegen.h"
#include "regalloc.h"

extern bool anyErrors;

/* print the assembly language instructions to filename.s */
static void doProc(FILE *out, F_frame frame, T_stm body)
{
 AS_proc proc;
 T_stmList stmList;
 AS_instrList iList;

 F_tempMap = Temp_empty();

 stmList = C_linearize(body);
 stmList = C_traceSchedule(C_basicBlocks(stmList));
 //printStmList(out, stmList);
 iList  = F_codegen(frame, stmList); /* 9 */

 struct RA_result ra = RA_regAlloc(frame, iList);  /* 10, 11 */

 fprintf(out, ".text\n.globl %s\n.type %s, @function\n%s:\n", Temp_labelstring(frame->name), Temp_labelstring(frame->name), Temp_labelstring(frame->name));
 fprintf(out, "pushl %%ebp\nmovl %%esp,%%ebp\n");
 if (frame->local_count != 0) {
   fprintf(out, "subl $%d,%%esp\n", 4 * frame->local_count);
 }
 //lab 6b
 AS_printInstrList(out, iList,
   Temp_layerMap(ra.coloring,Temp_layerMap(F_tempMap, Temp_name())));
 fprintf(out, "leave\nret\n");
}

int main(int argc, string *argv)
{
 A_exp absyn_root;
 S_table base_env, base_tenv;
 F_fragList frags;
 char outfile[100];
 FILE *out = stdout;
 char outfile1[100];
 FILE *out1 = stdout;

 if (argc==2) {
   absyn_root = parse(argv[1]);
   if (!absyn_root)
     return 1;
     
#if 0
   pr_exp(out, absyn_root, 0); /* print absyn data structure */
   fprintf(out, "\n");
#endif
  //If you have implemented escape analysis, uncomment this
   Esc_findEscape(absyn_root); /* set varDec's escape field */

   frags = SEM_transProg(absyn_root);
   if (anyErrors) return 1; /* don't continue */

   /* convert the filename */
   sprintf(outfile, "%s.s", argv[1]);
   out = fopen(outfile, "w");
   /* Chapter 8, 9, 10, 11 & 12 */
   int stime = 0;
   for (;frags;frags=frags->tail)
     if (frags->head->kind == F_procFrag) 
       doProc(out, frags->head->u.proc.frame, frags->head->u.proc.body);
   else if (frags->head->kind == F_stringFrag) {
     if (stime == 0) {
       stime++;
       fprintf(out, ".section .rodata\n");
     }
    fprintf(out, "%s:\n", Temp_labelstring(frags->head->u.stringg.label));
    char ptstr[200];
    int ind = 0;
    int i;
    for (i = 0; i < strlen(frags->head->u.stringg.str); i++) {
       if (frags->head->u.stringg.str[i] == '\n') {
         ptstr[ind] = '\\';
         ptstr[ind + 1] = 'n';
         ind = ind + 2;
       }
       else {
         ptstr[ind] = frags->head->u.stringg.str[i];
         ind++;
       }
     }
     ptstr[ind] = '\0';
     char sizebuf[4];
     *((int*)sizebuf) = strlen(frags->head->u.stringg.str);
     fprintf(out, ".string \"%c%c%c%c%s\"\n",sizebuf[0], sizebuf[1], sizebuf[2], sizebuf[3],ptstr);
   }

   fclose(out);
   return 0;
 }
 EM_error(0,"usage: tiger file.tig");
 return 1;
}
