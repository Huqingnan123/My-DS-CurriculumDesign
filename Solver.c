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
	while ((unitID = findUnitClause(data)) != 0)            //单子句传播策略
	{
		unitPropagation(data, unitID);
		if (have_no_clause(data))
			return TRUE;
		else if (haveEmptyClause(data))
			return FALSE;
	}
	if ((select_ID = select_mostID(data)) != 0)    //基于文字在子句中的出现频率的高低选取文字赋值
	{
		Assign_True(data, select_ID);              //选中文字赋真
		if (DPLL(data))
			return TRUE;
		else
		{
			level = find_level(data, select_ID);    //找到当前执行函数处于DPLL算法中的层数
			Go_back(data);							//遇到冲突之后回溯到当前层，将当前层选出的文字ID反之赋假
			Assign_False(data, select_ID);
			return DPLL(data);
		}
	}
	if (have_no_clause(data))                       //已移除全部子句，返回True
		return TRUE;
	else
		return FALSE;
}

int findUnitClause(Dimacs* data)
{
	Clause* Temp = data->root;
	while (Temp != NULL)
	{
		if (isUnitClause(Temp))         //遍历子句集，寻找单子句
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
		if (unsatisfy == Cl->length - 1 && satisfy == 0)     //当子句未被移除且不满足文字为Cl->length - 1说明该子句为单子句
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
		if (isEmptyClause(Temp))     //当子句中文字bool值全部为0，则该子句为空子句（均不满足）
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
		if (Cl->bool_removed == 1)     //当算例中所有子句均已被移除，则无子句存在
		{
			Cl = Cl->next;
			continue;
		}
		else
			return 0;
	}
	return 1;
}

void unitPropagation(Dimacs* data, int ID)    //ID为单子句的文字
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
		else if (Cl->bool_removed == 0)      //首先在未移除子句中寻找与传入ID相关的文字进行相应的赋值
		{
			Li = Cl->head;
			while (Li != NULL)
			{
				if (Li->ID == ID)
				{
					Li->bool = 1;
					Cl->bool_removed = 1;   //bool为1相当于移除该子句
					save_removed_Cl(data, Cl);
					Li = Li->next;
				}
				else if (Li->ID == -ID)
				{
					Li->bool = 0;			//bool为0相当于移除该文字
					Cl->unsatisfied_literal++;
					Li = Li->next;
				}
				else
					Li = Li->next;
			}
			Cl = Cl->next;
		}
	}
	save_booled_Li(data, ID, 1);    //将该ID赋真之后保存在赋值结果链表中
}

int select_mostID(Dimacs* data)
{
	int maxlength = 0;
	int most_ID = 0;
	int num;
	ID_occurs_List* head = data->ID_list_root;
	while (head != NULL)             //基于文字在子句中的出现频率（length) 的高低选取文字赋值
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

void save_removed_Cl(Dimacs* data, Clause* Cl)    //保存在之前已经remove的字句指针，以便之后回溯恢复bool_removed值
{
	removed_Cl* last_rmv = (removed_Cl*)malloc(sizeof(removed_Cl));
	last_rmv->removed_level = level;
	last_rmv->Cl = Cl;
	last_rmv->next = data->lastlevel_Cl_root;
	data->lastlevel_Cl_root = last_rmv;
	data->removed_Cl_num++;
}

void save_booled_Li(Dimacs* data, int ID, int bool)  //保存在之前已经赋值的文字ID，以便之后回溯恢复bool值
{
	Pair* newPair = (Pair*)malloc(sizeof(Pair));
	newPair->assigned_level = level;
	newPair->ID = ID;
	newPair->bool = bool;
	newPair->next = data->value_pair_root;
	data->value_pair_root = newPair;
	data->valued_number++;
}

void Assign_True(Dimacs* data, int ID)    //ID赋真，类似于单子句传播策略过程
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


void Assign_False(Dimacs* data, int ID)     //ID赋假，执行逻辑与Assign_True恰好相反，含有-ID的子句直接移除，值为ID的文字bool为0，移除文字
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

int find_level(Dimacs* data, int select_ID)           //在value_pair_root指向的赋值结果链表中寻找特定ID，返回其被赋值所在层数
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

int find_if_valued(Dimacs* data, int most_ID)          //查找most_ID或-most_ID是否已被赋值过
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

void Go_back(Dimacs* data)             //回溯核心，采用修改标记、更新赋值结果存储链表的方法
{
	removed_Cl* backtrack = data->lastlevel_Cl_root;
	removed_Cl* temp;
	while (backtrack != NULL && backtrack->removed_level >= level)     //当前level之前移除的子句bool_removed标记改回0，并进行free
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
	while (backLi != NULL && backLi->assigned_level >= level)          //遍历子句，遍历每一个文字，当前level之前赋值的文字ID改回原bool值，并进行free
	{
		Clause* Cl = data->root;
		Literal* Li;
		while (Cl != NULL)
		{
			if (Cl->bool_removed == 1)     //表示在level之前已经移除，不做讨论
			{
				Cl = Cl->next;
				continue;
			}
			if (Cl->bool_removed == 0)     //在未移除的子句里面找ID相同的
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


