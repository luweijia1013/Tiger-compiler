#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "tree.h"
#include "absyn.h"
#include "assem.h"
#include "frame.h"
#include "graph.h"
#include "color.h"
#include "table.h"

#define MAX 16
#define colorNum 8
static string regName[MAX];
static int regIndex[MAX];
#define EAX 0
#define EBX 1
#define ECX 2
#define EDX 3
#define ESI 4
#define EDI 5
#define EBP 6
#define ESP 7

struct COL_workList* head;

struct COL_stackElement {
	G_node node;
	G_nodeList adjnodes;
	bool mayspill;
};

struct COL_stack {
	int top;
	struct COL_stackElement* stack;
};

struct COL_workList {
	G_node node;
	struct COL_workList* next;
};

static struct COL_stack* COL_StackInit() {
	struct COL_stack* stack;
	stack = checked_malloc(sizeof(struct COL_stack));
	stack->top = -1;
	stack->stack = checked_malloc(100*sizeof(struct COL_stackElement));//here 100 is an assuming number.
	return stack;
}

static bool preColored(G_node node) {
	Temp_temp temp;

	assert(node!=NULL);
	temp = (Temp_temp)G_nodeInfo(node);
	Temp_tempList all = F_registers();
	int i = 0;
	for (i; i < 8; i++) {
		if (all->head == temp) {
			return TRUE;
		}
		all = all->tail;
	}
	return FALSE;
}

static int degree(G_node node) {
	int degree = G_degree(node)/2;
	return (degree);
}

static void COL_Push(struct COL_stack* stk, G_node node) {
	struct COL_stackElement ele;
	G_nodeList neighbour;
	struct COL_workList* insert=NULL;
	ele.adjnodes = G_succ(node);
	ele.node = node;
	neighbour = G_succ(node);
	int degrees = degree(node);
	while(neighbour != NULL) {
		G_rmEdge(node, neighbour->head);
		G_rmEdge(neighbour->head, node);
		if(preColored(neighbour->head) == FALSE && degree(neighbour->head) == colorNum-1) {
			insert = checked_malloc(sizeof(*insert));
			insert->node = neighbour->head;
			insert->next = head->next;
			head->next = insert;
		}
		neighbour = neighbour->tail;
	}
	if(degrees < colorNum) {
		ele.mayspill = FALSE;
	}
	else {
		ele.mayspill = TRUE;
	}
	stk->top = stk->top + 1;
	stk->stack[stk->top]=ele;//can here use []operator?
}

static G_node COL_Pop(struct COL_stack* stk) {
	struct COL_stackElement ele;
	G_nodeList neighbour;
	G_node node;

	ele = stk->stack[stk->top];
	stk->top=stk->top-1;
	neighbour = ele.adjnodes;
	node = ele.node;
	while(neighbour != NULL) {
		G_addEdge(node, neighbour->head);
		G_addEdge(neighbour->head, node);
		neighbour = neighbour->tail;
	}
	return node;
}

static struct COL_workList* COL_WorkListInit(G_graph ig) {
	struct COL_workList *head2, *ptr;
	G_nodeList nodes;
	nodes = G_nodes(ig);
	head2 = checked_malloc(sizeof(struct COL_workList));
	head2->node = NULL;
	head2->next = NULL;
	while(nodes != NULL) {
		if(preColored(nodes->head) == FALSE && degree(nodes->head) < colorNum) {
			ptr = checked_malloc(sizeof(struct COL_workList));
			ptr->node = nodes->head;
			ptr->next = head2->next;
			head2->next = ptr;
		}
		nodes = nodes->tail;
	}
	return head2;
}

static G_node nextNode(G_graph ig) {  /*Select next node to simplify*/
	G_nodeList nodes;          /*If workList(refed by "head") is empty, find one node */   
	G_node max;					   /*in the remaining graph that has the maxmium degree*/
	int maxdeg;
	if(head->next != NULL) {
		struct COL_workList* tmp = head->next;
		head->next = tmp->next;
		return tmp->node;
	}
	else {
		nodes = G_nodes(ig);
		max = NULL;
		maxdeg = 0;
		while(nodes != NULL) {
			if(preColored(nodes->head) == FALSE &&
				degree(nodes->head) > maxdeg) {
				maxdeg = degree(nodes->head);
				max = nodes->head;
			}
			nodes = nodes->tail;
		}
		return max;
	}
}

static int nextColor(G_node node, TAB_table tab) {
	G_nodeList nodes;
	int idx[MAX], i;
	int* p;
	Temp_temp test = G_nodeInfo(node);
	nodes = G_succ(node);
	for(i=0;i<MAX;i++)
		idx[i]=0;
	for(;nodes;nodes=nodes->tail){
		Temp_temp test2 = G_nodeInfo(nodes->head);
		p = (int*)TAB_look(tab, nodes->head);
		if(p) {
			idx[*p]=1;
		}
	}
	for(i=0;i<colorNum-2;i++) {//start from 1 means can't use eax;-2 means can't use ebp esp
		if(idx[i]==0) {
			break;
		}
	}
	if(i<colorNum-2) {
		return i;
	}
	else {
		return -1;
	}
}



struct COL_result COL_color(G_graph ig, Temp_map initial, Temp_tempList regs) {
	//your code here.
	struct COL_result ret;
	struct COL_stack* stk;
	Temp_map coloring=NULL;
	Temp_tempList spills=NULL;
	Temp_map map = Temp_empty();

	head=COL_WorkListInit(ig);
	stk=COL_StackInit();
	TAB_table tab=TAB_empty();
	int i;
	for(i=0;i<colorNum;i++) {
		regIndex[i]=i;
	}
	regName[EAX]="%eax";
	regName[EBX]="%ebx";
	regName[ECX]="%ecx";
	regName[EDX]="%edx";
	regName[EDI]="%edi";
	regName[ESI]="%esi";
	regName[EBP]="%ebp";
	regName[ESP]="%esp";
	G_nodeList nodes;
	for (nodes = G_nodes(ig); nodes != NULL; nodes = nodes->tail) {
		Temp_temp test = G_nodeInfo(nodes->head);
		if (preColored(nodes->head) == TRUE) {
			Temp_tempList ptr = F_registers();
			for (i = 0; i<colorNum; i++) {
				if (ptr->head == G_nodeInfo(nodes->head)) {
					TAB_enter(tab, nodes->head, &regIndex[i]);
					Temp_enter(map, G_nodeInfo(nodes->head), regName[i]);
					break;
				}
				else {
					ptr = ptr->tail;
				}
			}
			assert(i != colorNum);
		}
	}

	int num=0;
	G_node node;
	while((node=nextNode(ig))!=NULL) {
		COL_Push(stk, node);
		num++;
	}
	for(i=0;i<num;i++) {
		node = COL_Pop(stk);
		int curcol;
		curcol = nextColor(node, tab);
		if(curcol>=0) {
			TAB_enter(tab, node, &regIndex[curcol]);
			Temp_enter(map, G_nodeInfo(node), regName[curcol]);
		}
		else {
			if(spills==NULL) {
				spills = Temp_TempList(G_nodeInfo(node), NULL);
			}
			else {
				spills = Temp_TempList(G_nodeInfo(node), spills);
			}
		}
	}
	
	ret.spills = spills;
	ret.coloring = Temp_layerMap(map, initial);
	return ret;
}
