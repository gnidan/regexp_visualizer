#ifndef __FSM_H__
#define __FSM_H__


/* N.B. States are currently responsible for defining in essence a possible
 *  universe for the inputs on that state. It might make sense for a FSM to
 *  define its own universe instead.
 */

struct FSM
{
  struct State **states;
  struct State *start_state;
  int num_states;
};

void add_state(struct FSM *fsm, struct State *state);
void remove_state(struct FSM *fsm, struct State *state);

/*
 * Compares two values:
 *
 *   Return Value | Condition
 *   -------------+---------------
 *        -1      | left < right
 *   -------------+---------------
 *         0      | left == right
 *   -------------+---------------
 *         1      | left > right
 *   -------------+---------------
 */
typedef int (*comparator)(void *, void *);


/*
 * Transition acts sort of like a binary search tree.
 */
struct Transition
{
  void *value;
  struct State **to;
  int num_to;

  struct Transition *left;
  struct Transition *right;
};


struct State
{
  void *id;
  struct Transition *transitions_tree;
  int accepting;

  comparator cmp;
};


/* 
 * Create a new state with a pointer passed as a label/id.
 * This might be a (char *), but theoretically it can be anything,
 *   allowing for potential for easier lookup of state by id.
 */
struct State *new_state(void *id, comparator cmp);
void delete_state(struct State *state);

void add_transition(struct State *from, struct State *to, void *input);
void delete_transition(struct State *from, struct State *to, void *input);

void delete_struct_transition(struct Transition **t);

void delete_all_transitions(struct Transition *root);

int transitions_from_to(struct State *from, struct State *to,
    struct Transition ***ref_array);

struct Transition *transition_from_with_input(struct State *from, void *input);

int build_array_of_transitions_to(struct State *to, struct Transition *root,
    struct Transition ***ref_array);

struct FSM *fsmunion(struct FSM *left, struct FSM *right);
struct FSM *fsmcat(struct FSM *left, struct FSM *right);
struct FSM *fsmclosure(struct FSM *left);

#endif
