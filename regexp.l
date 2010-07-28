%{
#include <stdlib.h>
#include "fsm.h"
#include "dot_output.h"
#include "dfsm.h"
#include "regexp.tab.h"

int cmp(void *left, void *right);

extern void **alphabet;
extern int alphabet_size;

%}

%%

[a-z] {
        yylval.fsm = (struct FSM *) malloc(sizeof(struct FSM));
        yylval.fsm->num_states = 0;
        add_state(yylval.fsm, new_state(NULL, cmp));
        add_state(yylval.fsm, new_state(NULL, cmp));
        yylval.fsm->start_state = yylval.fsm->states[0];
        yylval.fsm->states[1]->accepting = 1;

        char s[2];
        sprintf(s, "%c", yytext[0]);
        add_if_not_present(&alphabet, &alphabet_size, strdup(s));

        add_transition(yylval.fsm->states[0], yylval.fsm->states[1], strdup(s));

        return CHARACTER;
      }
"|"   { return '|'; }
"*"   { return '*'; }
"("   { return '('; }
")"   { return ')'; }
\n    { return yytext[0]; }
[ \t] ;

%%

int cmp(void *left, void *right)
{
  if(left == EPSILON && right != EPSILON)
    return -1;
  else if(right == EPSILON && left != EPSILON)
    return 1;
  else if(left == right)
    return 0;
  else
    return strcmp((char *) left, (char *) right);
}
