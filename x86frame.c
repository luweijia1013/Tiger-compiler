#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "table.h"
#include "tree.h"
#include "assem.h"
#include "frame.h"

/*Lab5: Your implementation here.*/
const int F_WORD_SIZE =4;
Temp_tempList L(Temp_temp h, Temp_tempList t);

Temp_temp F_EAX() {
	static Temp_temp eax;
	if (eax == NULL) {
		eax = Temp_newtemp();
	}
	return eax;
}
Temp_temp F_EBX() {
	static Temp_temp ebx;
	if (ebx == NULL) {
		ebx = Temp_newtemp();
	}
	return ebx;
}
Temp_temp F_ECX() {
	static Temp_temp ecx;
	if (ecx == NULL) {
		ecx = Temp_newtemp();
	}
	return ecx;
}
Temp_temp F_EDX() {
	static Temp_temp edx;
	if (edx == NULL) {
		edx = Temp_newtemp();
	}
	return edx;
}
Temp_temp F_EDI() {
	static Temp_temp edi;
	if (edi == NULL) {
		edi = Temp_newtemp();
	}
	return edi;
}
Temp_temp F_ESI() {
	static Temp_temp esi;
	if (esi == NULL) {
		esi = Temp_newtemp();
	}
	return esi;
}
Temp_temp F_EBP() {
	static Temp_temp ebp;
	if (ebp == NULL) {
		ebp = Temp_newtemp();
	}
	return ebp;
}
Temp_temp F_ESP() {
	static Temp_temp esp;
	if (esp == NULL) {
		esp = Temp_newtemp();
	}
	return esp;
}



F_frame F_newFrame(Temp_label name,U_boolList formals){
	F_frame fm= checked_malloc(sizeof(*fm));
	fm->name=name;
	U_boolList fs=formals;
	F_accessList result=NULL;
	F_accessList order=NULL;
	int num=1;		
	while(fs){
		F_access f = NULL;
		if (!fs->head) {
			f = InReg(Temp_newtemp());
		}
		else {
			num += 1;
			f = InFrame(num*F_WORD_SIZE);
		}
		if(!result){
			result=F_AccessList(f,NULL);
			order =result;
		}
		else{
			order->tail=F_AccessList(f,NULL);
			order=order->tail;
		}
		fs=fs->tail;
	}
	fm->formals=result;
	fm->local_count=0;
	return fm;
}

Temp_label F_name(F_frame f){
	return f->name;
}

F_accessList F_formals(F_frame f){
	return f->formals;
}

F_access F_allocLocal(F_frame f,bool escape){// access need to be added into accesslist?accesslist's order?
	if (escape) {
		f->local_count += 1;
	}
	F_access head=NULL;
	if(escape)
		head=InFrame(-1*f->local_count*F_WORD_SIZE);
	else
		head=InReg(Temp_newtemp());
	return head;
}

F_access InFrame(int offset) {
	F_access a = checked_malloc(sizeof(*a));
	a->kind = inFrame;
	a->u.offset = offset;
	return a;
}

F_access InReg(Temp_temp reg) {
	F_access a = checked_malloc(sizeof(*a));
	a->kind = inReg;
	a->u.reg = reg;
	return a;
}

F_accessList F_AccessList(F_access head, F_accessList tail) {
	F_accessList l = checked_malloc(sizeof(*l));
	l->head = head;
	l->tail = tail;
	return l;
}


T_exp F_Exp(F_access access, T_exp framePtr){ 
	if (access->kind == inFrame) {
		T_exp e = T_Mem(T_Binop(T_plus, framePtr, T_Const(access->u.offset)));
		return e;
	} else {
		return T_Temp(access->u.reg);
	}
}

F_frag F_StringFrag(Temp_label label, string str) {
	F_frag strfrag = checked_malloc(sizeof(*strfrag));
	strfrag->kind = F_stringFrag;
	strfrag->u.stringg.label = label;
	strfrag->u.stringg.str = str;
	return strfrag;
}

F_frag F_ProcFrag(T_stm body, F_frame frame) {
	F_frag pfrag = checked_malloc(sizeof(*pfrag));
	pfrag->kind = F_procFrag;
	pfrag->u.proc.body = body;
	pfrag->u.proc.frame = frame;
	return pfrag;
}

F_fragList F_FragList(F_frag head, F_fragList tail) {
	F_fragList fl = checked_malloc(sizeof(*fl));
	fl->head = head;
	fl->tail = tail;
	return fl;
}

T_exp F_externalCall(string str, T_expList args) {
	return T_Call(T_Name(Temp_namedlabel(str)), args);
}

Temp_tempList F_registers(void) {
	
	return  L(F_EAX(), L(F_EBX(), L(F_ECX(), L(F_EDX(), L(F_EDI(), L(F_ESI(), L(F_EBP(), L(F_ESP(),  NULL))))))));
}



 static Temp_tempList callersaves() 
 {
 	/* assist-function of calldefs() */
 	return L(F_EAX(), L(F_ECX(), L(F_EDX(), NULL)));
 }

 static Temp_tempList specialregs()
 {
     static Temp_tempList spcregs = NULL;
     if (!spcregs) spcregs = L(F_ESP(), L(F_EBP(), L(F_EAX(), NULL)));
     return spcregs;
 }

 /*-short argsregs, because pass arg by stack*/

 static Temp_tempList calleesaves() 
 {   
     /* callee protect sp, fp, ebx */
     static Temp_tempList calleeregs = NULL;
     if (!calleeregs) {
         //Temp_enter(F_tempMap, F_EBX(), "ebx");
         calleeregs = L(F_ESI(), L(F_EDI(), L(F_EBX(), NULL)));
     }
     return calleeregs;
 }

 Temp_tempList F_calldefs() 
 {
 	/* some registers that may raise side-effect (caller procted, return-val-reg, return-addr-reg) */
 	static Temp_tempList protected_regs = NULL;
 	return protected_regs ? protected_regs : (protected_regs = callersaves());
 }

 Temp_tempList L(Temp_temp h, Temp_tempList t) {
	 return Temp_TempList(h, t);
 }