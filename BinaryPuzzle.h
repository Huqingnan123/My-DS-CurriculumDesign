#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#define TRUE 1
#define FALSE 0
#define UNCERTAIN -1

void Binary_Puzzle(char* sudoku_filename);
void Generate_Dimacs(char board[][12],char* sudoku_filename);
void Combine_select_row(int row, int selected_num, int n, int m);
void Combine_select_col(int col, int selected_num, int n, int m);
void add_extraCluase_row(int row1, int row2, int col, int* temp_clause);
void add_extraCluase_col(int col1, int col2, int row, int* temp_clause);
void PrintSolution();
extern void Output_result(char* filename, int is_satisfy, Dimacs* data, double runtime);
void Output_result_puzzle(char* sudoku_res_filename);