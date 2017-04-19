#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "temp.h"
#include "errormsg.h"
#include "tree.h"
#include "assem.h"
#include "frame.h"
#include "codegen.h"
#include "table.h"

//Lab 6: your code here
static void emit(AS_instr inst);
static void munchStm(T_stm s);
static Temp_temp munchExp(T_exp e);
static Temp_tempList munchArgs(int n, T_expList t_explist);
static Temp_tempList L(Temp_temp h, Temp_tempList t);
static T_expList reversel(T_expList now);
static AS_instrList iList = NULL, last = NULL;

int time = 0;
AS_instrList F_codegen(F_frame f, T_stmList stmList) {
	AS_instrList list; T_stmList sl;
	if (time == 0) {
		for (sl = stmList; sl; sl = sl->tail)
			munchStm(sl->head);
		time++;
	}
	else {
		char *ss = checked_malloc(sizeof(char*));
		F_access acc = F_allocLocal(f, TRUE);
		F_access acc1 = F_allocLocal(f, TRUE);
		F_access acc2 = F_allocLocal(f, TRUE);
		sprintf(ss, "movl %%edi, %d(`s0)\n",acc->u.offset);
		AS_instr i = AS_Move(String(ss),NULL, L(F_EBP(), NULL));
		emit(i);
		sprintf(ss, "movl %%esi, %d(`s0)\n", acc1->u.offset);
		AS_instr ii = AS_Move(String(ss), NULL, L(F_EBP(), NULL));
		emit(ii);
		sprintf(ss, "movl %%ebx, %d(`s0)\n", acc2->u.offset);
		AS_instr iii = AS_Move(String(ss), NULL, L(F_EBP(), NULL));
		emit(iii);

		for (sl = stmList; sl; sl = sl->tail)
			munchStm(sl->head);


		sprintf(ss, "movl  %d(`s0),%%edi\n", acc->u.offset);
		AS_instr i1 = AS_Move(String(ss), NULL, L(F_EBP(), NULL));
		emit(i1);
		sprintf(ss, "movl  %d(`s0),%%esi\n", acc1->u.offset);
		AS_instr ii1 = AS_Move(String(ss), NULL, L(F_EBP(), NULL));
		emit(ii1);
		sprintf(ss, "movl  %d(`s0),%%ebx\n", acc2->u.offset);
		AS_instr iii1 = AS_Move(String(ss), NULL, L(F_EBP(), NULL));
		emit(iii1);
	}

	list = iList; iList = last = NULL;
	return list;
}


static void munchStm(T_stm s) {
	//return;  //return here to pass the Lab6A test;
	char *ss = checked_malloc(sizeof(char*));
	switch (s->kind) {
	case T_SEQ: {
		assert(0 && "should not have seq\n");
	}
	case T_LABEL: {
		sprintf(ss, "%s:\n", Temp_labelstring(s->u.LABEL));
		AS_instr i = AS_Label(String(ss), s->u.LABEL);
		emit(i);
		break;
	}
	case T_JUMP: {
		sprintf(ss, "jmp `j0\n");
		ss = String(ss);
		AS_instr i = AS_Oper(ss, NULL, NULL, AS_Targets(s->u.JUMP.jumps));//temp_labellist's yongfa
		emit(i);
		break;
	}
	case T_CJUMP: {
		Temp_temp left = munchExp(s->u.CJUMP.left);
		Temp_temp right = munchExp(s->u.CJUMP.right);
		char *ssc = checked_malloc(sizeof(char*));
		sprintf(ssc, "cmp `s1,`s0\n");
		AS_instr i_cmp = AS_Oper(String(ssc), NULL, L(left, L(right, NULL)), NULL);
		emit(i_cmp);
		string op_s = NULL;
		switch (s->u.CJUMP.op) {
		case T_eq: op_s = "je"; break;
		case T_ne: op_s = "jne"; break;
		case T_lt: op_s = "jl"; break;
		case T_gt: op_s = "jg"; break;
		case T_le: op_s = "jle"; break;
		case T_ge: op_s = "jge"; break;
		default: assert(0);
		}
		sprintf(ss, "%s `j0\n", op_s);
		AS_instr i_jmp = AS_Oper(String(ss), NULL, NULL, AS_Targets(Temp_LabelList(s->u.CJUMP.true, NULL))); // origin no String()
		emit(i_jmp);
		break;
	}
	case T_MOVE: {
		T_exp ds = s->u.MOVE.dst;
		T_exp sr = s->u.MOVE.src;
		switch (ds->kind) {
		case T_MEM: {
			if (ds->u.MEM->kind == T_BINOP&&ds->u.MEM->u.BINOP.op == T_plus) {
				if (ds->u.MEM->u.BINOP.right->kind == T_CONST) {
					sprintf(ss, "movl `s0,%d(`s1)\n", ds->u.MEM->u.BINOP.right->u.CONST);
					AS_instr i = AS_Move(String(ss), NULL, L(munchExp(sr), L(munchExp(ds->u.MEM->u.BINOP.left), NULL)));
					emit(i);
				}
				else if (ds->u.MEM->u.BINOP.left->kind == T_CONST) {
					sprintf(ss, "movl `s0,%d(`s1)\n", ds->u.MEM->u.BINOP.left->u.CONST);
					AS_instr i = AS_Move(String(ss), NULL, L(munchExp(sr), L(munchExp(ds->u.MEM->u.BINOP.right), NULL)));
					emit(i);
				}
				else {
					sprintf(ss, "movl `s0,(`d0)\n");
					AS_instr i = AS_Move(String(ss), L(munchExp(ds->u.MEM), NULL), L(munchExp(sr), NULL));
					emit(i);
				}
			}
			else if (ds->u.MEM->kind == T_CONST) {
				sprintf(ss, "movl `s0,($%d)\n", ds->u.MEM->u.CONST);
				AS_instr i = AS_Move(String(ss), NULL, L(munchExp(sr), NULL));
				emit(i);
			}
			//if reach here dst must be (dst->u.MEM->kind==T_TEMP)
			else if (sr->kind == T_MEM) {
				sprintf(ss, "movl (`s0),(`s1)\n");
				AS_instr i = AS_Move(String(ss), NULL, L(munchExp(sr->u.MEM), L(munchExp(ds->u.MEM), NULL)));
				emit(i);
			}
			//if reach here src must be reg(src can be reg or mem)
			else {
				sprintf(ss, "movl `s0,(`s1)\n");
				AS_instr i = AS_Move(String(ss), NULL, L(munchExp(sr), L(munchExp(ds->u.MEM), NULL)));
				emit(i);
			}
			break;
		}
		case T_TEMP: {
			sprintf(ss, "movl `s0,`d0\n");
			AS_instr i = AS_Move(String(ss), L(ds->u.TEMP, NULL), L(munchExp(sr), NULL));
			emit(i);
			break;
		}
		default:assert(0);
		}
		break;
	}
	case T_EXP: {
		munchExp(s->u.EXP);
		break;
	}
	}
}

int nopass = 0;
static Temp_temp munchExp(T_exp e) {
	Temp_temp r = Temp_newtemp();
	int a = sizeof(char*);
	char* ss = checked_malloc(sizeof(char*));
	switch (e->kind) {
	case T_BINOP: {
		T_exp left = e->u.BINOP.left;
		T_exp right = e->u.BINOP.right;
		string op;
		switch (e->u.BINOP.op) {
		case T_plus: {op = "addl"; break; }
		case T_minus: {op = "subl"; break; }
		case T_mul: {op = "imull"; break; }
		case T_div: {op = "idiv"; break; }
		}
		if (op == "idiv") {
			Temp_temp r1, r2, r3, tm;
			int con;
			if (e->u.BINOP.right->kind == T_CONST) {
				con = e->u.BINOP.right->u.CONST;
				r1 = Temp_newtemp();
				r2 = munchExp(e->u.BINOP.left);
				tm = Temp_newtemp();
				sprintf(ss, "mov $%d, `d0\n", con);
				emit(AS_Oper(String(ss), Temp_TempList(tm, NULL), NULL, NULL));
				sprintf(ss, "mov $0, `d0\n");
				emit(AS_Oper(String(ss), Temp_TempList(F_EDX(), NULL),
					NULL, NULL));
				sprintf(ss, "mov `s0, `d0\n");
				emit(AS_Move(String(ss), Temp_TempList(F_EAX(), NULL),
					Temp_TempList(r2, NULL)));
				sprintf(ss, "idiv `s2\n");
				emit(AS_Oper(String(ss), Temp_TempList(F_EAX(), Temp_TempList(F_EDX(), NULL)),
					Temp_TempList(F_EAX(), Temp_TempList(F_EDX(), Temp_TempList(tm, NULL))), NULL));
				sprintf(ss, "mov `s0, `d0\n");
				emit(AS_Move(String(ss), Temp_TempList(r1, NULL),
					Temp_TempList(F_EAX(), NULL)));
				return r1;
			}
			else {
				r1 = Temp_newtemp();
				r2 = munchExp(e->u.BINOP.left);
				r3 = munchExp(e->u.BINOP.right);
				sprintf(ss, "mov $0, `d0\n");
				emit(AS_Oper(String(ss), Temp_TempList(F_EDX(), NULL),
					NULL, NULL));
				sprintf(ss, "mov `s0, `d0\n");
				emit(AS_Move(String(ss), Temp_TempList(F_EAX(), NULL),
					Temp_TempList(r2, NULL)));
				sprintf(ss, "idiv `s2\n");
				emit(AS_Oper(String(ss), Temp_TempList(F_EAX(), Temp_TempList(F_EDX(), NULL)),
					Temp_TempList(F_EAX(), Temp_TempList(F_EDX(), Temp_TempList(r3, NULL))),
					NULL));
				sprintf(ss, "mov `s0, `d0\n");
				emit(AS_Move(String(ss), Temp_TempList(r1, NULL),
					Temp_TempList(F_EAX(), NULL)));
				return r1;
			}
		}
		if (right->kind == T_CONST) {
			sprintf(ss, "movl `s0,`d0\n");
			emit(AS_Move(String(ss), L(r, NULL), L(munchExp(left), NULL)));
			sprintf(ss, "%s $%d,`d0\n", op,right->u.CONST);
			emit(AS_Oper(String(ss), L(r, NULL), L(r, NULL), NULL));
		}
		else if (left->kind == T_CONST) {
			sprintf(ss, "movl $%d,`d0\n",left->u.CONST);
			emit(AS_Move(String(ss), L(r, NULL), NULL));
			sprintf(ss, "%s `s0,`d0\n", op);
			emit(AS_Oper(String(ss), L(r, NULL), L(munchExp(e->u.BINOP.right), L(r, NULL)), NULL));
		}
		else {
			sprintf(ss, "movl `s0,`d0\n");
			AS_instr ib = AS_Move(String(ss), L(r, NULL), L(munchExp(left), NULL));
			emit(ib);
			sprintf(ss, "%s `s1,`d0\n", op);//here s0 or s1?
			AS_instr i = AS_Oper(String(ss), L(r, NULL), L(r, L(munchExp(right), NULL)), NULL); // is this right?
			emit(i);
		}
		return r;

		
	}
	case T_MEM: {
		if (e->u.MEM->kind == T_BINOP&&e->u.MEM->u.BINOP.op == T_plus) {
			if (e->u.MEM->u.BINOP.right->kind == T_CONST) {
				sprintf(ss, "movl %d(`s0),`d0\n", e->u.MEM->u.BINOP.right->u.CONST);
				emit(AS_Move(String(ss), L(r, NULL), L(munchExp(e->u.MEM->u.BINOP.left), NULL)));
			}
			else if (e->u.MEM->u.BINOP.left->kind == T_CONST) {
				sprintf(ss, "movl %d(`s0),`d0\n", e->u.MEM->u.BINOP.left->u.CONST);
				emit(AS_Move(String(ss), L(r, NULL), L(munchExp(e->u.MEM->u.BINOP.right), NULL)));
			}
			else {
				T_exp e1 = e->u.MEM->u.BINOP.left;
				Temp_temp temp_temp_t1 = munchExp(e1);
				T_exp e2 = e->u.MEM->u.BINOP.right;
				sprintf(ss, "addl `s0,`d0\n");
				emit(AS_Oper(String(ss), L(temp_temp_t1, NULL), L(munchExp(e2), L(temp_temp_t1, NULL)), NULL));
				char *ss2 = checked_malloc(sizeof(char*));
				sprintf(ss2, "movl (`s0),`d0\n");
				emit(AS_Move(String(ss2), L(r, NULL), L(temp_temp_t1, NULL)));
				
			}
		}
		else if (e->u.MEM->kind == T_CONST) {
			sprintf(ss, "movl (%d),`d0\n", e->u.MEM->u.CONST);
			AS_instr i = AS_Move(String(ss), L(r, NULL), NULL);
			emit(i);
			assert(0 && "i think it's impossible");
		}
		else {
			sprintf(ss, "movl (`s0),`d0\n");
			AS_instr i = AS_Move(String(ss), L(r, NULL), L(munchExp(e->u.MEM), NULL));
			emit(i);
		}
		break;
	}
	case T_TEMP: {
		r = e->u.TEMP;
		break;
	}
	case T_ESEQ: {
		assert(0);
	}
	case T_NAME: {
		Temp_enter(F_tempMap, r, Temp_labelstring(e->u.NAME));
		if (Temp_labelstring(e->u.NAME)[0] == '.') {
			sprintf(ss, "movl $%s,`d0\n", Temp_labelstring(e->u.NAME));
			emit(AS_Move(String(ss), L(r, NULL), NULL));
		}
		break;
	}
	case T_CONST: {
		sprintf(ss, "movl $%d,`d0\n", e->u.CONST);
		AS_instr i = AS_Move(String(ss), L(r, NULL), NULL);
		emit(i);
		break;
	}
	case T_CALL: {
		r = munchExp(e->u.CALL.fun);
		assert(e->u.CALL.fun->kind == T_NAME&&"call fun's type should be a T_NAME(temp_label)");
		Temp_label lb = e->u.CALL.fun->u.NAME;
		sprintf(ss, "call %s\n", Temp_labelstring(lb));
		char pp = Temp_labelstring(lb)[0];
		if (pp != 'F'&& pp != 'm' && pp != 'i'&&pp!='s') {
			//no passing staticlink  (printi print has link but no need)(initialrecord/array,malloc,stringequal hasn't link)
			nopass = 1;
		}
		else {
			nopass = 0;
		}
		AS_instr i = AS_Oper(String(ss), F_calldefs(), munchArgs(0, reversel(e->u.CALL.args)), NULL);//see github munchargs
		emit(i);
		return F_EAX();
		break;
	}
	default:
		assert(0);
	}
	return r;
}

//the aruguments' order is wrong,use this static function to reverse the order.
static T_expList reversel(T_expList now) {
	T_expList reverse = NULL;
	T_exp first = now->head;
	now = now->tail;
	while (now != NULL) {
		reverse = T_ExpList(now->head, reverse);
		now = now->tail;
	}
	reverse = T_ExpList(first, reverse);
	return reverse;
}

static Temp_tempList munchArgs(int n, T_expList t_expList) {
	if (t_expList == NULL)return NULL;
	if (nopass == 1 && n == 0) {
		return munchArgs(n + 1, t_expList->tail);
	}
	char* ss = checked_malloc(sizeof(char*));
	Temp_temp r = munchExp(t_expList->head);
	Temp_tempList rest = munchArgs(n + 1, t_expList->tail);
	sprintf(ss, "pushl `s0\n");
	emit(AS_Oper(String(ss), NULL, L(r, NULL), NULL));
	return L(r, rest);
}

static void emit(AS_instr inst) {
	if (last != NULL)
		last = last->tail = AS_InstrList(inst, NULL);
	else last = iList = AS_InstrList(inst, NULL);
}

Temp_tempList L(Temp_temp h, Temp_tempList t) {
	return Temp_TempList(h, t);
}




