#include <stdio.h>
#include "util.h"
#include "errormsg.h"
#include "symbol.h"
#include "absyn.h"
#include "types.h"
#include "temp.h"
#include "tree.h"
#include "frame.h"
#include "translate.h"
#include "env.h"
#include "semant.h"
 
struct expty {Tr_exp exp; Ty_ty ty;};

struct expty expTy(Tr_exp e, Ty_ty t) {
	struct expty et;
	et.exp = e;
	et.ty  = t;
	return et;
}

struct expty transVar(Tr_level level,Tr_exp bk,S_table venv, S_table tenv, A_var v);
struct expty transExp(Tr_level level,Tr_exp bk,S_table venv, S_table tenv, A_exp e);
Tr_exp transDec(Tr_level level,Tr_exp bk,S_table venv,	S_table tenv, A_dec d);
Ty_ty transTy(S_table tenv, A_ty ty);
Ty_tyList makeFormalTyList(S_table tenv , A_fieldList fl);
Ty_fieldList makeFieldLists(S_table tenv, A_fieldList fl);
bool argsMatch(Tr_level level, Tr_exp bk,S_table venv,S_table tenv,A_expList el,Ty_tyList tl,A_exp fun);
bool tyMatch(Ty_ty t1,Ty_ty t2);
bool efieldsMatch(Tr_level level, Tr_exp bk,S_table venv, S_table tenv,Ty_ty ty,A_exp e);
Ty_ty actual_ty(Ty_ty ty);
U_boolList makeFormals(A_fieldList);

F_fragList SEM_transProg(A_exp exp)
{
	struct expty et;
	
	et = transExp(Tr_outermost(), NULL, E_base_venv(), E_base_tenv(), exp);
	Frag_Initial(et.exp);
	F_fragList result = Tr_getResult();
	return result;
}

struct expty transVar(Tr_level level,Tr_exp bk,S_table venv, S_table tenv, A_var v)
{
	switch(v->kind)  
	{
	
		case A_simpleVar: 
		{	
			E_enventry x = S_look(venv,v->u.simple);
			if(x && x->kind == E_varEntry){
				Tr_exp trans;
				trans=Tr_simpleVar(x->u.var.access,level);
				return expTy(trans,actual_ty(x->u.var.ty));
			}
			else
			{
				EM_error(v->pos,"undefined variable %s",S_name(v->u.simple));
				return expTy(NULL,Ty_Int()); //NULL or Tr_noExp()?Ty_void()?
			}
		}	
		case A_fieldVar:// reference test22.tig(lab4)
		{	
			struct expty et = transVar(level,bk,venv, tenv, v->u.field.var);
      			if (et.ty && et.ty->kind == Ty_record)
		    	{
       			 	Ty_fieldList fl = et.ty->u.record;
					int index=0;
        		 	for(; fl; fl = fl->tail,index++) 
				    {
          		 		if (fl->head->name == v->u.field.sym) 
            					return expTy(Tr_fieldVar(et.exp,index), actual_ty(fl->head->ty));
        			}
				    // not return,because field not in record type
     				EM_error(v->pos, "field %s doesn't exist",S_name(v->u.field.sym));
				    return expTy(NULL,Ty_Void());//NULL or Tr_noExp()?
      			}	
      		// not return,because kind!=Ty_record(maybe ty==null?)
     		EM_error(v->pos, "not a record type");
			return expTy(NULL,Ty_Void());//NULL or Tr_noExp()?
      	}
		case A_subscriptVar: /* a[b], et points to a, et2 points to b */
		{
     		struct expty et = transVar(level,bk,venv, tenv, v->u.subscript.var);
     		if (et.ty && et.ty->kind == Ty_array)
			{
        	  	struct expty ind = transExp(level,bk,venv, tenv, v->u.subscript.exp);
				if (ind.ty->kind == Ty_int)
				 	return expTy(Tr_subscriptVar(et.exp, ind.exp), actual_ty(et.ty->u.array));
				else 
				   	EM_error(v->pos, "int required");
			}
      			 // not return,because type not correct
      		EM_error(v->pos, "array type required");
     		return expTy(NULL, Ty_Array(NULL));///NULL or Tr_noExp()?Ty_void()?
     	}
     	default:
			assert(0);	 
	}		
}

struct expty transExp(Tr_level level, Tr_exp bk,S_table venv, S_table tenv, A_exp e)
{
	 static U_boolList loopstack = NULL;
	 
 	
	switch(e->kind)
	{
	 	case A_varExp:
      		return transVar(level,bk,venv, tenv, e->u.var);

   	    case A_nilExp:
      		return expTy(Tr_nilExp(), Ty_Nil());

   	    case A_intExp:
      		return expTy(Tr_intExp(e->u.intt), Ty_Int());

   	    case A_stringExp:
      		return expTy(Tr_stringExp(e->u.stringg), Ty_String());
   		case A_callExp:
      		{
    			E_enventry callinfo;
      			callinfo = S_look(venv,e->u.call.func);
      			if(callinfo &&callinfo->kind == E_funEntry )
      			{
					Tr_exp trans;
      				if(callinfo->u.fun.result)
						{
						Tr_expList result=NULL;
						A_expList args;
						for(args=e->u.call.args;args;args=args->tail){
							struct expty arg = transExp(level, bk, venv, tenv, args->head);
           					Tr_expList_prepend(arg.exp, &result);
						}
						trans = Tr_callExp(callinfo->u.fun.label, level, callinfo->u.fun.level, result);
      					return expTy( trans, actual_ty(callinfo->u.fun.result));
					}
      				else return expTy(trans,Ty_Void());
      			}
				return expTy(NULL, Ty_Void());
		}
 			  		
   	    case A_opExp:
   	    {
      		A_oper oper = e->u.op.oper;
      		struct expty left = transExp(level,bk,venv,tenv,e->u.op.left);
      		struct expty right = transExp(level,bk,venv,tenv,e->u.op.right);
      		switch(oper)
      		{
      			case A_plusOp:
   				case A_minusOp:
       			case A_timesOp:
    			case A_divideOp:
	      			if (left.ty->kind != Ty_int) {
	       				 EM_error(e->u.op.left->pos, "integer required");
	      			}
	      			if (right.ty->kind != Ty_int) {
	        			EM_error(e->u.op.right->pos, "integer required");
	      			}
	      			return expTy(Tr_arithExp(oper,left.exp,right.exp), Ty_Int());
	      		case A_eqOp:
				case A_neqOp:
					if (!tyMatch(left.ty,right.ty)) {
						EM_error(e->pos, "same type required");
					}
					else{
						switch(left.ty->kind){
							case Ty_int:
								return expTy(Tr_eqExp(oper, left.exp, right.exp),Ty_Int());
								break;
							case Ty_string:
								return expTy(Tr_eqStringExp(oper, left.exp, right.exp),Ty_Int());
								break;
							case Ty_array:
							case Ty_record:
								return expTy(Tr_eqExp(oper, left.exp, right.exp),Ty_Int());//should record comparison here call another compare-function instead of Tr_eqExp()?
								break;
							default:// how to handle nil?
								EM_error(e->u.op.right->pos, "type can't be compared");
						}
					}
					return expTy(NULL, Ty_Void());
				case A_ltOp:
				case A_leOp:
				case A_gtOp:
				case A_geOp://just support integer, can string be compared here?  or  a[3]>2?
					if (!tyMatch(left.ty,right.ty)) {
						EM_error(e->pos, "same type required");
					}
					if(left.ty->kind!=Ty_int){
						EM_error(e->pos, "integer required");
					}
					return expTy(Tr_relExp(oper,left.exp,right.exp), Ty_Int());	
      			default:
					assert(0);	
      		}
      	}
    	case A_recordExp:
    	{
    	
      		Ty_ty recordTy = actual_ty(S_look(tenv,e->u.record.typ));
      		if(recordTy!=NULL)
      		{
      			if(recordTy->kind !=Ty_record)
      			{
      				EM_error(e->pos, "%s is not a record type", S_name(e->u.record.typ));	
					return expTy(NULL, Ty_Record(NULL));
      			}
      			//if(efieldsMatch(level,bk,venv,tenv,recordTy,e)){
      			/*  efieldsMatch here call transexp,may cause a repeat call to kind:a_stringexp,thus cause frame++ */
      			if(1){
					  int index=0;
					  Tr_expList result=NULL;
					  A_efieldList el;
					  for(el=e->u.record.fields;el;el=el->tail,index++){
						  struct expty val=transExp(level,bk,venv,tenv,el->head->exp);
						  Tr_expList_prepend(val.exp,&result);
					  }
					  return expTy(Tr_recordExp(index,result),recordTy);
				  }
      				
      		}
      		else 
      			EM_error(e->pos, "undefined type %s", S_name(e->u.record.typ)); 
			return expTy(NULL,Ty_Record(NULL));
		}
    	case A_seqExp:
    	{
      		A_expList list = e->u.seq;
			Tr_expList result=NULL;
      		if(!list)
      		{
      			return expTy(NULL, Ty_Void());
      		}
      		struct expty now;
      		while (list)
		    {
				now=transExp(level,bk,venv, tenv, list->head);
				Tr_expList_prepend(now.exp,&result);
				list = list->tail;
			}
			  return expTy(Tr_seqExp(result),now.ty);
      	}
    	case A_assignExp:
    	{
      		struct expty  tvar =  transVar(level,bk,venv,tenv,e->u.assign.var);
			struct expty evar =  transExp(level,bk,venv,tenv,e->u.assign.exp);
			/* if tvar's type is deterministic, kind can be nil*/
			/* reference test44.tig */
			//if(evar.ty->kind==Ty_nil)
				//EM_error(e->pos, "init should not be nil without type specified");
			if (tvar.ty!=Ty_Void()&&!tyMatch(tvar.ty, evar.ty)) //ty_void?
				EM_error(e->pos, "unmatched assign exp");
			return expTy(Tr_assignExp(tvar.exp,evar.exp), Ty_Void());
			//return expTy(NULL,Ty_Void());
		}
    	case A_ifExp:
    	{//printf("ifexp,condition kind:%d\n",e->u.iff.test->kind);
      		struct expty condition = transExp(level,bk,venv,tenv,e->u.iff.test);
      		struct expty then = transExp(level,bk,venv,tenv,e->u.iff.then); 
			struct expty elsee ={NULL,NULL};
      		if(condition.ty->kind != Ty_int)
      			EM_error(e->u.iff.test->pos, "int required");
			if(e->u.iff.elsee!=NULL)
			{
				elsee = transExp(level,bk,venv,tenv,e->u.iff.elsee);
				if(!tyMatch(then.ty,elsee.ty))
				{
			  	 EM_error(e->pos, "then exp and else exp type mismatch");
				}
			}else 
			{
    			if (then.ty != Ty_Void()) 
      				EM_error(e->pos, "if-then exp's body must produce no value");	
    			//return expTy(NULL, Ty_Void());
    		}; 
			return expTy(Tr_ifExp(condition.exp,then.exp,elsee.exp), then.ty);
		}
    	case A_whileExp:
    	{
			Tr_exp done = Tr_doneExp();
			struct expty condition = transExp(level,bk,venv,tenv,e->u.whilee.test);
			struct expty body =transExp(level,done,venv,tenv,e->u.whilee.body);
			if(condition.ty->kind !=Ty_int){
				EM_error(e->pos, "int required");
				return expTy(NULL,Ty_Void());
			}
			if (body.ty != Ty_Void()){
				EM_error(e->pos, "while body must produce no value");
				return expTy(NULL,Ty_Void());
			}
			return expTy(Tr_whileExp(condition.exp, body.exp, done), Ty_Void());
					}

    	case A_forExp:
    	{//printf("forexp\n");
			A_exp by = A_WhileExp(0, A_OpExp(0, A_leOp, A_VarExp(0, A_SimpleVar(0,e->u.forr.var)), A_VarExp(0, A_SimpleVar(0, S_Symbol("limit")))), A_SeqExp(0, A_ExpList(e->u.forr.body, A_ExpList(A_AssignExp(0, A_SimpleVar(0,e->u.forr.var), A_OpExp(0, A_plusOp, A_VarExp(0, A_SimpleVar(0,e->u.forr.var)), A_IntExp(0, 1))), NULL))));
			A_exp transwhile = A_LetExp(0, A_DecList(A_VarDec(0, e->u.forr.var,S_Symbol("int"), e->u.forr.lo),A_DecList(A_VarDec(0, S_Symbol("limit"), S_Symbol("int"), e->u.forr.hi),NULL)), by);
			return transExp(level, bk, venv, tenv, transwhile);
			
		}
		case A_breakExp: {
			if (!bk) return expTy(Tr_noExp(), Ty_Void());
			return expTy(Tr_breakExp(bk), Ty_Void());
		}
    	case A_letExp:
    	{
      		A_decList decs;
			Tr_expList result=NULL;
      		
			S_beginScope(venv);
			S_beginScope(tenv);
			int debugnum=0;
			for (decs = e->u.let.decs; decs!=NULL; decs = decs->tail) 
			{
				debugnum++;
				Tr_exp now=transDec(level,bk,venv, tenv, decs->head);
				//printf("letexp add dec ,times:%d\n",debugnum);
				/*if(decs->head->kind==A_typeDec){
					if(decs->head->u.type->tail!=NULL&&S_name(decs->head->u.type->head->name)==S_name(decs->head->u.type->tail->head->name))
						EM_error(e->pos,"two types have the same name");
				}
				if(decs->head->kind==A_functionDec){
					if(decs->head->u.function->tail!=NULL&&S_name(decs->head->u.function->head->name)==S_name(decs->head->u.function->tail->head->name))
						EM_error(e->pos,"two functions have the same name");
				}*/
				Tr_expList_prepend(now,&result);
			}struct expty ex;//printf("letexp:%d\n",e->u.let.body->kind);
			ex = transExp(level,bk,venv, tenv, e->u.let.body);
			Tr_expList_prepend(ex.exp,&result);
			S_endScope(venv);
			S_endScope(tenv);
			return expTy(Tr_seqExp(result),ex.ty);
			//return expTy(Tr_noExp(),ex.ty);
		}
    	case A_arrayExp:
    	{
      		struct expty size = transExp(level,bk,venv, tenv, e->u.array.size);
  			struct expty init = transExp(level,bk,venv, tenv, e->u.array.init);
  			Ty_ty type =S_look(tenv, e->u.array.typ);
  			if(type==NULL)
  				EM_error(e->pos, "undefined type %s (arrayExp)", S_name(e->u.record.typ)); 
  			else
  			{
  				if (size.ty != Ty_Int()) 
    					EM_error(e->pos, "type of array size is not integer");
  				if(actual_ty(type)->kind !=Ty_array){
					EM_error(e->pos, "%s is not a array", S_name(e->u.array.typ));	
				return expTy(NULL,Ty_Void());
				}
				if(tyMatch(actual_ty(type)->u.array,init.ty))//whether x is match(array of x)
					return expTy(Tr_arrayExp(size.exp,init.exp), actual_ty(type));
				else
					EM_error(e->pos,"type mismatch");
  			}
  			return expTy(NULL,Ty_Void());//wrong
  		}
     }
}
Tr_exp transDec(Tr_level level,Tr_exp bk,S_table venv,S_table tenv, A_dec d)
{
	switch (d->kind)
	{
		case A_varDec:// wrong here not return,should be handled
		{
			Ty_ty type;
			//printf("transdec in, init exp kind:%d\n",d->u.var.init->kind);
			struct expty e = transExp(level,bk,venv,tenv,d->u.var.init);
			Tr_access varac=Tr_allocLocal(level,d->u.var.escape);
			if (d->u.var.typ!=NULL) {
    			type = S_look(tenv, d->u.var.typ);
   			if (type==NULL){
      				EM_error(d->pos, "undefined type %s", S_name(d->u.var.typ));
				}
     				//return;
			if(!( tyMatch(type,e.ty) && type!=Ty_Nil() ))
      				EM_error(d->pos, "type mismatch");//test28.tig
      				//return;
  			}
			else if(e.ty->kind==Ty_nil)
				EM_error(d->pos,"init should not be nil without type specified");
			if(e.ty->kind==Ty_nil&&d->u.var.typ!=NULL)
				S_enter(venv,d->u.var.var,E_VarEntry(varac,type));
			else
				S_enter(venv,d->u.var.var,E_VarEntry(varac,e.ty));
			return Tr_assignExp(Tr_simpleVar(varac,level),e.exp);
		}
		case  A_functionDec:
		{
			A_fundecList fcl; 
			Ty_ty result;
			Ty_tyList formalTys,s;
			A_fieldList l;
			E_enventry fun;
			struct expty e;

		    for(fcl = d->u.function;fcl;fcl = fcl->tail)
			{
				if (fcl->head->result!=NULL)
				{
					result = S_look(tenv,fcl->head->result);
					if(result==NULL)
					{
						EM_error(fcl->head->pos, "undefined type for return type");
						result = Ty_Void();
					}
				}
				else
				{
					result = Ty_Void();
				}
				formalTys = makeFormalTyList(tenv, fcl->head->params);
				Temp_label funl=Temp_newfunlabel();
				Tr_level lev=Tr_newLevel(level, funl, makeFormals(fcl->head->params));
			    S_enter(venv,fcl->head->name,E_FunEntry(lev,funl,formalTys,result));
			}
			for(fcl = d->u.function;fcl;fcl = fcl->tail)
			{
				A_fundec f = fcl->head;
				S_beginScope(venv);
				E_enventry funname = S_look(venv, f->name);
				formalTys = funname->u.fun.formals;
				Tr_accessList acls = funname->u.fun.level->formals;
				for (l = f->params, s = formalTys; l && s && acls; l = l->tail, s = s->tail,acls=acls->tail) {
					S_enter(venv, l->head->name, E_VarEntry(acls->head,s->head));
				}
				e = transExp(funname->u.fun.level,bk,venv, tenv, f->body);
				fun = S_look(venv, f->name);
				if (!tyMatch(fun->u.fun.result, e.ty)) {
					EM_error(f->pos, "procedure returns value");
				} 
				if (f->result == NULL) {
					Tr_procEntryExit(funname->u.fun.level, e.exp, acls);
				}
				else {
					Tr_procEntryExit2(funname->u.fun.level, e.exp, acls);
				}
				S_endScope(venv);
			}
			return Tr_noExp();//or NULL?
		}
		case A_typeDec:{
  			A_nametyList nl;
			bool cyc=TRUE;
			for (nl = d->u.type; nl; nl = nl->tail) {
			S_enter(tenv, nl->head->name, Ty_Name(nl->head->name,NULL));
			}
			for (nl = d->u.type; nl; nl = nl->tail){
				Ty_ty result = transTy(tenv,nl->head->ty);
				if( result -> kind != Ty_name ){

					cyc=FALSE;
				}
				Ty_ty change = S_look(tenv, nl->head->name);
				change->u.name.ty = result;
			}
    			if(cyc)
				EM_error(d->pos, "illegal type cycle");
    			return Tr_noExp();
		}
	}
}

Ty_ty  transTy(S_table tenv, A_ty ty)
{
	Ty_ty final ;
	Ty_fieldList tfl;
	switch(ty->kind)
	{
		case A_nameTy:
			final = S_look(tenv,ty->u.name);
			if(!final)
			{
				EM_error(ty->pos,"undefined type %s", S_name(ty->u.name));
				return Ty_Int();
			}
			return final;
		case A_recordTy://not handle
			tfl = makeFieldLists(tenv,ty->u.record);
			return Ty_Record(tfl);
		case A_arrayTy:
			final = S_look(tenv,ty->u.array);
			if(!final)
			{
				EM_error(ty->pos,"undefined type %s", S_name(ty->u.array));
			}
			return Ty_Array(final);
		default:
			assert(0);
	}
}

Ty_fieldList makeFieldLists(S_table tenv, A_fieldList fl)
{
	Ty_ty ty;
	A_fieldList afl =fl;
	Ty_fieldList final = NULL ,head = NULL;
	Ty_field t;
	for (;afl;afl=afl->tail)
	{
		ty = S_look(tenv,afl->head->typ);
		if(!ty)
		{
			EM_error(afl->head->pos,"undefined type %s", S_name(afl->head->typ));
		}
		else
		{
			t = Ty_Field(afl->head->name,ty);
			if(!final)
			{
				final = Ty_FieldList(t,NULL);
				head = final;
			}else
			{
				final->tail = Ty_FieldList(t,NULL);
				final = final->tail;
			}
		}
		
	}
	return head;
}

Ty_tyList makeFormalTyList(S_table t , A_fieldList fl)
{
	Ty_ty ty;
	A_fieldList afl = fl;
	Ty_tyList final = NULL,head = NULL;
	for (;afl;afl = afl->tail)
	{
		ty = S_look(t,afl->head->typ);
		if(!ty)
		{
			EM_error(afl->head->pos,"undefined type %s", S_name(afl->head->typ));
			ty = Ty_Int();
		}
		if(!final)
		{
			final = Ty_TyList(ty,NULL);
			head = final;
		}else
		{
			final->tail = Ty_TyList(ty,NULL);
			final = final->tail;
		}
	}
	return head;
}
bool tyMatch(Ty_ty t1,Ty_ty t2)
{
	Ty_ty tt1 = actual_ty(t1);
	Ty_ty tt2 = actual_ty(t2);
	int tk1 = tt1->kind;
	int tk2 = tt2->kind;
	bool match=0;
	if(tk1==Ty_nil||tk2==Ty_nil){
		if(tk1==Ty_record||tk2==Ty_record)
			match=1;
	}
	else{
		if(tk1==tk2){
			if(tk1==Ty_record||tk1==Ty_array)	
				match=(tt1==tt2);
			else
				match=1;
		}
	}
	return match;
}

bool argsMatch(Tr_level level, Tr_exp bk,S_table venv,S_table tenv,A_expList el,Ty_tyList tl,A_exp fun)
{//printf("argsmatch in\n");
	struct expty t;
	A_expList ell = el;
	Ty_tyList tll = tl;
	while(ell && tll)
	{//printf("argsmatch 1,kind1:%d,kind2:%d\n",el->head->kind,tl->head->kind);
		t = transExp(level,bk,venv,tenv,ell->head);//printf("argsmatch 2\n");
		if (!tyMatch(t.ty,tll->head))
		{
			EM_error(fun->pos, "para type mismatch\n");
			return FALSE;
		}
		ell = ell->tail;
		tll = tll->tail;
	}
	if (ell && !tll)
	{
		EM_error(fun->pos,"too many params in function %s\n", S_name(fun->u.call.func));
		return FALSE;
	}else if(!ell && tll) 
	{
		EM_error(fun->pos, "less params in function %s\n", S_name(fun->u.call.func));
		return FALSE;
	}else
	{
		return TRUE;
	}
}
bool efieldsMatch(Tr_level level, Tr_exp bk,S_table venv, S_table tenv,Ty_ty ty,A_exp e)
{
	struct expty et;
	Ty_fieldList tfl = ty->u.record;
	A_efieldList efl = e->u.record.fields;
	while (tfl &&efl)
	{
		et = transExp(level,bk,venv,tenv,efl->head->exp);
		if( tfl->head->name != efl->head->name ||!tyMatch(tfl->head->ty,et.ty))
		{
			EM_error(e->pos, "unmatched name: type in %s", S_name(e->u.record.typ));
			return FALSE;
		}
		tfl = tfl->tail;
		efl = efl->tail;
	}
	if(tfl &&!efl)
	{
		EM_error(e->pos, "less fields in %s", S_name(e->u.record.typ));
		return FALSE;
	}else if (!tfl && efl)
	{
		EM_error(e->pos, "too many field in %s", S_name(e->u.record.typ));
		return FALSE;
	}
	return TRUE;
}
Ty_ty actual_ty(Ty_ty ty)
{
	if (!ty) return ty;
	if (ty->kind == Ty_name)  return actual_ty(ty->u.name.ty);
	else return ty;
}

U_boolList makeFormals(A_fieldList params) {
	/* assume escape=1 */
	U_boolList head = NULL, tail = NULL;
	A_fieldList p = params;
	for (; p; p = p->tail) {
		if (head) {
			tail->tail = U_BoolList(p->head->escape, NULL);
			tail = tail->tail;
		} else {
			head = U_BoolList(p->head->escape, NULL);
			tail = head;
		}
	}
	return head;
}
