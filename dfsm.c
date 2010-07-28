/* Deterministic finite automata related functions
 * (c) 2007 G. Nicholas D'Andrea
 */

#include <stdlib.h>
#include <stdio.h>

#include "fsm.h"
#include "dfsm.h"

extern void *EPSILON;

struct FSM *deterministic_fsm(struct FSM *ndfa, void **alphabet,
    int num_symbols)
{
  /* Basic idea behind converting an NFA to a DFA:
   *  In an NFA, we must keep track of the possible states you could be in at
   *  any given time in your input.
   *  Thus, if we keep track of all possible states on each input from a list
   *  of all possible states before it, we can link possible state "metastate"
   *  and the result will be deterministic.
   */


  struct FSM *dfa;
  dfa = (struct FSM *) malloc( sizeof(struct FSM) );

  /* We have to make a list of the possible states that we can start at */
  struct StateArray *starting_states = (struct StateArray *)
    malloc( sizeof(struct StateArray) );
  starting_states->states = (struct State **)
    malloc( 1 * sizeof(struct State *) );

  /* This will first include, well, the actual start state */
  starting_states->states[0] = ndfa->start_state;
  starting_states->num_states = 1;

  /* And very importantly, the epsilon closure thereof. */
  epsilon_closure(&starting_states->states, &starting_states->num_states);

  struct State *metastate = new_state(starting_states, ndfa->start_state->cmp);
  add_state(dfa, metastate);
  dfa->start_state = metastate;

  /* If any state is accepting within our metastate of start states, the
   * metastate is also accepting
   */
  int i;
  for(i = 0; i < starting_states->num_states; i++)
      if(starting_states->states[i]->accepting)
        metastate->accepting = 1;

  /* GO! */
  build_dfa_from_metastate(dfa, metastate, alphabet, num_symbols);

  return dfa;
}

void build_dfa_from_metastate(struct FSM *dfa, struct State *metastate,
    void **alphabet, int num_symbols)
{

  int i, j;
  struct State *link_to;

  /* Test each possible symbol from this metastate */
  for(i = 0; i < num_symbols; i++)
  {
    struct StateArray *states = (struct StateArray *) metastate->id;
    
    struct StateArray *possible_states =
      possible_states_from(states, alphabet[i]);

    /* Check if the possible state metastate already exists */
    link_to = NULL;
    for(j = 0; j < dfa->num_states; j++)
    {
      if(are_state_arrays_equal(possible_states,
            (struct StateArray *) dfa->states[j]->id))
      {
        link_to = dfa->states[j];
        break;
      } 
    }

    /* If it doesn't, make it */
    if(!link_to)
    {
      link_to = new_state(possible_states, metastate->cmp);

      for(j = 0; j < possible_states->num_states; j++)
        if(possible_states->states[j]->accepting)
          link_to->accepting = 1;
      
      add_state(dfa, link_to);
    }

    /* Now connect this metastate to the one it should link to */
    add_transition(metastate, link_to, alphabet[i]);

    /* And recurse!
     *
     * To avoid infinite looping, make sure the thing we're pointing to
     * hasn't already been started.
     * This should cause each metastate to be searched exactly once:
     *  If a transition from it's been defined, it either has all
     *    transitions already defined
     *  or the function adding transitions to that particular state is still
     *    on the call stack.
     */
    if(link_to->transitions_tree == NULL)
      build_dfa_from_metastate(dfa, link_to, alphabet, num_symbols);
  }

}

int are_state_arrays_equal(struct StateArray *left, struct StateArray *right)
{
  if(left->num_states != right->num_states)
    return 0;

  int i, j;

  for(i = 0; i < left->num_states; i++)
  {
    int found = 0;
    
    /* They won't necessarily be in the same order.
     * And I'm a lazy programmer.
     * So this is O(n^2) for now.
     */
    for(j = 0; j < right->num_states; j++)
    {
      if(left->states[i] == right->states[j])
        found = 1;
    }

    if(!found)
      return 0;
  }

  return 1;
}

struct StateArray *possible_states_from(struct StateArray *states, void *input)
{
  int num_possible_next_states = 0, i, j;

  struct StateArray *possible_next_states = (struct StateArray *)
    malloc( sizeof(struct StateArray));

  possible_next_states->num_states = 0;

  for(i = 0; i < states->num_states; i++)
  {
    struct Transition *from_here =
      transition_from_with_input(states->states[i], input);

    if(from_here != NULL)
      for(j = 0; j < from_here->num_to; j++)
        add_if_not_present((void ***) &possible_next_states->states,
            &possible_next_states->num_states, from_here->to[j]);
  }

  epsilon_closure(&possible_next_states->states,
      &possible_next_states->num_states);

  return possible_next_states;
}

void epsilon_closure(struct State ***possible_states, int *num_possible_states)
{
  int previous_num, i, j;
  do
  {
    previous_num = (*num_possible_states);

    for(i = 0; i < (*num_possible_states); i++)
    {
      struct Transition *epsilon =
        transition_from_with_input((*possible_states)[i], EPSILON);

      if(epsilon != NULL)
        for(j = 0; j < epsilon->num_to; j++)
          add_if_not_present((void ***) possible_states,
              num_possible_states, epsilon->to[j]);
    }
  } while(previous_num != (*num_possible_states));
}

int add_if_not_present(void ***ref_array, int *size, void *item)
{
  if(*size)
  {
    int i;
    for(i = 0; i < *size; i++)
      if((*ref_array)[i] == item)
        return 0;

    (*ref_array) = realloc((*ref_array), ++(*size) * sizeof((*ref_array)[0]));
    (*ref_array)[*size - 1] = item;
    return 1;
  }
  else
  {
    (*ref_array) = malloc(1 * sizeof(item));
    (*ref_array)[0] = item;
    *size = 1;
    return 1;
  }
}
