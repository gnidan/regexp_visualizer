%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fsm.h"
#include "dot_output.h"
#include "dfsm.h"

void **alphabet;
int alphabet_size = 0;

int input_number = 0;

char *symbol_string(void *value);
char *id_string(void *id);
char *meta_id_string(void *id);

int test_string(struct FSM *fsm, char *string);
%}

%union {
  struct FSM *fsm;
}

%token <fsm> CHARACTER
%type  <fsm> option
%type  <fsm> sequence
%type  <fsm> subexp

%%

regular_expression_list: regular_expression '\n'
                       | regular_expression_list regular_expression '\n'
                       ;

regular_expression     : option   { struct FSM *fsm = $1;
                                    int i, digits, temp;
                                    char *s;

                                    input_number++;
                                    temp = input_number;
                                    for(digits = 1; temp /= 10; digits++);

                                    s = (char *) malloc((digits + 5) *
                                      sizeof(char));
                                    sprintf(s, "%i.dot", input_number);

                                    FILE *file = fopen(s, "w");
                                    
                                    free(s);
 
                                    for(i = 0; i < fsm->num_states; i++)
                                    {
                                      temp = i;
                                      for(digits = 1; temp /= 10; digits++);
                                      s = (char *) malloc((digits + 2) *
                                        sizeof(char));
                                      sprintf(s, "s%i", i);
                                      fsm->states[i]->id = s;
                                    }
                             
                                    struct FSM *dfa =
                                      deterministic_fsm(fsm, alphabet,
                                      alphabet_size);
 
                                    for(i = 0; i < dfa->num_states; i++)
                                    {
                                      int digits, temp = i;
                                      for(digits = 1; temp /= 10; digits++);
                                      s = (char *) malloc((digits + 2) *
                                        sizeof(char));
                                      sprintf(s, "s%i", i);
                                      dfa->states[i]->id = s;
                                    }
 
                                    fprint_fsm(file, fsm,
                                      symbol_string, id_string);
                                  }
                       ;

option                 : option '|' sequence { struct FSM *left = $1;
                                               struct FSM *right = $3; 
                                               $$ = fsmunion(left, right);
                                               free(left);
                                               free(right);
                                             }
                       | sequence            { $$ = $1 }
                       ;

sequence               : sequence subexp     { struct FSM *left = $1;
                                               struct FSM *right = $2; 
                                               $$ = fsmcat(left, right);
                                               free(left);
                                               free(right);
                                             }
                       | subexp              { $$ = $1 }
                       ;

subexp                 : '(' option ')'      { $$ = $2 }
                       | subexp '*'          { struct FSM *fsm = $1;
                                               $$ = fsmclosure(fsm);
                                               free(fsm);
                                             }
                       | CHARACTER           { $$ = $1 }
                       ;

%%

int test_string(struct FSM *fsm, char *string)
{
  char *cur = string;

  struct StateArray *possible_states = (struct StateArray *)
    malloc( sizeof(struct StateArray *) );

  possible_states->states = (struct State **)
    malloc( 1 * sizeof(struct State *) );

  possible_states->states[0] = fsm->start_state;
  possible_states->num_states = 1;

  int i, j;

  epsilon_closure(&possible_states->states, &possible_states->num_states);

  do
  {
    char temp[2];
    sprintf(temp, "%c", *cur);

    possible_states = possible_states_from(possible_states, temp);

  } while(*(++cur) != '\0');


  for(i = 0; i < possible_states->num_states; i++)
    if(possible_states->states[i]->accepting)
      return 1;

  return 0;
}

char *symbol_string(void *value)
{
  if(value == EPSILON)
    return strdup("&#949;");
  else
    return strdup((char *) value);
}

char *meta_id_string(void *id)
{
  struct StateArray *states = (struct StateArray *) id;
  /* Size starts out as 3 because we must allow for {, }, and \0 */
  int i, size = 3;

  for(i = 0; i < states->num_states; i++)
    size += strlen(id_string(states->states[i]->id)) + 1;

  char *string = (char *) malloc( size * sizeof(char) );

  string[0] = '{';
  
  for(i = 0; i < states->num_states; i++)
  {
    strcat(string, id_string(states->states[i]->id));
    if(i != states->num_states - 1)
      strcat(string, ",");
  }

  strcat(string, "}");

  return string;
}

char *id_string(void *id)
{
  if(id == NULL)
    return strdup("");
  return strdup((char *) id);
}

