#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "cnfparser.h"
#include "solver.h"
#include "BinaryPuzzle.h"

int is_satisfy = 0;
int order = 0;

void Sudoku();
void SAT();
void Output_result(char* filename, int is_satisfy, Dimacs* data, double runtime);
int find_TRUE_OR_FALSE(Dimacs* data, int ID);

int main()
{
	int option;
	while (1)
	{
		system("cls");
		printf("\n\n\t\t\t\t\t----------------------------\n");
		printf("\t\t\t\t\t|                          |\n");
		printf("\t\t\t\t\t|     Sudoku or SAT ?      |\n");
		printf("\t\t\t\t\t|                          |\n");
		printf("\t\t\t\t\t|     enter 1 to SAT.      |\n");
		printf("\t\t\t\t\t|                          |\n");
		printf("\t\t\t\t\t|     enter 2 to Sudoku.   |\n");
		printf("\t\t\t\t\t|                          |\n");
		printf("\t\t\t\t\t|     enter 0 to quit.     |\n");
		printf("\t\t\t\t\t|                          |\n");
		printf("\t\t\t\t\t----------------------------\n\n");
		printf("Please enter your option:");
		input:scanf("%d", &option);
		if (option != 1 && option != 2 && option != 0)
		{
			printf("Invalid input!Input again!\n");
			goto input;
		}
		if (option == 1)			//选择进入不同模块
			SAT();
		if (option == 2)
			Sudoku();
		if (option == 0)
			exit(1);
	}
	return 0;
}

void SAT()
{
	char cnf_filename[50], output_filename[50];
	printf("Please input your CNF format filename: ");
	scanf("%s", cnf_filename);
	Dimacs* data = Read_cnf(cnf_filename);
	strcpy(output_filename,cnf_filename);					//res文件同名复制
	int i;
	for (i = 0; output_filename[i] != '\0'; i++);
	output_filename[--i] = 's';
	output_filename[--i] = 'e';
	output_filename[--i] = 'r';
	printf("Solving..........\n");
	clock_t start, end;
	double runtime;
	start = clock();
	is_satisfy = DPLL(data);                                //DPLL核心算法求解算例
	end = clock();
	runtime = (double)(end-start)*CLOCKS_PER_SEC/1000;      //统计求解时间
	Output_result(output_filename, is_satisfy, data, runtime);
	printf("\nFinished........\n");
	printf("Please open %s file to get the result.\n",output_filename);
	getchar();
	getchar();
}

void Sudoku()
{
	char sudoku_filename[50], cnf_filename[50], output_filename[50];
	printf("Please input the order of your binarypuzzle: ");
	scanf("%d", &order);									//输入数独阶数
	getchar();
	printf("Please input your binarypuzzle filename: ");
	scanf("%s", sudoku_filename);
	Binary_Puzzle(sudoku_filename);
	getchar();
}

void Output_result(char* filename, int is_satisfy, Dimacs* data, double runtime)
{
	FILE* output = fopen(filename, "w");
	if(is_satisfy==1)
	{ 
		fprintf(output, "s 1\n");
		fprintf(output, "v ");
		int i;
		for (i = 1; i <= data->varnum; i++)         
		{
			Pair* temp = data->value_pair_root;				//在赋值结果存储链表中寻找特定变元
			while (temp != NULL)
			{
				if (abs(temp->ID) == i)
				{
					if (temp->ID < 0 && temp->bool == 0)   //最后输出10表示10号变元赋值为真，输出-10表示10号变元赋值为假
					{
						fprintf(output, "%d ", abs(temp->ID));
						break;
					}
					if (temp->ID < 0 && temp->bool == 1)
					{
						fprintf(output, "%d ", temp->ID);
						break;
					}
					if (temp->ID > 0 && temp->bool == 0)
					{
						fprintf(output, "-%d ", temp->ID);
						break;
					}
					if (temp->ID > 0 && temp->bool == 1)
					{
						fprintf(output, "%d ", temp->ID);
						break;
					}
				}
				else
					temp = temp->next;
			}
		}
		fprintf(output, "\n");
	}
	else
		fprintf(output,"s 0\n");
	fprintf(output, "t %.2lf ms\n", runtime);
	fprintf(output, "finished after %d levels assignment\n", data->value_pair_root->assigned_level);
	fclose(output);

	FILE* Verify_file2 = fopen("Verify_res_file.txt", "w");    //遍历每一个子句中的每一个文字，输出它们的赋值结果True or Flase,用来验证算例可满足
	Clause* Cl = data->root;
	while (Cl->next != NULL)
		Cl = Cl->next;
	while (Cl->prev != NULL)
	{
		Literal* Li = Cl->head;
		fprintf(Verify_file2, "( ");
		while (Li != NULL)
		{
			if(find_TRUE_OR_FALSE(data,Li->ID))     
				fprintf(Verify_file2, "True ");
			else
				fprintf(Verify_file2, "False ");
			Li = Li->next;
		}
		fprintf(Verify_file2, ")\n");
		Cl = Cl->prev;
	}
	fclose(Verify_file2);
}

int find_TRUE_OR_FALSE(Dimacs* data, int ID)      //传入文字ID到data中的赋值结果链表中寻找其值True or Flase
{
	Pair* temp = data->value_pair_root;
	while (temp != NULL)
	{
		if (temp->ID == ID)
		{
			if (temp->bool == 1)
			{
				return 1;
			}
			if (temp->bool == 0)
			{
				return 0;
			}
		}
		if (temp->ID == -ID)
		{
			if (temp->bool == 1)
			{
				return 0;
			}
			if (temp->bool == 0)
			{
				return 1;
			}
		}
		temp = temp->next;
	}
	return 0;
}

