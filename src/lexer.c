/*
 * The supervize lexer, which is a simple coroutine.
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "supervize.h"
#include "parser.tab.h"
#include "frun.h"

#define YYACCEPT 0
#define YYABORT 1
#define YYERROR -1

#define coBEGIN     static int state = 0; switch (state) { case 0:
#define coRETURN(v) { state = __LINE__; return v; case __LINE__:; } while(0);
#define coEND       }

#define dprintf(...) fprintf(stderr, __VA_ARGS__);
#undef dprintf
#define dprintf(...)

char **g_argv;
int g_index;

#define LEXER_BUFFER_MAX 1024

static char _lexer_buffer[1024];
static int _lexer_pos = 0;

/* _get_argument - Uses coroutines to send a specific argument over and over again.
 *  
 */
char *
_get_argument()
{
  coBEGIN;
  while (1)
  {
    if (g_mode == CLI)
    {
      dprintf("LEXER: returning argument: %s\n", g_argv[g_index]);
      coRETURN(g_argv[g_index++]);
    }
    else
    {
      char c;
      _lexer_pos = 0;
      
      dprintf("LEXER: flushing whitespace\n");
      do
      {
        c = fgetc(stdin);
        if (c <= 0) return NULL;
      } while (isspace(c));
      
      dprintf("LEXER: reading characters\n");
      do
      {
        _lexer_buffer[_lexer_pos++] = c;
        c = fgetc(stdin);
        if (c <= 0) break;
        if (_lexer_pos > LEXER_BUFFER_MAX) return NULL;
      } while (!isspace(c));
      
      if (_lexer_pos == 0) return NULL;
      
      _lexer_buffer[_lexer_pos++] = '\0';
      _lexer_pos = 0;
      
      dprintf("LEXER: returning argument: %s\n", _lexer_buffer);
      coRETURN(_lexer_buffer);
    }
  }
  coEND;
}

int
yylex(YYSTYPE *lvalp)
{
  char *argument;
  coBEGIN;
  
  while (1)
  {
    dprintf("LEXER: state = begin\n");
    argument = _get_argument();
    
    if (argument == NULL)
    {
      coRETURN(YYACCEPT);
      break;
    }
    
    if (strcmp(argument, "or") == 0)
    {
      coRETURN(OR);
      continue;
    }

    if (strcmp(argument, "!") == 0 || strcmp(argument, "not") == 0)
    {
      coRETURN(NOT);
      continue;
    }
    
    if (strcmp(argument, "and") == 0)
    {
      coRETURN(AND);
      continue;
    }
    
    if (strcmp(argument, "{") == 0)
    {
      coRETURN('{');
      continue;
    }

    if (strcmp(argument, "}") == 0)
    {
      coRETURN('}');
      continue;
    }
    
    if (strcmp(argument, "do") == 0)
    {
      coRETURN(DO);
      continue;
    }
    
    if (strcmp(argument, "else") == 0)
    {
      coRETURN(ELSE);
      continue;
    }
    
    if (argument[0] == '-' && argument[1] != '-')
    {
      goto read_function;
    }
    
    coRETURN(YYERROR);
    continue;
    
read_function:;
    dprintf("LEXER: state = read_function\n");
    
    globals *global = (globals *)g_cs->global;
    
    frun_option *func = frun_get(global->functions, argument + 1);

    if (func == NULL)
    {
      fprintf(stderr, "Could not find function '%s'\n", argument + 1);
      coRETURN(YYERROR);
      continue;
    }
    
    (*lvalp).string = string_append(g_cs, argument + 1);
    coRETURN(ARGUMENT);
    
    /* read variadic amound of arguments */
    if (func->argc == -1)
    {
      while (1)
      {
        argument = _get_argument();
        
        if (argument == NULL)
        {
          fprintf(stderr, "No end of arguments - Could not find '$'\n");
          coRETURN(YYERROR);
          return YYERROR; /* dead lock */
        }
        
        if (strcmp(argument, "$") == 0)
        {
          break;
        }
        
        (*lvalp).string = string_append(g_cs, argument);
        coRETURN(ARGUMENT);
      }
      
      coRETURN(ARGEND);
      continue;
    }
    
    int args = func->argc;
    
    /* read all arguments */
    while (args-- > 0)
    {
      argument = _get_argument();
      
      if (argument == NULL)
      {
        if (args == -1)
        {
          fprintf(stderr, "Too few arguments - Expected %d\n", func->argc);
          coRETURN(YYERROR);
        }
        
        coRETURN(YYABORT);
        break;
      }
      
      (*lvalp).string = string_append(g_cs, argument);
      coRETURN(ARGUMENT);
    }
    
    coRETURN(ARGEND);
    continue;
  }
  
  coEND;
  fprintf(stderr, "Lexer called after YYACCEPT\n");
  return YYERROR;
}

void
yyerror(const char *message)
{
  printf("%s: Parser Error - %s\n", g_program_name, message);
  print_help();
  return;
}

