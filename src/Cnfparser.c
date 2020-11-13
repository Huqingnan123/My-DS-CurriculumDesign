#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "cnfparser.h"
#include "solver.h"
#include "BinaryPuzzle.h"



Dimacs* Read_cnf(char* filename)
{
	FILE* input = fopen(filename, "r");
	char ch;
	int line[256];
	Dimacs* data = (Dimacs*)malloc(sizeof(Dimacs));     //Dimacs�ṹ�������������������Ϣ
	if (input == NULL)
	{
		printf("open ERROR!");
		getchar();
		exit(1);
	}
	while ((ch = fgetc(input)) == 'c')
		while ((ch = fgetc(input)) != '\n');			//����cnf�ļ��е�ע�ͺ�"p cnf"
	for (int i = 0; i < 5; i++)
		ch = fgetc(input);
	fscanf(input, "%d", &(data->varnum));
	fscanf(input, "%d", &(data->clanum));
	data->removed_Cl_num = 0;                            //Dimacs�ṹ���Ա��ʼ��
	data->lastlevel_Cl_root = NULL;
	data->value_pair_root = NULL;
	data->root = NULL;
	data->valued_number = 0;
	data->ID_list_root = NULL;
	for (int i = 0; i < data->clanum; i++)
	{
		int j = 0;
		do
		{
			fscanf(input, "%d", &line[j]);                //һ��һ�ж�ȡ�Ӿ��е����֣�����0Ϊֹ
			j++;
		} while (line[j-1] != 0);
		addClause(data, j-1, line);
	}
	fclose(input);
	FILE* Verify_file = fopen("Verify_cnf_file.txt", "w");     //������֤�������Ӿ���Ϣ����Dimacs�ṹ�����ȷ��
	Clause* Cl = data->root;
	while (Cl->next != NULL)
		Cl = Cl->next;
	while (Cl->prev != NULL)
	{
		Literal* Li = Cl->head;
		while (Li != NULL)
		{
			fprintf(Verify_file, "%d ", Li->ID);
			Li = Li->next;
		}
		fprintf(Verify_file, "0\n");
		Cl = Cl->prev;
	}
	fclose(Verify_file);
	return data;
}

void addClause(Dimacs* data, int literal_num, int* line)         //��Dimacs�ܽṹ��������Ӿ�
{
	Clause* newclause = (Clause*)malloc(sizeof(Clause));
	newclause->head = NULL;
	newclause->bool_removed = 0;
	newclause->length = 0;
	newclause->prev= NULL;
	newclause->next = NULL;
	newclause->unsatisfied_literal = 0;
	for (int i = 0; i < literal_num; i++)
		addliteral(newclause,line[i],data);                      //ѭ����ǰ�Ӿ����������
	if (data->root)
		data->root->prev = newclause;
	newclause->next = data->root;
	newclause->prev = NULL;  
	data->root = newclause;				//�����¼ÿһ���Ӿ�
}

void addliteral(Clause* clause, int ID, Dimacs* data) 
{
	Literal* newLiteral = (Literal*)malloc(sizeof(Literal));
	newLiteral->bool = UNCERTAIN;
	newLiteral->ID = ID;
	newLiteral->next = clause->head;
	clause->head = newLiteral;
	clause->length++;               //�����¼ÿһ������
	add_ID_list_length(data, ID);
}

void add_ID_list_length(Dimacs* data, int ID)         //��¼���������ֵ�ID����ID_list
{
	ID_occurs_List* temp = data->ID_list_root;
	ID_occurs_List* prev = data->ID_list_root;
	if (data->ID_list_root == NULL)
	{
		data->ID_list_root = (ID_occurs_List*)malloc(sizeof(ID_occurs_List));
		data->ID_list_root->ID = ID;
		data->ID_list_root->relevent_Cl_length = 1;
		data->ID_list_root->next = NULL;
	}
	else
	{
		while (temp != NULL)
		{
			if (temp->ID == ID)          //����ID�Ѵ�����ID_list�У����ID����+1�������¿���һ����Ԫ�洢��ID�ĳ��ִ��������Ժ���select����
			{
				temp->relevent_Cl_length++;
				break;
			}
			else
			{
				prev = temp;
				temp = temp->next;
			}
		}
		if (temp == NULL)
		{
			temp = (ID_occurs_List*)malloc(sizeof(ID_occurs_List));
			temp->ID = ID;
			temp->relevent_Cl_length = 1;
			prev->next = temp;
			temp->next = NULL;
		}
	}
}

