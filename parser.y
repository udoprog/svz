/* Supervize argument parser */

%{
#include <stdio.h>
#include <stdlib.h>
#include "supervize.h"
#include "stack.h"
#include "frun.h"
%}

%pure-parser

%union {
  int proc;
  int string;
  int array;
}

%token ECHO EXEC SPAWN PID WHEN DO ELSE AND OR NOT ARGEND
%token <string> ARGUMENT
%type <proc> stmt run_stmt
%type <array> argv

%left AND
%left OR
%left NOT

%% /* Grammar rules and actions follow */
exp: stmt DO stmt ELSE stmt {sv_root(g_cs, proc_run(g_cs, $1), $3, $5);}
exp: stmt DO stmt           {sv_root(g_cs, proc_run(g_cs, $1), $3, -1);}
exp: stmt                   {sv_root(g_cs, proc_run(g_cs, $1), -1, -1);}

stmt: NOT stmt      {$$ = proc_append(g_cs, sv_stmt_not, 1, $2);}
    | run_stmt
    | '{' stmt '}'  {$$ = $2}
    | stmt AND stmt {$$ = proc_append(g_cs, sv_stmt_and, 2, $1, $3);}
    | stmt OR stmt  {$$ = proc_append(g_cs, sv_stmt_or, 2, $1, $3);}
;

run_stmt: ARGUMENT argv ARGEND {
  frun_option *func;
  globals *global = (globals *)g_cs->global;
  
  func = frun_get(global->functions, string_get(g_cs, $1));
  
  if (func != NULL)
  {
    $$ = proc_append(g_cs, func->func, 1, $2);
  }
  else
  {
    yyerror("no such function");
    exit(1);
  }
}

argv: ARGUMENT                { $$ = array_create(g_cs);
                                array_append(g_cs, $$, $1); }
    | argv ARGUMENT           { $$ = $1; 
                                array_append(g_cs, $$, $2); }
;
