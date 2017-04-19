#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "tree.h"
#include "absyn.h"
#include "assem.h"
#include "frame.h"
#include "graph.h"
#include "flowgraph.h"
#include "errormsg.h"
#include "table.h"


Temp_tempList FG_def(G_node n) {
	//your code here.
	AS_instr i = G_nodeInfo(n);
	switch(i->kind){
		case I_OPER:{
			return i->u.OPER.dst;
			break;
		}
		case I_LABEL:{
			return NULL;
			break;
		}
		case I_MOVE:{
			return i->u.MOVE.dst;
			break;
		}
	}
	return NULL;
}

Temp_tempList FG_use(G_node n) {
	//your code here.
	AS_instr i = G_nodeInfo(n);
	switch(i->kind){
		case I_OPER:{
			return i->u.OPER.src;
			break;
		}
		case I_LABEL:{
			return NULL;
			break;
		}
		case I_MOVE:{
			return i->u.MOVE.src;
			break;
		}
	}
	return NULL;
}

bool FG_isMove(G_node n) {
	//your code here.
	AS_instr i = G_nodeInfo(n);
	if(i->kind==I_MOVE){
		return TRUE;
	}
	else{
		return FALSE;
	}
}

//G_graph FG_AssemFlowGraph(AS_instrList il, F_frame f) {
G_graph FG_AssemFlowGraph(AS_instrList il) {
	//your code here.
	G_graph result=G_Graph();
	G_nodeList nodes = NULL;
	TAB_table tb=TAB_empty();
	G_node pred=NULL;
	AS_instrList ils=il;
	for(;ils;ils=ils->tail){
		AS_instr i=ils->head;
		G_node me=G_Node(result,i);
		if(pred){
			G_addEdge(pred,me);
		}
		pred=me;
		switch(i->kind){
			case I_LABEL:{
				TAB_enter(tb,i->u.LABEL.label,me);
				break;
			}
			case I_MOVE:{
				break;
			}
			case I_OPER:{
				if(i->u.OPER.jumps){
					nodes=G_NodeList(me,nodes);
				}
				break;
			}
		}
	}
	for(;nodes;nodes=nodes->tail){
		G_node branch=nodes->head;
		AS_instr i = G_nodeInfo(branch);
		Temp_tempList lbs=i->u.OPER.jumps->labels;
		for(;lbs;lbs=lbs->tail){
			G_node target=TAB_look(tb,lbs->head);
			if(!target){
				printf("label jump to is not existed\n");
			}
			if(!G_goesTo(branch,target)){
				G_addEdge(branch,target);
			}
		}
	}
	return result;
}
