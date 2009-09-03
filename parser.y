/* Supervize argument parser */

%{
#include <stdio.h>
#include "supervize.h"
#include "stack.h"
%}

%pure-parser

%union {
  int proc;
  int string;
  int array;
}

%token BEGIN END ECHO EXEC PID WHEN DO ELSE AND OR NOT ARGEND
%token <string> ARGUMENT
%type <proc> pid_stmt exec_stmt echo_stmt stmt
%type <array> argv

%left '&'
%left '|'
%left '!'

%% /* Grammar rules and actions follow */
exp: stmt DO stmt ELSE stmt {sv_root(g_cs, proc_run(g_cs, $1), $3, $5);}
exp: stmt DO stmt           {sv_root(g_cs, proc_run(g_cs, $1), $3, -1);}
exp: stmt                   {sv_root(g_cs, proc_run(g_cs, $1), -1, -1);}

stmt: NOT stmt      {$$ = proc_append(g_cs, sv_stmt_not, 1, $2);}
    | pid_stmt
    | exec_stmt
    | echo_stmt
    | '{' stmt '}'  {$$ = $2}
    | stmt AND stmt {$$ = proc_append(g_cs, sv_stmt_and, 2, $1, $3);}
    | stmt OR stmt  {$$ = proc_append(g_cs, sv_stmt_or, 2, $1, $3);}
;

exec_stmt:  EXEC argv ARGEND  {$$ = proc_append(g_cs, sv_exec, 1, $2);}
echo_stmt:  ECHO argv ARGEND  {$$ = proc_append(g_cs, sv_echo, 1, $2);}
pid_stmt:   PID ARGUMENT      {$$ = proc_append(g_cs, sv_pid, 1, $2);}

argv: ARGUMENT                { $$ = array_create(g_cs);
                                array_append(g_cs, $$, $1); }
    | argv ARGUMENT           { $$ = $1; 
                                array_append(g_cs, $$, $2); }
;
