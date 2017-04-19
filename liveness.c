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
#include "liveness.h"
#include "table.h"

Live_moveList Live_MoveList(G_node src, G_node dst, Live_moveList tail) {
	Live_moveList lm = (Live_moveList) checked_malloc(sizeof(*lm));
	lm->src = src;
	lm->dst = dst;
	lm->tail = tail;
	return lm;
}

static void enterLiveMap(G_table t,G_node flowNode,Temp_tempList temp){
  G_enter(t,flowNode,temp);
}

static Temp_tempList lookupLiveMap(G_table t,G_node flownode){
  return (Temp_tempList)G_look(t,flownode);
}

static G_nodeList reverseList(G_nodeList g_nodeList){
  G_nodeList reverse = NULL;
  while(g_nodeList!=NULL){
    reverse = G_NodeList(g_nodeList->head,reverse);
    g_nodeList = g_nodeList->tail;
  }
  return reverse;
}

static bool isMember(Temp_tempList set,Temp_temp elem){
  while(set!=NULL){
    if(set->head==elem)
      return TRUE;
    set = set->tail;
  }
  return FALSE;
}

static int sizeofset(Temp_tempList temp_tempList){
  int size = 0;
  for(;temp_tempList;temp_tempList=temp_tempList->tail){
  	size++;
  }
  return size;
}

bool inTempList(Temp_temp temp, Temp_tempList temps) {
	assert(temp!=NULL);
	if(temps == NULL) {
		return FALSE;
	}
	else {
		if(temp == temps->head) {
			return TRUE;
		}
		else {
			return inTempList(temp, temps->tail);
		}
	}
}

static Temp_tempList setminus(Temp_tempList set1_,Temp_tempList set2_){
  Temp_tempList re_head = NULL,re_tail = NULL,set1=set1_;
  while(set1!=NULL){
    if(!isMember(set2_,set1->head)){
      Temp_tempList temp = Temp_TempList(set1->head,NULL);
      if(re_tail==NULL){
        re_head = re_tail = temp;
      }else{
        re_tail->tail = temp;
        re_tail = re_tail->tail;
      }
    }
    set1 = set1->tail;
  }
  return re_head;
}

static Temp_tempList setplus(Temp_tempList set1_,Temp_tempList set2_){
  Temp_tempList re_head = NULL,re_tail = NULL,set1 = set1_,set2=set2_;
  while(set1!=NULL){
    Temp_tempList temp = Temp_TempList(set1->head,NULL);
    if(re_tail==NULL){
      re_head = re_tail = temp;
    }else{
      re_tail = re_tail->tail = temp;
    }
    set1 = set1->tail;
  }
  while(set2!=NULL){
    Temp_tempList temp = Temp_TempList(set2->head,NULL);
    if(!isMember(set1_,set2->head)){
      if(re_tail == NULL){
        re_head = re_tail = temp;
      }else{
        re_tail = re_tail->tail = temp;
      }
    }
    set2 = set2->tail;
  }
  return re_head;
}

static G_node getNodeByTemp(TAB_table table,G_graph graph,Temp_temp temp){
  G_node g_node = TAB_look(table,temp);
  if(g_node==NULL){
    g_node = G_Node(graph,temp);
    TAB_enter(table,temp,g_node);
  }
  return g_node;
}

// what's this function's use? i don't know.
Temp_temp Live_gtemp(G_node n) {
	//your code here.
	return G_nodeInfo(n); // need (Temp_temp) kind trans?   //why return n->info wrong?
}

struct Live_graph Live_liveness(G_graph flow) {
	//your code here.
	struct Live_graph lg;
	G_graph graph=G_Graph();
	Live_moveList mvls=NULL;

	G_table table_in=G_empty();
	G_table table_out=G_empty();
	G_nodeList ori=G_nodes(flow);
	G_nodeList new=reverseList(ori);
	bool diff=FALSE;
	bool first = TRUE;
	while(first||diff){
		first = FALSE;
		diff = FALSE;
		G_nodeList lt=new;
		for(;lt;lt=lt->tail){
			G_node now=lt->head;
			Temp_tempList old_out=lookupLiveMap(table_out,now);
			Temp_tempList old_in=lookupLiveMap(table_in,now);
			Temp_tempList new_out=NULL;
			Temp_tempList new_in=NULL;
			G_nodeList succ=G_succ(now);
			for(;succ;succ=succ->tail){
				G_node succone=succ->head;
				assert(succone);
				new_out=setplus(new_out,lookupLiveMap(table_in,succone));
			}
			new_in=setplus(FG_use(now),setminus(new_out,FG_def(now)));
			if(sizeofset(new_out)!=sizeofset(old_out)){
				diff=TRUE;
				enterLiveMap(table_out,now,new_out);
			}
			if(sizeofset(new_in)!=sizeofset(old_in)){
				diff=TRUE;
				enterLiveMap(table_in,now,new_in);
			}
		}
	}
	TAB_table tempnode_tb=TAB_empty();
	G_nodeList lt=new;
	for(;lt;lt=lt->tail){
		G_node now=lt->head;
		Temp_tempList dflt=FG_def(now);
		Temp_tempList uslt=FG_use(now);
		Temp_tempList out=lookupLiveMap(table_out,now);
		Temp_tempList copy = out;
		if(FG_isMove(now)){
			out=setminus(out,uslt);// for those move instruction
			//G_node dst=getNodeByTemp(tempnode_tb,graph,dflt->head);
			//G_node src=getNodeByTemp(tempnode_tb,graph,uslt->head);
			//mvls=Live_MoveList(src,dst,mvls);// src->dst or dst->src ??
		}
		for (; uslt; uslt = uslt->tail) {
			Temp_temp use_one = uslt->head;
			getNodeByTemp(tempnode_tb, graph, use_one);
		}
		for(;dflt;dflt=dflt->tail){
			Temp_temp df_one=dflt->head;
			G_node df_node=getNodeByTemp(tempnode_tb,graph,df_one);
			for(out=copy;out;out=out->tail){
				G_node out_node=getNodeByTemp(tempnode_tb,graph,out->head);
				if (G_nodeKey(out_node) == G_nodeKey(df_node)) continue;
				G_addEdge(out_node,df_node);
				G_addEdge(df_node,out_node);
			}
		}
	}
	lg.graph=graph;
	lg.moves=mvls;
	return lg;
}


