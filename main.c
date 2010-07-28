#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fsm.h"
#include "dot_output.h"
#include "dfsm.h"

/*
 * regexp     -> option
 * option     -> sequence {'|' sequence}
 * sequence   -> subexp {subexp}
 * subexp     -> '(' option ')'
 *             | '(' option ')' '*'
 *             | CHARACTER '*'
 *             | CHARACTER
 */

char token;

void **alphabet;
int alphabet_size = 0;

void match(char c);
void getToken();
void error();

struct FSM *regexp();
struct FSM *option();
struct FSM *sequence();
struct FSM *subexp();

int cmp(void *left, void *right);
char *symbol_string(void *value);
char *id_string(void *id);
char *meta_id_string(void *id);

void match(char c)
{
  if(c == token)
    getToken();
  else
    error();
}

void getToken()
{
  token = getchar();
}

void error()
{
  printf("parse error\n");
  exit(1);
}

struct FSM *regexp()
{
  struct FSM *fsm = option();  
  if(token == '\n')
    match('\n');
  return fsm;
}

struct FSM *option()
{
  struct FSM *left = sequence();
  while(token == '|')
  {
    match('|');
    struct FSM *temp = left;
    struct FSM *right = sequence();

    left = fsmunion(left, right);

    free(temp);
    free(right);
  }

  return left;
}

struct FSM *sequence()
{
  struct FSM *left = subexp();
  while(token >= 'a' && token <= 'z')
  {
    struct FSM *temp = left;
    struct FSM *right = subexp();

    left = fsmcat(left, right);

    free(temp);
    free(right);
  }

  return left;
}

struct FSM *subexp()
{
  struct FSM *fsm;
  if(token == '(')
  {
    match('(');
    fsm = option();
    match(')');
  }
  else
  {
    fsm = (struct FSM *) malloc(sizeof(struct FSM));
    fsm->num_states = 0;
    add_state(fsm, new_state(NULL, cmp));
    add_state(fsm, new_state(NULL, cmp));
    fsm->start_state = fsm->states[0];
    fsm->states[1]->accepting = 1;

    char s[2];
    sprintf(s, "%c", token);

    add_if_not_present(&alphabet, &alphabet_size, strdup(s));
    add_transition(fsm->states[0], fsm->states[1], strdup(s));

    match(token);
  }
  
  if(token == '*')
  {
    match('*');
    struct FSM *temp = fsm;
    fsm = fsmclosure(fsm);
    free(temp); 
  }

  return fsm;
}

int main(int argc, char **argv)
{
  int input_number = 0;
  struct FSM *fsm;

  getToken();
  while(!feof(stdin))
  {
    fsm = regexp();
    int i, digits, temp;
    char *s;

    input_number++;
    temp = input_number;
    for(digits = 1; temp /= 10; digits++);

    s = (char *) malloc((digits + 5) * sizeof(char));
    sprintf(s, "%i.dot", input_number);

    FILE *file = fopen(s, "w");

    free(s);

    for(i = 0; i < fsm->num_states; i++)
    {
      temp = i;
      for(digits = 1; temp /= 10; digits++);
      s = (char *) malloc((digits + 2) * sizeof(char));
      sprintf(s, "s%i", i);
      fsm->states[i]->id = s;
    }

    fprint_fsm(file, fsm, symbol_string, id_string);

    fclose(file);
  }

  return 0;
}

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

