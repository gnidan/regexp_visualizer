/*
 * fsm.c | Implementation for Finite State Machine
 */

#include <stdlib.h>
#include "fsm.h"

void *EPSILON;

void add_state(struct FSM *fsm, struct State *state)
{
  if(fsm->num_states > 0)
    fsm->states = (struct State **) realloc(fsm->states,
        ++fsm->num_states * sizeof(struct State *));
  else
    fsm->states = (struct State **)
      malloc(++fsm->num_states * sizeof(struct State *));

  fsm->states[fsm->num_states-1] = state;
}

void remove_state(struct FSM *fsm, struct State *state)
{
  int i, found_yet = 0, j;

  /* We want to shift everything down in the array, then resize it.
   * Don't worry, we don't have to free() the state, we just have to
   *  remove it from the FSM.
   */
  for(i = 0; i < fsm->num_states - 1; i++)
    if(found_yet || (fsm->states[i] == state && (found_yet = 1)))
      fsm->states[i] = fsm->states[i+1];

  if(found_yet || fsm->states[fsm->num_states - 1] == state)
    fsm->states = (struct State **) realloc(fsm->states,
        --fsm->num_states * sizeof(struct State *));
  else
    return;

  /* Now we have to get rid of the transitions that lead to this state */

  /* Loop through each state */
  for(i = 0; i < fsm->num_states; i++)
  {
    struct State *current_state = fsm->states[i];
    struct Transition ***ref_transitions;
    int num_transitions;

    ref_transitions = (struct Transition ***)
      malloc( sizeof(struct Transition ***) );

    /* Figure out which transitions this state has that point to the state
     *  that we want to remove.
     */
    num_transitions = transitions_from_to(fsm->states[i], state,
        ref_transitions);

    /* Delete all of those */
    for(j = 0; j < num_transitions; j++)
      delete_transition(fsm->states[i], state, (*ref_transitions)[j]->value);
    
    free(ref_transitions);
  }
}

                             /* -------------- */

struct State *new_state(void *id, comparator cmp)
{
  struct State *state;

  state = (struct State *) malloc( sizeof(struct State) );
  
  state->id = id;
  state->cmp = cmp;
  state->transitions_tree = NULL;
  state->accepting = 0;

  return state;
}

void delete_state(struct State *state)
{
  if(state->transitions_tree != NULL)
    delete_all_transitions(state->transitions_tree);

  free(state);
}

void delete_all_transitions(struct Transition *root)
{
  if(root->left != NULL)
    delete_all_transitions(root->left);
  
  if(root->right != NULL)
    delete_all_transitions(root->right);

  free(root);
}

void add_transition(struct State *from, struct State *to, void *input)
{
  struct Transition *cur = from->transitions_tree;
  
  if(cur == NULL)
  {
    struct Transition *t = (struct Transition *)
      malloc( sizeof(struct Transition) );

    t->value = input;
    t->to = (struct State **) malloc( 1 * sizeof(struct State *) );
    t->num_to = 1;
    t->left = NULL;
    t->right = NULL;
    t->to[0] = to;

    from->transitions_tree = t;
  }

  while(cur != NULL)
  {
    int comparison = (*from->cmp)(input, cur->value);
    if(comparison < 0)
    {
      /* less than cur: Go to left child */
      if(cur->left != NULL)
        cur = cur->left;
      else
      {
        struct Transition *t = (struct Transition *)
          malloc( sizeof(struct Transition) );

        t->value = input;
        t->to = (struct State **) malloc( 1 * sizeof(struct State *) );
        t->num_to = 1;
        t->to[0] = to;
        t->left = NULL;
        t->right = NULL;

        cur->left = t; 

        cur = NULL;
      }
    }
    else if(comparison > 0)
    {
      /* greater than cur: Go to right child */
      if(cur->right != NULL)
        cur = cur->right;
      else
      {
        struct Transition *t = (struct Transition *)
          malloc( sizeof(struct Transition) );

        t->value = input;
        t->to = (struct State **) malloc( 1 * sizeof(struct State *) );
        t->num_to = 1;
        t->to[0] = to;
        t->left = NULL;
        t->right = NULL;

        cur->right = t;

        cur = NULL;
      }
    }
    else
    {
      /* equal to cur: Add to to cur */
      struct Transition *t = cur;
      int i, found = 0;

      for(i = 0; i < t->num_to; i++)
        if(t->to[i] == to)
        {
          found = 1; 
          break;
        }
      
      if(!found)
      {
        t->to = (struct State **) realloc(t->to,
            ++t->num_to * sizeof(struct State *));

        t->to[t->num_to - 1] = to;
      }

      cur = NULL;
    }
  }
}

void delete_transition(struct State *from, struct State *to, void *input)
{
  struct Transition **cur = &from->transitions_tree;
  int comparison;

  /* Gotta make sure there are transitions from this state to begin with... */
  if(cur == NULL)
    return;

  do
  {
    /* We now gotta find the (struct Transition *) with the right input...
     * Remember, the transition bst is organized by input symbol
     */
    comparison = (*from->cmp)((*cur)->value, input);

    if(comparison < 0)
      /* Look to the left */
      if((*cur)->left != NULL)
        (*cur) = (*cur)->left;
      else
        return;

    else if(comparison > 0)
      /* Look to the right */
      if((*cur)->right != NULL)
        (*cur) = (*cur)->right;
      else
        return;
  } while(comparison != 0);

  /* In theory we now have cur assigned to the correct struct Transition *.
   * Now let's see if it's going to the struct State we're trying to delete.
   */
  int i, found = 0;
  for(i = 0; i < (*cur)->num_to; i++)
  {
    if(!found)
      if((*cur)->to[i] == to)
        found = 1;

    if(found)
      if(i == (*cur)->num_to - 1)
        (*cur)->to = (struct State **) realloc((*cur)->to,
            --(*cur)->num_to * sizeof(struct State *));
      else
        (*cur)->to[i] = (*cur)->to[i+1];
  }

  /* Now it should have successfully been removed from that array, and that
   * array has been downsized.
   * But that might leave us with an empty array. If that's the case, let's
   * remove the struct Transition * from the BST.
   */
  if(!(*cur)->num_to)
    delete_struct_transition(cur);
}

void delete_struct_transition(struct Transition **node)
{
  if((*node)->left == NULL)
  {
    struct Transition *temp;
    temp = *node;
    *node = (*node)->right;
    free(temp);
  }
  else if((*node)->right == NULL)
  {
    struct Transition *temp;
    temp = *node;
    *node = (*node)->left;
    free(temp);
  }
  else
  {
    struct Transition **pred = &(*node)->left;

    while((*pred)->right != NULL)
    {
      (*pred) = (*pred)->right;
    }

    (*node)->value = (*pred)->value;
    (*node)->to = (*pred)->to;
    (*node)->num_to = (*pred)->num_to;

    delete_struct_transition(pred);
  }
}

int transitions_from_to(struct State *from, struct State *to,
    struct Transition ***ref_array)
{
  if(from->transitions_tree == NULL)
    return 0;
  else
    return build_array_of_transitions_to(to, from->transitions_tree, ref_array);
}

struct Transition *transition_from_with_input(struct State *from, void *input)
{
  struct Transition *cur = from->transitions_tree;

  while(cur != NULL)
  {
    int comparison = (*from->cmp)(input, cur->value);
    if(comparison < 0)
    {
      if(cur->left != NULL)
        cur = cur->left;
      else
        return NULL;
    }
    else if(comparison > 0)
    {
      if(cur->right != NULL)
        cur = cur->right;
      else
        return NULL;
    }
    else
      return cur;
  }

  return NULL;
}

int build_array_of_transitions_to(struct State *to, struct Transition *root,
    struct Transition ***ref_array)
{
  int array_size = 0;

  if(root->left != NULL)
    array_size += build_array_of_transitions_to(to, root->left, ref_array);

  if(root->right != NULL)
    array_size += build_array_of_transitions_to(to, root->right, ref_array);

  int i, num_to;

  for(i = 0; i < root->num_to; i++)
  {
    if(root->to[i] == to)
    {
      (*ref_array) = realloc((*ref_array),
          ++array_size * sizeof(struct Transition *));
      (*ref_array)[array_size - 1] = root;
    }
  }

  return array_size;
}

struct FSM *fsmunion(struct FSM *left, struct FSM *right)
{
  struct FSM *fsm = (struct FSM *) malloc(sizeof(struct FSM));

  fsm->num_states = 0;

  struct State *start = new_state(NULL, left->start_state->cmp);
  struct State *end = new_state(NULL, left->start_state->cmp);

  add_state(fsm, start);

  fsm->start_state = start;

  add_transition(start, left->start_state, EPSILON);
  add_transition(start, right->start_state, EPSILON);

  int i;
  for(i = 0; i < left->num_states; i++)
  {
    add_state(fsm, left->states[i]);
    if(left->states[i]->accepting)
    {
      left->states[i]->accepting = 0;
      add_transition(left->states[i], end, EPSILON);
    }
  }

  for(i = 0; i < right->num_states; i++)
  {
    add_state(fsm, right->states[i]);
    if(right->states[i]->accepting)
    {
      right->states[i]->accepting = 0;
      add_transition(right->states[i], end, EPSILON);
    }
  }

  add_state(fsm, end);

  end->accepting = 1;

  return fsm;
}

struct FSM *fsmcat(struct FSM *left, struct FSM *right)
{
  struct FSM *fsm = (struct FSM *) malloc(sizeof(struct FSM));

  fsm->num_states = 0;

  fsm->start_state = left->start_state;

  int i;
  for(i = 0; i < left->num_states; i++)
  {
    add_state(fsm, left->states[i]);
    if(left->states[i]->accepting)
    {
      left->states[i]->accepting = 0;
      add_transition(left->states[i], right->start_state, EPSILON);
    }
  }

  for(i = 0; i < right->num_states; i++)
    add_state(fsm, right->states[i]);

  return fsm;
}

struct FSM *fsmclosure(struct FSM *left)
{
  struct FSM *fsm = (struct FSM *) malloc(sizeof(struct FSM));
  fsm->num_states = 0;

  struct State *start = new_state(NULL, left->start_state->cmp);
  struct State *end = new_state(NULL, left->start_state->cmp);

  add_state(fsm, start);

  fsm->start_state = start;

  add_transition(fsm->start_state, left->start_state, EPSILON);

  int i;
  for(i = 0; i < left->num_states; i++)
  {
    add_state(fsm, left->states[i]);
    if(left->states[i]->accepting)
    {
      left->states[i]->accepting = 0;
      add_transition(left->states[i], end, EPSILON);

      add_transition(left->start_state, left->states[i], EPSILON);
      add_transition(left->states[i], left->start_state, EPSILON);
    }
  }

  end->accepting = 1;
  add_state(fsm, end);

  return fsm;
}

