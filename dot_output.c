/* Output methods for finite state machines
 * (c) 2007 G. Nicholas D'Andrea
 */

#include <stdio.h>
#include "fsm.h"
#include "dot_output.h"

void fprint_fsm(FILE *stream, struct FSM *fsm,
    symbol_string_func symbol_string, id_string_func id_string)
{
  /* Generic stuff for all FSMs */
  fprintf(stream, "digraph fsm\n{\n");
  fprintf(stream, "rankdir=\"LR\"\n");
  fprintf(stream, "edge [fontname=\"Verdana\"]\n");
  fprintf(stream, "node [fontname=\"Verdana\"]\n");
  fprintf(stream, "start [shape=\"plaintext\",label=\"start\"]\n");

  /* Now let's get into specifics... */
  int i, start = 0;

  for(i = 0; i < fsm->num_states; i++)
  {
    char *s = id_string(fsm->states[i]->id);
    fprintf(stream, "%i [shape=\"%s\",label=\"%s\"]\n",
        i, (fsm->states[i]->accepting ? "doublecircle" : "circle"), s);
    free(s);

    if(fsm->states[i] == fsm->start_state)
      start = i;
  }

  fprintf(stream, "start->%i\n", start);

  for(i = 0; i < fsm->num_states; i++)
  {
    if(fsm->states[i]->transitions_tree != NULL)
      fprint_transitions(stream, fsm->states[i]->transitions_tree, 
          fsm, fsm->states[i], symbol_string);
  }

  fprintf(stream, "}\n");
}

void fprint_transitions(FILE *stream, struct Transition *root,
    struct FSM *fsm, struct State *from, symbol_string_func symbol_string)
{
  if(root->left != NULL)
    fprint_transitions(stream, root->left, fsm, from, symbol_string);

  int i, j, from_index, to_index;

  for(i = 0; i < fsm->num_states; i++)
    if(fsm->states[i] == from)
      from_index = i;

  for(i = 0; i < root->num_to; i++)
  {
    for(j = 0; j < fsm->num_states; j++)
      if(fsm->states[j] == root->to[i])
        to_index = j;

    char *s = symbol_string(root->value);
    fprintf(stream, "%i->%i [label=\"%s\"]\n", from_index, to_index, s);
    free(s);
  }

  if(root->right != NULL)
    fprint_transitions(stream, root->right, fsm, from, symbol_string);
}
