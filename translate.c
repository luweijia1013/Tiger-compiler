#include <stdio.h>
#include "util.h"
#include "table.h"
#include "symbol.h"
#include "absyn.h"
#include "temp.h"
#include "tree.h"
#include "printtree.h"
#include "frame.h"
#include "translate.h"

static Tr_level outlevel = NULL;
/* first part */
Tr_access Tr_Access(Tr_level l ,F_access ac){
	Tr_access a=checked_malloc(sizeof(*a));
	a->level=l;
	a->access=ac;
	return a;
}

Tr_access Tr_allocLocal(Tr_level l, bool escape) {
	Tr_access a = checked_malloc(sizeof(*a));
	a->level = l;
	a->access = F_allocLocal(l->frame, escape);
	return a;
}

Tr_accessList Tr_AccessList(Tr_access h, Tr_accessList t){
	Tr_accessList al=checked_malloc(sizeof(*al));
	al->head=h;
	al->tail=t;
	return al;
}

Tr_accessList Tr_formals(Tr_level level){
	return level->formals;
}


Tr_level Tr_outermost(void) {
	if (!outlevel) outlevel = Tr_newLevel(NULL, Temp_namedlabel(String("tigermain")), NULL);
	return outlevel;
}

Tr_level Tr_newLevel(Tr_level parent, Temp_label name, U_boolList formals){
	Tr_level l = checked_malloc(sizeof(*l));
	l->parent = parent;
	l->name = name;
	l->frame = F_newFrame(name, U_BoolList(TRUE, formals));
	Tr_accessList result=NULL;
	Tr_accessList order=NULL;
	F_accessList al=NULL;
	al=F_formals(l->frame)->tail;
	while(al){
		Tr_access a=Tr_Access(l,al->head);
		if(!result){
			result=Tr_AccessList(a,NULL);
			order=result;
		}
		else{
			order->tail=Tr_AccessList(a,NULL);
			order=order->tail;
		}
		al=al->tail;
	}
	l->formals = result;
	return l;
}

/* second part */
Tr_exp Tr_Ex(T_exp exp) {
	Tr_exp e = checked_malloc(sizeof(*e));
	e->kind = Tr_ex;
	e->u.ex = exp;
	return e;
}

Tr_exp Tr_Nx(T_stm stm) {
	Tr_exp e = checked_malloc(sizeof(*e));
	e->kind = Tr_nx;
	e->u.nx = stm;
	return e;
}

Tr_exp Tr_Cx(patchList trues, patchList falses, T_stm stm) {
	Tr_exp e = checked_malloc(sizeof(*e));
	e->kind = Tr_cx;
	e->u.cx.trues  = trues;
	e->u.cx.falses = falses;
	e->u.cx.stm    = stm;
	return e;
}

T_exp unEx(Tr_exp e){
	switch (e->kind) {
	case Tr_ex:
		return e->u.ex;
	case Tr_nx:
		return T_Eseq(e->u.nx, T_Const(0));
	case Tr_cx: {
		Temp_temp r = Temp_newtemp(); /* temp for save exp-val */
		Temp_label t = Temp_newlabel(), f = Temp_newlabel();
		doPatch(e->u.cx.trues, t);
		doPatch(e->u.cx.falses, f);
		return T_Eseq(T_Move(T_Temp(r), T_Const(1)),
				      T_Eseq(e->u.cx.stm,
						     T_Eseq(T_Label(f),
								    T_Eseq(T_Move(T_Temp(r), T_Const(0)),
										   T_Eseq(T_Label(t), T_Temp(r))))));
		}
	}
	assert(0);
}

T_stm unNx(Tr_exp e){
	switch (e->kind) {
	case Tr_ex:
		return T_Exp(e->u.ex);
	case Tr_nx:
		return e->u.nx;
	case Tr_cx: {
		Temp_temp r = Temp_newtemp(); /* temp for save exp-val */
		Temp_label t = Temp_newlabel(), f = Temp_newlabel(); 
		doPatch(e->u.cx.trues, t);
		doPatch(e->u.cx.falses, f);
		return T_Exp(T_Eseq(T_Move(T_Temp(r), T_Const(1)),
				            T_Eseq(e->u.cx.stm,
						           T_Eseq(T_Label(f),
								          T_Eseq(T_Move(T_Temp(r), T_Const(0)),
										         T_Eseq(T_Label(t), T_Temp(r)))))));
		}
	}
	assert(0);
}

struct Cx unCx(Tr_exp e){
	switch(e->kind){
	case Tr_ex:{
		struct Cx cx;
		cx.stm=T_Cjump(T_eq,e->u.ex,T_Const(0),NULL,NULL);
		cx.trues = PatchList(&(cx.stm->u.CJUMP.false), NULL);
		cx.falses = PatchList(&(cx.stm->u.CJUMP.true), NULL);
		return cx;
	}
	case Tr_nx:
		break;
	case Tr_cx:
		return e->u.cx;	
	}
	assert(0);
}

patchList PatchList(Temp_label * h, patchList t) {
	patchList p = checked_malloc(sizeof(*p));
	p->head = h;
	p->tail = t;
	return p;
}

void doPatch(patchList t, Temp_label label) {
	while(t!=NULL){
    *(t->head) = label;
    t = t->tail;
  }
}

patchList joinPatch(patchList first, patchList second) {
	if (!first) return second;
	while(first->tail!=NULL){
      first = first->tail;
    }
	first->tail = second;
	return first;
}

Tr_expList Tr_ExpList(Tr_exp h, Tr_expList t) {
    Tr_expList l = checked_malloc(sizeof(*l));
	l->head = h;
	l->tail = t;
	return l;
}

void Tr_expList_prepend(Tr_exp h, Tr_expList * l) {
	Tr_expList newhead = Tr_ExpList(h, NULL);
	newhead->tail = *l;
	*l = newhead;
	/*if (*l = NULL) {
		*l = Tr_ExpList(h, NULL);
	}
	else {
		Tr_expList head = *l;
		for (; head; head = head->tail) {}
		head = Tr_ExpList(h, NULL);
	}*/
}


/* third part(specific stm or exp..) */
Tr_exp Tr_arithExp(A_oper op, Tr_exp left, Tr_exp right){
	T_binOp oper;
	if(op==A_plusOp) oper=T_plus;
	else if(op==A_minusOp) oper=T_minus;
	else if(op==A_timesOp) oper=T_mul;
	else if(op==A_divideOp) oper=T_div;
	else assert(0);
	return Tr_Ex(T_Binop(oper,unEx(left),unEx(right)));
}

Tr_exp Tr_simpleVar(Tr_access acc, Tr_level level) {
    Tr_level defined = acc->level;
    Tr_level used = level;
    T_exp fp = T_Temp(F_EBP());
    while(used != defined) {
        //fp = T_Mem(fp);
		fp = T_Mem(T_Binop(T_plus, fp, T_Const(8)));
        used = used->parent;
    }

    T_exp var = F_Exp(acc->access, fp);
    return Tr_Ex(var);
}

Tr_exp Tr_fieldVar(Tr_exp base, int offs) {
	return Tr_Ex(T_Mem(T_Binop(T_plus, unEx(base), T_Const(offs * F_WORD_SIZE))));
}

Tr_exp Tr_subscriptVar(Tr_exp base,Tr_exp ind){
	T_exp offset;
	offset=T_Binop(T_mul,unEx(ind),T_Const(F_WORD_SIZE));
	return Tr_Ex(T_Mem(T_Binop(T_plus,unEx(base),offset)));
}

F_fragList stringFragList = NULL;
Tr_exp Tr_stringExp(string sg){
	Temp_label sl = Temp_newstlabel();
	F_frag fragment = F_StringFrag(sl, sg);
	stringFragList = F_FragList(fragment, stringFragList);
	return Tr_Ex(T_Name(sl));
}

Tr_exp Tr_intExp(int num){
	return Tr_Ex(T_Const(num));
}

Tr_exp Tr_noExp(){
	return Tr_Ex(T_Const(0));//or return NULL?
}
//why
Tr_exp Tr_callExp(Temp_label fun, Tr_level called, Tr_level defined, Tr_expList args) {
    T_expList list = NULL, exps = NULL;

    while(args) {
		T_expList now = T_ExpList(unEx(args->head),NULL);
        if(list == NULL) {
            list = now;
			exps = list;
        } else {
			exps->tail = now;
			exps = exps->tail;
        }
        args = args->tail;        
    }
	list=T_ExpList(unEx(Tr_StaticLink(called, defined)),list);
    return Tr_Ex(T_Call(T_Name(fun),list));
}
//why
Temp_temp nilTemp = NULL;
Tr_exp Tr_nilExp(){
	return  Tr_Ex(T_Const(0));
	if (!nilTemp) {
		nilTemp = Temp_newtemp(); 
		T_stm alloc = T_Move(T_Temp(nilTemp),
				             F_externalCall(String("allocRecord"), T_ExpList(T_Const(0), NULL)));
		return Tr_Ex(T_Eseq(alloc, T_Temp(nilTemp)));
	}
	return Tr_Ex(T_Temp(nilTemp));
}
//why?
Tr_exp Tr_recordExp(int n, Tr_expList l){
	Temp_temp r = Temp_newtemp();
	T_stm alloc = T_Move(T_Temp(r),
			             F_externalCall(String("malloc"), T_ExpList(T_Const(n * F_WORD_SIZE), NULL)));

	int i = n - 1;
	T_stm seq = T_Move(T_Mem(T_Binop(T_plus, T_Temp(r), T_Const(i-- * F_WORD_SIZE))), 
						     unEx(l->head));

	for (l = l->tail; l; l = l->tail, i--) {
		seq = T_Seq(T_Move(T_Mem(T_Binop(T_plus, T_Temp(r), T_Const(i * F_WORD_SIZE))), 
						   unEx(l->head)),
				    seq);
	}
	return Tr_Ex(T_Eseq(T_Seq(alloc, seq), T_Temp(r)));
}
//why
Tr_exp Tr_arrayExp(Tr_exp size, Tr_exp init){
	return Tr_Ex(F_externalCall(String("initArray"), 
				                T_ExpList(unEx(size), T_ExpList(unEx(init), NULL))));
}

Tr_exp Tr_seqExp(Tr_expList l){
	if(!l||!l->head) return Tr_noExp();
	T_exp first = unEx(l->head); /* resl cant be NULL */
	if(!first) return Tr_noExp();
	for (l = l->tail; l; l = l->tail) {
		if(l->head==NULL){break;}
		first = T_Eseq(T_Exp(unEx(l->head)), first);
	}
	return Tr_Ex(first);
}

Tr_exp Tr_doneExp(){
	return Tr_Ex(T_Name(Temp_newlabel()));
}

Tr_exp Tr_whileExp(Tr_exp test, Tr_exp body, Tr_exp done){
	Temp_label testl = Temp_newlabel(), bodyl = Temp_newlabel();
	return Tr_Ex(T_Eseq(T_Jump(T_Name(testl), Temp_LabelList(testl, NULL)), 
				        T_Eseq(T_Label(bodyl),
							   T_Eseq(unNx(body),
								      T_Eseq(T_Label(testl),
								             T_Eseq(T_Cjump(T_eq, unEx(test), T_Const(0), unEx(done)->u.NAME, bodyl),
										            T_Eseq(T_Label(unEx(done)->u.NAME), T_Const(0))))))));
}

Tr_exp Tr_assignExp(Tr_exp var, Tr_exp init){
	return Tr_Nx(T_Move(unEx(var), unEx(init)));
}

//not be used because for has changed into while
Tr_exp Tr_forExp(Tr_exp lo, Tr_exp hi, Tr_exp body, Temp_label ldone) {
      Temp_temp  temp = Temp_newtemp();
    Temp_label ltest = Temp_newlabel();
    Temp_label lbody = Temp_newlabel();
    //Temp_label ldone = Temp_newlabel();

    T_exp loexp = unEx(lo);
    T_exp hiexp = unEx(hi);
    T_stm bodystm = unNx(body); // produce no value

    return Tr_Nx(T_Seq(T_Move(T_Temp(temp),loexp),
                    T_Seq(T_Label(ltest),
                        T_Seq(T_Cjump(T_lt,T_Temp(temp),hiexp,lbody,ldone),
                            T_Seq(T_Label(lbody),
                                T_Seq(bodystm,
                                    T_Seq(T_Jump(T_Name(ltest),Temp_LabelList(ltest,NULL)),T_Label(ldone))))))));
}

Tr_exp Tr_breakExp(Tr_exp bk){
	return Tr_Nx(T_Jump(T_Name(unEx(bk)->u.NAME), Temp_LabelList(unEx(bk)->u.NAME, NULL)));
}

Tr_exp Tr_eqExp(A_oper op, Tr_exp left, Tr_exp right){
	T_relOp opp;
	if (op == A_eqOp) opp = T_eq; 
	else opp = T_ne;
	T_stm cond = T_Cjump(opp, unEx(left), unEx(right), NULL, NULL);
	patchList trues = PatchList(&cond->u.CJUMP.true, NULL);
	patchList falses = PatchList(&cond->u.CJUMP.false, NULL);
	return Tr_Cx(trues, falses, cond);
}

Tr_exp Tr_eqStringExp(A_oper op, Tr_exp left, Tr_exp right){
	T_exp resl = F_externalCall(String("stringEqual"), T_ExpList(unEx(left), T_ExpList(unEx(right), NULL)));
	if (op == A_eqOp) return Tr_Ex(resl);
	else {
		T_exp e = (resl->kind == T_CONST && resl->u.CONST != 0) ? T_Const(0): T_Const(1);
		return Tr_Ex(e);
	} 
}

Tr_exp Tr_relExp(A_oper op, Tr_exp left, Tr_exp right){
	T_relOp oper;
	switch(op) {
		case A_ltOp: oper = T_lt; break;
		case A_leOp: oper = T_le; break;
		case A_gtOp: oper = T_gt; break;
		case A_geOp: oper = T_ge; break;
        assert(0);
	}
	T_stm cond = T_Cjump(oper, unEx(left), unEx(right), NULL, NULL);
	patchList trues = PatchList(&cond->u.CJUMP.true, NULL);
	patchList falses = PatchList(&cond->u.CJUMP.false, NULL);
	return Tr_Cx(trues, falses, cond);
}

Tr_exp Tr_ifExp(Tr_exp test,Tr_exp thenn,Tr_exp elsee){

  if(test==NULL||thenn==NULL)
    return NULL;
  Temp_label t_label = Temp_newlabel();
  Temp_label f_label = Temp_newlabel();
  struct Cx cx = unCx(test);
  doPatch(cx.trues,t_label);
  doPatch(cx.falses,f_label);
  if(elsee==NULL){
    //doPatch(cx.trues,t_label);
    //doPatch(cx.falses,t_label);
    return Tr_Nx(T_Seq(cx.stm,
                       T_Seq(T_Label(t_label),
                              T_Seq(unNx(thenn),
                                    T_Label(f_label)))));

  }else{
    Temp_label r_label = Temp_newlabel();// place that then and else both jump to.
    Temp_temp temp_temp = Temp_newtemp();// the value  then or else gives.
    T_stm jump_stm = T_Jump(T_Name(r_label),Temp_LabelList(r_label,NULL));
    return Tr_Ex(T_Eseq(cx.stm,
                          T_Eseq(T_Label(t_label),
                                 T_Eseq(T_Move(T_Temp(temp_temp),unEx(thenn)),
                                        T_Eseq(jump_stm,
                                               T_Eseq(T_Label(f_label),
                                                      T_Eseq(T_Move(T_Temp(temp_temp),unEx(elsee)),
                                                             T_Eseq(T_Label(r_label),T_Temp(temp_temp)))))))));
      
  }
  assert(0);
}



F_fragList fragList = NULL;
void Frag_Initial(Tr_exp exp){
	F_frag init=F_ProcFrag(unNx(exp),Tr_outermost()->frame);//how to get T_stm?
	fragList=F_FragList(init,fragList);//add one level of outmost
}

void Tr_procEntryExit(Tr_level level, Tr_exp body, Tr_accessList formals) {
	F_frag procfrag = F_ProcFrag(unNx(body), level->frame);
	fragList = F_FragList(procfrag, fragList);
}

void Tr_procEntryExit2(Tr_level level, Tr_exp body, Tr_accessList formals) {
	T_stm t_stm = T_Move(T_Temp(F_EAX()), unEx(body));
	F_frag f_frag = F_ProcFrag(t_stm, level->frame);
	fragList = F_FragList(f_frag, fragList);
}


F_fragList Tr_getResult(){
	F_fragList cur = NULL, prev = NULL;
	for (cur = stringFragList; cur; cur = cur->tail)
		prev = cur;
	if (prev){
        prev->tail = fragList;
        return stringFragList;
    } 
	else return fragList;
}


Tr_exp Tr_StaticLink(Tr_level now, Tr_level def) {
	T_exp addr = T_Temp(F_EBP());// frame-point 
	while(now && (now != def->parent)) { // until find the level which def the function 
		F_access sl = F_formals(now->frame)->head;
		addr = F_Exp(sl, addr);
		now = now->parent;
	}
	return Tr_Ex(addr);
}


