#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#define TRUE 1
#define FALSE 0
#define UNCERTAIN -1

int DPLL(Dimacs* data);
int findUnitClause(Dimacs* data);
int isUnitClause(Clause* Cl);
int isEmptyClause(Clause* Cl);
int haveEmptyClause(Dimacs* data);
int have_no_clause(Dimacs* data);
void unitPropagation(Dimacs* data, int ID);
int select_mostID(Dimacs* data);
void save_removed_Cl(Dimacs* data, Clause* Cl);
void save_booled_Li(Dimacs* data, int ID,int bool);
void Assign_True(Dimacs* data, int ID);
void Assign_False(Dimacs* data, int ID);
void Go_back(Dimacs* data);
int find_level(Dimacs* data, int select_ID);
int find_if_valued(Dimacs* data, int most_ID);

