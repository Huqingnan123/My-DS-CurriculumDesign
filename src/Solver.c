#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "Cnfparser.h"
#include "Solver.h"
#include "BinaryPuzzle.h"

int level = 0;

int DPLL(Dimacs* data)
{
	int unitID;
	int select_ID;
	while ((unitID = findUnitClause(data)) != 0)            //���Ӿ䴫������
	{
		unitPropagation(data, unitID);
		if (have_no_clause(data))
			return TRUE;
		else if (haveEmptyClause(data))
			return FALSE;
	}
	if ((select_ID = select_mostID(data)) != 0)    //�����������Ӿ��еĳ���Ƶ�ʵĸߵ�ѡȡ���ָ�ֵ
	{
		Assign_True(data, select_ID);              //ѡ�����ָ���
		if (DPLL(data))
			return TRUE;
		else
		{
			level = find_level(data, select_ID);    //�ҵ���ǰִ�к�������DPLL�㷨�еĲ���
			Go_back(data);							//������ͻ֮����ݵ���ǰ�㣬����ǰ��ѡ��������ID��֮����
			Assign_False(data, select_ID);
			return DPLL(data);
		}
	}
	if (have_no_clause(data))                       //���Ƴ�ȫ���Ӿ䣬����True
		return TRUE;
	else
		return FALSE;
}

int findUnitClause(Dimacs* data)
{
	Clause* Temp = data->root;
	while (Temp != NULL)
	{
		if (isUnitClause(Temp))         //�����Ӿ伯��Ѱ�ҵ��Ӿ�
		{
			Literal* Li = Temp->head;
			while (Li != NULL)
			{
				if (Li->bool == -1)
					return Li->ID;
				Li = Li->next;
			}
		}
		Temp = Temp->next;
	}
	return 0;
}

int isUnitClause(Clause* Cl)
{
	int satisfy=0, unsatisfy=0;
	if (Cl->bool_removed == 0)
	{
		Literal* Li = Cl->head;
		while (Li != NULL)
		{
			if (Li->bool == 1)
				satisfy++;
			if (Li->bool == 0)
				unsatisfy++;
			Li = Li->next;
		}
		if (unsatisfy == Cl->length - 1 && satisfy == 0)     //���Ӿ�δ���Ƴ��Ҳ���������ΪCl->length - 1˵�����Ӿ�Ϊ���Ӿ�
			return 1;
	}
	return 0;
}

int isEmptyClause(Clause* Cl)
{
	Literal* Li = Cl->head;
	while (Li != NULL)
	{
		if (Li->bool != 0)
			return 0;
		else
			Li = Li->next;
	}
	return 1;
}

int haveEmptyClause(Dimacs* data)
{
	Clause* Temp = data->root;
	while (Temp != NULL)
	{
		if (isEmptyClause(Temp))     //���Ӿ�������boolֵȫ��Ϊ0������Ӿ�Ϊ���Ӿ䣨�������㣩
			return 1;
		Temp = Temp->next;
	}
	return 0;
}

int have_no_clause(Dimacs* data)
{
	Clause* Cl = data->root;
	while (Cl != NULL)
	{
		if (Cl->bool_removed == 1)     //�������������Ӿ���ѱ��Ƴ��������Ӿ����
		{
			Cl = Cl->next;
			continue;
		}
		else
			return 0;
	}
	return 1;
}

void unitPropagation(Dimacs* data, int ID)    //IDΪ���Ӿ������
{
	Clause* Cl = data->root;
	Literal* Li;
	while (Cl != NULL)
	{
		if (Cl->bool_removed == 1)
		{
			Cl = Cl->next;
			continue;
		}
		else if (Cl->bool_removed == 0)      //������δ�Ƴ��Ӿ���Ѱ���봫��ID��ص����ֽ�����Ӧ�ĸ�ֵ
		{
			Li = Cl->head;
			while (Li != NULL)
			{
				if (Li->ID == ID)
				{
					Li->bool = 1;
					Cl->bool_removed = 1;   //boolΪ1�൱���Ƴ����Ӿ�
					save_removed_Cl(data, Cl);
					Li = Li->next;
				}
				else if (Li->ID == -ID)
				{
					Li->bool = 0;			//boolΪ0�൱���Ƴ�������
					Cl->unsatisfied_literal++;
					Li = Li->next;
				}
				else
					Li = Li->next;
			}
			Cl = Cl->next;
		}
	}
	save_booled_Li(data, ID, 1);    //����ID����֮�󱣴��ڸ�ֵ���������
}

int select_mostID(Dimacs* data)
{
	int maxlength = 0;
	int most_ID = 0;
	int num;
	ID_occurs_List* head = data->ID_list_root;
	while (head != NULL)             //�����������Ӿ��еĳ���Ƶ�ʣ�length) �ĸߵ�ѡȡ���ָ�ֵ
	{
		if (maxlength < head->relevent_Cl_length && (num = find_if_valued(data, head->ID)) == 0)      
		{
			maxlength = head->relevent_Cl_length;
			most_ID = head->ID;
			head = head->next;
		}
		else
		{
			head = head->next;
			continue;
		}
	}
	return most_ID;
}

void save_removed_Cl(Dimacs* data, Clause* Cl)    //������֮ǰ�Ѿ�remove���־�ָ�룬�Ա�֮����ݻָ�bool_removedֵ
{
	removed_Cl* last_rmv = (removed_Cl*)malloc(sizeof(removed_Cl));
	last_rmv->removed_level = level;
	last_rmv->Cl = Cl;
	last_rmv->next = data->lastlevel_Cl_root;
	data->lastlevel_Cl_root = last_rmv;
	data->removed_Cl_num++;
}

void save_booled_Li(Dimacs* data, int ID, int bool)  //������֮ǰ�Ѿ���ֵ������ID���Ա�֮����ݻָ�boolֵ
{
	Pair* newPair = (Pair*)malloc(sizeof(Pair));
	newPair->assigned_level = level;
	newPair->ID = ID;
	newPair->bool = bool;
	newPair->next = data->value_pair_root;
	data->value_pair_root = newPair;
	data->valued_number++;
}

void Assign_True(Dimacs* data, int ID)    //ID���棬�����ڵ��Ӿ䴫�����Թ���
{
	level++;
	Clause* Cl = data->root;
	Literal* Li;
	while (Cl != NULL)
	{
		if (Cl->bool_removed == 1)
		{
			Cl = Cl->next;
			continue;
		}
		else if (Cl->bool_removed == 0)
		{
			Li = Cl->head;
			while (Li != NULL)
			{
				if (Li->ID == ID)
				{
					Li->bool = 1;
					Cl->bool_removed = 1;
					save_removed_Cl(data, Cl);
					Li = Li->next;
					continue;
				}
				else if (Li->ID == -ID)
				{
					Li->bool = 0;
					Cl->unsatisfied_literal++;
					Li = Li->next;
				}
				else
					Li = Li->next;
			}
			Cl = Cl->next;
		}
	}
	save_booled_Li(data, ID, 1);
}


void Assign_False(Dimacs* data, int ID)     //ID���٣�ִ���߼���Assign_Trueǡ���෴������-ID���Ӿ�ֱ���Ƴ���ֵΪID������boolΪ0���Ƴ�����
{
	Clause* Cl = data->root;
	Literal* Li;
	while (Cl != NULL)
	{
		if (Cl->bool_removed == 1)
		{
			Cl = Cl->next;
			continue;
		}
		if (Cl->bool_removed == 0)
		{
			Li = Cl->head;
			while (Li != NULL)
			{
				if (Li->ID == ID)
				{
					Li->bool = 0;
					Cl->unsatisfied_literal++;
					Li = Li->next;
				}
				else if (Li->ID == -ID)
				{
					Li->bool = 1;
					Cl->bool_removed = 1;
					save_removed_Cl(data, Cl);
					Li = Li->next;
					continue;
				}
				else
					Li = Li->next;
			}
			Cl = Cl->next;
		}
	}
	save_booled_Li(data, ID, 0);
}

int find_level(Dimacs* data, int select_ID)           //��value_pair_rootָ��ĸ�ֵ���������Ѱ���ض�ID�������䱻��ֵ���ڲ���
{
	Pair* pair = data->value_pair_root;
	while (pair != NULL)
	{
		if (pair->ID == select_ID)
			return pair->assigned_level;
		else
			pair = pair->next;
	}
	return 0;
}

int find_if_valued(Dimacs* data, int most_ID)          //����most_ID��-most_ID�Ƿ��ѱ���ֵ��
{
	Pair* pair = data->value_pair_root;
	while (pair != NULL)
	{
		if (pair->ID == most_ID || pair->ID == -most_ID)
			return pair->assigned_level;
		else
			pair = pair->next;
	}
	return 0;
}

void Go_back(Dimacs* data)             //���ݺ��ģ������޸ı�ǡ����¸�ֵ����洢����ķ���
{
	removed_Cl* backtrack = data->lastlevel_Cl_root;
	removed_Cl* temp;
	while (backtrack != NULL && backtrack->removed_level >= level)     //��ǰlevel֮ǰ�Ƴ����Ӿ�bool_removed��ǸĻ�0��������free
	{
		backtrack->Cl->bool_removed = 0;
		temp = backtrack;
		backtrack = backtrack->next;
		free(temp);
		data->removed_Cl_num--;
	}
	data->lastlevel_Cl_root = backtrack;

	Pair* backLi = data->value_pair_root;
	Pair* temp_2;
	while (backLi != NULL && backLi->assigned_level >= level)          //�����Ӿ䣬����ÿһ�����֣���ǰlevel֮ǰ��ֵ������ID�Ļ�ԭboolֵ��������free
	{
		Clause* Cl = data->root;
		Literal* Li;
		while (Cl != NULL)
		{
			if (Cl->bool_removed == 1)     //��ʾ��level֮ǰ�Ѿ��Ƴ�����������
			{
				Cl = Cl->next;
				continue;
			}
			if (Cl->bool_removed == 0)     //��δ�Ƴ����Ӿ�������ID��ͬ��
			{
				Li = Cl->head;
				while (Li != NULL)
				{
					if (Li->ID == backLi->ID && backLi->bool == 1)
					{
						Li->bool = -1;
						Cl->bool_removed = 0;
						Li = Li->next;
						continue;
					}
					if (Li->ID == -backLi->ID && backLi->bool == 1)
					{
						Li->bool = -1;
						Cl->unsatisfied_literal--;
						Li = Li->next;
						continue;
					}
					if (Li->ID == backLi->ID && backLi->bool == 0)
					{
						Li->bool = -1;
						Cl->unsatisfied_literal--;
						Li = Li->next;
						continue;
					}
					if (Li->ID == -backLi->ID && backLi->bool == 0)
					{
						Li->bool = -1;
						Cl->bool_removed = 0;
						Li = Li->next;
						continue;
					}
					else
						Li = Li->next;
				}
				Cl = Cl->next;
			}
		}
		temp_2 = backLi;
		backLi = backLi->next;
		free(temp_2);
		data->valued_number--;
	}
	data->value_pair_root = backLi;
}


