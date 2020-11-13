#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#define TRUE 1
#define FALSE 0
#define UNCERTAIN -1

typedef struct Literal
{
	int bool;			//-1δ����0Ϊ�٣�1Ϊ��
	int ID;				//�����������ű�ʾ����
	struct Literal* next;
}Literal;

typedef struct Clause 
{
	int length;
	int bool_removed;
	struct Literal* head;
	struct Clause* prev;
	struct Clause* next;
	int unsatisfied_literal;
}Clause;

typedef struct Dimacs
{
	int varnum;
	int clanum;
	int removed_Cl_num;
	struct Clause* root;
	struct ID_occurs_List* ID_list_root;
	struct removed_Cl* lastlevel_Cl_root;     //������ס֮ǰ���ɾ�����Ӿ�ָ�루���еڼ����־��
	struct Pair* value_pair_root;			  //�����洢�Ѿ���ֵ�ı�Ԫ
	int valued_number;
}Dimacs;

typedef struct removed_Cl
{
	int removed_level;
	struct Clause* Cl;
	struct removed_Cl* next;
}removed_Cl;

typedef struct Pair
{
	int assigned_level;
	int ID;
	int bool;
	struct Pair* next;
}Pair;

typedef struct ID_occurs_List
{
	int ID;
	int relevent_Cl_length;
	struct ID_occurs_List* next;
}ID_occurs_List;

Dimacs* Read_cnf(char* filename);
void addClause(Dimacs* data,int literal_num,int* line);
void addliteral(Clause* clause, int ID, Dimacs* data);
void add_ID_list_length(Dimacs* data, int ID);

