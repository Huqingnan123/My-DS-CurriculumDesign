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
char board[12][12];            //存储初始数独格局
int result_board[12][12];	   //存储数独求解格局
char sudoku_res_filename[50];

char line_or_col_array[12];
char results[7];               //存储一次组合选取的结果
int results_end = 0;

int extra_var_num;
int ci[500];                    //进行Tseytin变换时的附加变量存储数组
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
			fscanf(input, "%c", &board[i][j]);       //从文件中读取格局存储到board中
			if (j == order-1)
				fscanf(input, "%c", &ch);
		}
	}
	printf("\nBegin Solving!\n");
	printf("\n");
	for (print_number = 0; print_number < 2 * order + 5; print_number++)        //以下在控制台中打印数独初始格局
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
			if (j % (order/2) == 0)                             //计算格式距离要求
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


void Generate_Dimacs(char board[][12],char* sudoku_filename)   //根据初始条件以及三个约束条件向suduko_data总结构体中添加子句信息
{
	int i, j,number,row,col;
	for (number = 0; sudoku_filename[number] != '.'; number++)
		sudoku_res_filename[number] = sudoku_filename[number];
	sudoku_res_filename[number++] = '.';
	sudoku_res_filename[number++] = 'r';
	sudoku_res_filename[number++] = 'e';
	sudoku_res_filename[number++] = 's';
	sudoku_res_filename[number++] = '\0';                      //res文件同名复制

	sudoku_data = (Dimacs*)malloc(sizeof(Dimacs));             //数独信息结构体suduko_data初始化
	sudoku_data->varnum = order * order;
	sudoku_data->clanum = 0;
	sudoku_data->removed_Cl_num = 0;
	sudoku_data->lastlevel_Cl_root = NULL;
	sudoku_data->value_pair_root = NULL;
	sudoku_data->root = NULL;
	sudoku_data->valued_number = 0;
	sudoku_data->ID_list_root = NULL;

	//先将棋盘中的已有文字信息作为单子句录入
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

	//第一约束条件，每行每列不能连续3个1或3个0
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

	//第二约束条件，每行或每列中1与0的个数相同，即一行或一列中order个数中任意(order/2+1)个数至少有1个0或1个1（要用组合选取）
	for (row = 0; row < order; row++)
	{
		for (col = 0; col < order; col++)
			line_or_col_array[col] = col;
		Combine_select_row(row, 0, order, order / 2 + 1);   //在每一行中选取order/2+1个单元
		results_end = 0;
	}
	for (col = 0; col < order; col++)
	{
		for (row = 0; row < order; row++)
			line_or_col_array[row] = row;
		Combine_select_col(col, 0, order, order / 2 + 1);   //在每一列中选取order/2+1个单元
		results_end = 0;
	}

	//第三约束条件，不存在相同的某两行或者某两列，添加附加变元ai,bi,ci,di进行间接表示
	int row1, row2, col1, col2;
	extra_var_num = order * order + 1;                      //附加变元编号依次往后顺移
	for (row1 = 0; row1 < order - 1; row1++)
	{
		for (row2 = row1 + 1; row2 < order; row2++)
		{
			di_for_two_RowOrCol = extra_var_num++;
			for (col = 0; col < order; col++)
			{
				add_extraCluase_row(row1, row2, col, temp_clause);      //对于选中的两行row1和row2,col从0-order进行对应附加变元的添加
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
			addClause(sudoku_data, 1 , temp_clause);         //最后进行di和当前一轮下来ci的合取添加子句
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
				add_extraCluase_col(col1, col2, row, temp_clause);     //对于选中的两列col1和col2,row从0-order进行对应附加变元的添加
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
			addClause(sudoku_data, 1, temp_clause);       //最后进行di和当前一轮下来ci的合取添加子句
			sudoku_data->clanum++;
		}
	}
}

//在某一行中进行随机组合选取
void Combine_select_row(int row, int selected_num, int n, int m)
{
	int temp_clause[24];
	int i = 0;
	if (selected_num > n) // 越界递归结束
		return;
	if (results_end == m) // 已选出order/2+1个单元的列号，可以添加子句
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
	Combine_select_row(row, selected_num + 1, n, m); //向下一级递归选取
	results_end--;
	Combine_select_row(row, selected_num + 1, n, m); //向下一级递归选取
}

//在某一列中进行随机组合选取
void Combine_select_col(int col, int selected_num, int n, int m)
{
	int temp_clause[24];
	int i = 0;
	if (selected_num > n) // 越界递归结束
		return;
	if (results_end == m) // 已选出order/2+1个单元的行号，可以添加子句
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
	Combine_select_col(col, selected_num + 1, n, m); //向下一级递归选取
	results_end--;
	Combine_select_col(col, selected_num + 1, n, m); //向下一级递归选取
}

//顺次组合选取两行来添加附加变元
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
	extra_var_num++;                       //对于ai,每个ai可以与两个原有变元生成三个新子句

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
	extra_var_num++;                        //对于bi，每个bi可以与两个原有变元生成三个新子句

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
	extra_var_num++;                          //对于ci，每个ci可以与之前的两个ai,bi生成三个新子句

	temp_clause[0] = extra_var_num - 1;
	temp_clause[1] = di_for_two_RowOrCol;
	addClause(sudoku_data, 2, temp_clause);    //对于di,先添加当前di和前一个ci生成一个新子句
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
	extra_var_num++;                       //对于ai,每个ai可以与两个原有变元生成三个新子句

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
	extra_var_num++;                        //对于bi，每个bi可以与两个原有变元生成三个新子句

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
	extra_var_num++;                          //对于ci，每个ci可以与之前的两个ai,bi生成三个新子句

	temp_clause[0] = extra_var_num - 1;
	temp_clause[1] = di_for_two_RowOrCol;
	addClause(sudoku_data, 2, temp_clause);    //对于di,先添加当前di和前一个ci生成一个新子句
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
		Output_result_puzzle(sudoku_res_filename);       //数独有解时，进行格局输出
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
		while ((ch = fgetc(input)) != '\n');    //过滤掉res文件中的第一行信息
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
	for (print_number = 0; print_number < 2 * order + 5; print_number++)   //以下在控制台中打印数独最终格局
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
			if (result_board[i][j] < 0 )     //最终结果每个单为0或1，表示该单元变元取真或取假
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
