#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "Cnfparser.h"
#include "Solver.h"
#include "BinaryPuzzle.h"

extern int order;    
Dimacs* sudoku_data;
int temp_clause[24];
char board[12][12];            //�洢��ʼ�������
int result_board[12][12];	   //�洢���������
char sudoku_res_filename[50];

char line_or_col_array[12];
char results[7];               //�洢һ�����ѡȡ�Ľ��
int results_end = 0;

int extra_var_num;
int ci[500];                    //����Tseytin�任ʱ�ĸ��ӱ����洢����
int ci_num = 0;
int di_for_two_RowOrCol;

void Binary_Puzzle(char* sudoku_filename)
{
	FILE* input = fopen(sudoku_filename, "r");
	if (input == NULL)
	{
		printf("open ERROR!\n");
		exit(1);
	}
	int i, j,print_number;
	char ch;
	for (i = 0; i < order; i++)
	{
		for (j = 0; j < order; j++)
		{
			fscanf(input, "%c", &board[i][j]);       //���ļ��ж�ȡ��ִ洢��board��
			if (j == order-1)
				fscanf(input, "%c", &ch);
		}
	}
	printf("\nBegin Solving!\n");
	printf("\n");
	for (print_number = 0; print_number < 2 * order + 5; print_number++)        //�����ڿ���̨�д�ӡ������ʼ���
		printf("-");
	printf("\n");
	for (i = 0; i < order; i++)
	{
		if (i == order / 2)
		{
			printf("|");
			for (print_number = 0; print_number < 2 * order + 3; print_number++)
				printf("-");
			printf("|\n");
		}
		for (j = 0; j < order; j++)
		{
			if (j % (order/2) == 0)                             //�����ʽ����Ҫ��
				printf("| ");
			if (board[i][j] == '#')
				printf("_ ");
			if (board[i][j] == '0' || board[i][j] == '1')
				printf("%c ", board[i][j]);
		}
		printf("|\n");
	}
	for (print_number = 0; print_number < 2 * order + 5; print_number++)
		printf("-");
	printf("\n");
	fclose(input);
	Generate_Dimacs(board,sudoku_filename);          
	sudoku_data->varnum += (3 * order + 1) * order * (order - 1);
	PrintSolution();
}


void Generate_Dimacs(char board[][12],char* sudoku_filename)   //���ݳ�ʼ�����Լ�����Լ��������suduko_data�ܽṹ��������Ӿ���Ϣ
{
	int i, j,number,row,col;
	for (number = 0; sudoku_filename[number] != '.'; number++)
		sudoku_res_filename[number] = sudoku_filename[number];
	sudoku_res_filename[number++] = '.';
	sudoku_res_filename[number++] = 'r';
	sudoku_res_filename[number++] = 'e';
	sudoku_res_filename[number++] = 's';
	sudoku_res_filename[number++] = '\0';                      //res�ļ�ͬ������

	sudoku_data = (Dimacs*)malloc(sizeof(Dimacs));             //������Ϣ�ṹ��suduko_data��ʼ��
	sudoku_data->varnum = order * order;
	sudoku_data->clanum = 0;
	sudoku_data->removed_Cl_num = 0;
	sudoku_data->lastlevel_Cl_root = NULL;
	sudoku_data->value_pair_root = NULL;
	sudoku_data->root = NULL;
	sudoku_data->valued_number = 0;
	sudoku_data->ID_list_root = NULL;

	//�Ƚ������е�����������Ϣ��Ϊ���Ӿ�¼��
	for (i = 0; i < order; i++)
	{
		for (j = 0; j < order; j++)
		{
			if (board[i][j] == '0')
			{
				temp_clause[0] = -(order * i + j + 1);
				addClause(sudoku_data, 1, temp_clause);
				sudoku_data->clanum++;
			}
			if (board[i][j] == '1')
			{
				temp_clause[0] = (order * i + j + 1);
				addClause(sudoku_data, 1, temp_clause);
				sudoku_data->clanum++;
			}
		}
	}

	//��һԼ��������ÿ��ÿ�в�������3��1��3��0
	for (row = 0; row < order; row++)
	{
		for (col = 0; col < order - 2; col++)
		{
			temp_clause[0] = order * row + col + 1;
			temp_clause[1] = order * row + col + 2;
			temp_clause[2] = order * row + col + 3;
			addClause(sudoku_data, 3, temp_clause);
			sudoku_data->clanum++;
			temp_clause[0] = -(order * row + col + 1);
			temp_clause[1] = -(order * row + col + 2);
			temp_clause[2] = -(order * row + col + 3);
			addClause(sudoku_data, 3, temp_clause);
			sudoku_data->clanum++;
		}
	}
	for (col = 0; col < order; col++)
	{
		for (row = 0; row < order - 2; row++)
		{
			temp_clause[0] = order * row + col + 1;
			temp_clause[1] = order * row + col + order+1;
			temp_clause[2] = order * row + col + 2*order + 1;
			addClause(sudoku_data, 3, temp_clause);
			sudoku_data->clanum++;
			temp_clause[0] = -(order * row + col + 1);
			temp_clause[1] = -(order * row + col + order + 1);
			temp_clause[2] = -(order * row + col + 2*order + 1);
			addClause(sudoku_data, 3, temp_clause);
			sudoku_data->clanum++;
		}
	}

	//�ڶ�Լ��������ÿ�л�ÿ����1��0�ĸ�����ͬ����һ�л�һ����order����������(order/2+1)����������1��0��1��1��Ҫ�����ѡȡ��
	for (row = 0; row < order; row++)
	{
		for (col = 0; col < order; col++)
			line_or_col_array[col] = col;
		Combine_select_row(row, 0, order, order / 2 + 1);   //��ÿһ����ѡȡorder/2+1����Ԫ
		results_end = 0;
	}
	for (col = 0; col < order; col++)
	{
		for (row = 0; row < order; row++)
			line_or_col_array[row] = row;
		Combine_select_col(col, 0, order, order / 2 + 1);   //��ÿһ����ѡȡorder/2+1����Ԫ
		results_end = 0;
	}

	//����Լ����������������ͬ��ĳ���л���ĳ���У���Ӹ��ӱ�Ԫai,bi,ci,di���м�ӱ�ʾ
	int row1, row2, col1, col2;
	extra_var_num = order * order + 1;                      //���ӱ�Ԫ�����������˳��
	for (row1 = 0; row1 < order - 1; row1++)
	{
		for (row2 = row1 + 1; row2 < order; row2++)
		{
			di_for_two_RowOrCol = extra_var_num++;
			for (col = 0; col < order; col++)
			{
				add_extraCluase_row(row1, row2, col, temp_clause);      //����ѡ�е�����row1��row2,col��0-order���ж�Ӧ���ӱ�Ԫ�����
			}
			for (ci_num = 0; ci_num < order; ci_num++)
			{
				temp_clause[ci_num] = -ci[ci_num];
			}
			ci_num = 0;
			temp_clause[order] = -di_for_two_RowOrCol;
			addClause(sudoku_data, order + 1, temp_clause);
			sudoku_data->clanum++;
			temp_clause[0] = di_for_two_RowOrCol;
			addClause(sudoku_data, 1 , temp_clause);         //������di�͵�ǰһ������ci�ĺ�ȡ����Ӿ�
			sudoku_data->clanum++;
		}
	}
	for (col1 = 0; col1 < order - 1; col1++)
	{
		for (col2 = col1 + 1; col2 < order; col2++)
		{
			di_for_two_RowOrCol = extra_var_num++;
			for (row = 0; row < order; row++)
			{
				add_extraCluase_col(col1, col2, row, temp_clause);     //����ѡ�е�����col1��col2,row��0-order���ж�Ӧ���ӱ�Ԫ�����
			}
			for (ci_num = 0; ci_num < order; ci_num++)
			{
				temp_clause[ci_num] = -ci[ci_num];
			}
			ci_num = 0;
			temp_clause[order] = -di_for_two_RowOrCol;
			addClause(sudoku_data, order + 1, temp_clause);
			sudoku_data->clanum++;
			temp_clause[0] = di_for_two_RowOrCol;
			addClause(sudoku_data, 1, temp_clause);       //������di�͵�ǰһ������ci�ĺ�ȡ����Ӿ�
			sudoku_data->clanum++;
		}
	}
}

//��ĳһ���н���������ѡȡ
void Combine_select_row(int row, int selected_num, int n, int m)
{
	int temp_clause[24];
	int i = 0;
	if (selected_num > n) // Խ��ݹ����
		return;
	if (results_end == m) // ��ѡ��order/2+1����Ԫ���кţ���������Ӿ�
	{
		for (i = 0; i < m; i++)
			temp_clause[i] = order * row + results[i] + 1;
		addClause(sudoku_data, m, temp_clause);
		sudoku_data->clanum++;
		for (i = 0; i < m; i++)
			temp_clause[i] = -(order * row + results[i] + 1);
		addClause(sudoku_data, m, temp_clause);
		sudoku_data->clanum++;
		return;
	}
	results[results_end++] = line_or_col_array[selected_num];
	Combine_select_row(row, selected_num + 1, n, m); //����һ���ݹ�ѡȡ
	results_end--;
	Combine_select_row(row, selected_num + 1, n, m); //����һ���ݹ�ѡȡ
}

//��ĳһ���н���������ѡȡ
void Combine_select_col(int col, int selected_num, int n, int m)
{
	int temp_clause[24];
	int i = 0;
	if (selected_num > n) // Խ��ݹ����
		return;
	if (results_end == m) // ��ѡ��order/2+1����Ԫ���кţ���������Ӿ�
	{
		for (i = 0; i < m; i++)
			temp_clause[i] = order * results[i] + col + 1;
		addClause(sudoku_data, m, temp_clause);
		sudoku_data->clanum++;
		for (i = 0; i < m; i++)
			temp_clause[i] = -(order * results[i] + col + 1);
		addClause(sudoku_data, m, temp_clause);
		sudoku_data->clanum++;
		return;
	}
	results[results_end++] = line_or_col_array[selected_num];
	Combine_select_col(col, selected_num + 1, n, m); //����һ���ݹ�ѡȡ
	results_end--;
	Combine_select_col(col, selected_num + 1, n, m); //����һ���ݹ�ѡȡ
}

//˳�����ѡȡ��������Ӹ��ӱ�Ԫ
void add_extraCluase_row(int row1, int row2, int col, int* temp_clause)
{
	temp_clause[0] = -extra_var_num;
	temp_clause[1] = order * row1 + col + 1;
	addClause(sudoku_data, 2, temp_clause);
	sudoku_data->clanum++;
	temp_clause[0] = -extra_var_num;
	temp_clause[1] = order * row2 + col + 1;
	addClause(sudoku_data, 2, temp_clause);
	sudoku_data->clanum++;
	temp_clause[0] = extra_var_num;
	temp_clause[1] = -(order * row1 + col + 1);
	temp_clause[2] = -(order * row2 + col + 1);
	addClause(sudoku_data, 3, temp_clause);
	sudoku_data->clanum++;
	extra_var_num++;                       //����ai,ÿ��ai����������ԭ�б�Ԫ�����������Ӿ�

	temp_clause[0] = -extra_var_num;
	temp_clause[1] = -(order * row1 + col + 1);
	addClause(sudoku_data, 2, temp_clause);
	sudoku_data->clanum++;
	temp_clause[0] = -extra_var_num;
	temp_clause[1] = -(order * row2 + col + 1);
	addClause(sudoku_data, 2, temp_clause);
	sudoku_data->clanum++;
	temp_clause[0] = extra_var_num;
	temp_clause[1] = order * row1 + col + 1;
	temp_clause[2] = order * row2 + col + 1;
	addClause(sudoku_data, 3, temp_clause);
	sudoku_data->clanum++;
	extra_var_num++;                        //����bi��ÿ��bi����������ԭ�б�Ԫ�����������Ӿ�

	temp_clause[0] = extra_var_num;
	temp_clause[1] = -(extra_var_num - 2);
	addClause(sudoku_data, 2, temp_clause);
	sudoku_data->clanum++;
	temp_clause[0] = extra_var_num;
	temp_clause[1] = -(extra_var_num - 1);
	addClause(sudoku_data, 2, temp_clause);
	sudoku_data->clanum++;
	temp_clause[0] = -(extra_var_num);
	temp_clause[1] = extra_var_num - 2;
	temp_clause[2] = extra_var_num - 1;
	addClause(sudoku_data, 3, temp_clause);
	sudoku_data->clanum++;
	ci[ci_num++] = extra_var_num;
	extra_var_num++;                          //����ci��ÿ��ci������֮ǰ������ai,bi�����������Ӿ�

	temp_clause[0] = extra_var_num - 1;
	temp_clause[1] = di_for_two_RowOrCol;
	addClause(sudoku_data, 2, temp_clause);    //����di,����ӵ�ǰdi��ǰһ��ci����һ�����Ӿ�
	sudoku_data->clanum++;
}

void add_extraCluase_col(int col1, int col2, int row, int* temp_clause)
{
	temp_clause[0] = -extra_var_num;
	temp_clause[1] = order * row + col1 + 1;
	addClause(sudoku_data, 2, temp_clause);
	sudoku_data->clanum++;
	temp_clause[0] = -extra_var_num;
	temp_clause[1] = order * row + col2 + 1;
	addClause(sudoku_data, 2, temp_clause);
	sudoku_data->clanum++;
	temp_clause[0] = extra_var_num;
	temp_clause[1] = -(order * row + col1 + 1);
	temp_clause[2] = -(order * row + col2 + 1);
	addClause(sudoku_data, 3, temp_clause);
	sudoku_data->clanum++;
	extra_var_num++;                       //����ai,ÿ��ai����������ԭ�б�Ԫ�����������Ӿ�

	temp_clause[0] = -extra_var_num;
	temp_clause[1] = -(order * row + col1 + 1);
	addClause(sudoku_data, 2, temp_clause);
	sudoku_data->clanum++;
	temp_clause[0] = -extra_var_num;
	temp_clause[1] = -(order * row + col2 + 1);
	addClause(sudoku_data, 2, temp_clause);
	sudoku_data->clanum++;
	temp_clause[0] = extra_var_num;
	temp_clause[1] = (order * row + col1 + 1);
	temp_clause[2] = (order * row + col2 + 1);
	addClause(sudoku_data, 3, temp_clause);
	sudoku_data->clanum++;
	extra_var_num++;                        //����bi��ÿ��bi����������ԭ�б�Ԫ�����������Ӿ�

	temp_clause[0] = extra_var_num;
	temp_clause[1] = -(extra_var_num - 2);
	addClause(sudoku_data, 2, temp_clause);
	sudoku_data->clanum++;
	temp_clause[0] = extra_var_num;
	temp_clause[1] = -(extra_var_num - 1);
	addClause(sudoku_data, 2, temp_clause);
	sudoku_data->clanum++;
	temp_clause[0] = -extra_var_num;
	temp_clause[1] = extra_var_num - 2;
	temp_clause[2] = extra_var_num - 1;
	addClause(sudoku_data, 3, temp_clause);
	sudoku_data->clanum++;
	ci[ci_num++] = extra_var_num;
	extra_var_num++;                          //����ci��ÿ��ci������֮ǰ������ai,bi�����������Ӿ�

	temp_clause[0] = extra_var_num - 1;
	temp_clause[1] = di_for_two_RowOrCol;
	addClause(sudoku_data, 2, temp_clause);    //����di,����ӵ�ǰdi��ǰһ��ci����һ�����Ӿ�
	sudoku_data->clanum++;
}

void PrintSolution()
{
	int is_satisfy;
	printf("Solving..........\n");
	clock_t start, end;
	double runtime;
	start = clock();
	is_satisfy = DPLL(sudoku_data);
	end = clock();
	runtime = (double)(end - start) * CLOCKS_PER_SEC / 1000;
	Output_result(sudoku_res_filename, is_satisfy, sudoku_data, runtime);
	if (is_satisfy == 1)
		Output_result_puzzle(sudoku_res_filename);       //�����н�ʱ�����и�����
	else
	{
		printf("\nNo result!\n");
		getchar();
	}
	printf("\nFinished........\n");
}

void Output_result_puzzle(char* sudoku_res_filename)
{
	FILE* input = fopen(sudoku_res_filename, "r");
	if (input == NULL)
	{
		printf("open ERROR!\n");
		exit(1);
	}
	char ch;
	while ((ch = fgetc(input)) == 's')
		while ((ch = fgetc(input)) != '\n');    //���˵�res�ļ��еĵ�һ����Ϣ
	ch = fgetc(input);
	int i, j;
	for (i = 0; i < order; i++)
	{
		for (j = 0; j < order; j++)
		{
			fscanf(input, "%d", &result_board[i][j]);      
			if (j == order - 1)
				fscanf(input, "%c", &ch);
		}
	}
	int print_number;
	printf("\n");
	for (print_number = 0; print_number < 2 * order + 5; print_number++)   //�����ڿ���̨�д�ӡ�������ո��
		printf("-");
	printf("\n");
	for (i = 0; i < order; i++)
	{
		if (i == order / 2)
		{
			printf("|");
			for (print_number = 0; print_number < 2 * order + 3; print_number++)
				printf("-");
			printf("|\n");
		}
		for (j = 0; j < order; j++)
		{
			if (j % (order/2) == 0)
				printf("| ");
			if (result_board[i][j] < 0 )     //���ս��ÿ����Ϊ0��1����ʾ�õ�Ԫ��Ԫȡ���ȡ��
				printf("0 ");
			if (result_board[i][j] > 0)
				printf("1 ");
		}
		printf("|\n");
	}
	for (print_number = 0; print_number < 2 * order + 5; print_number++)
		printf("-");
	printf("\n");
	fclose(input);
	getchar();
}
