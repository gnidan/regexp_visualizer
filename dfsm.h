/* Headers for deterministic finite automata functions
 * (c) 2007 G. Nicholas D'Andrea
 */

#ifndef __DFSM_H__
#define __DFSM_H__

/* We define EPSILON here as just a memory location. All pointers to an epsilon
 * transition shall use this memory location, and nothing that isn't this memory
 * location shall be epsilon.
 */
void *EPSILON;

struct StateArray
{
  struct State **states;
  int num_states;
};

/* Make a deterministic FSM out of a non-deterministic one
 * Accepts an array of symbols in the alphabet, so it knows what to check
 */
struct FSM *deterministic_fsm(struct FSM *ndfa, void **alphabet,
    int num_symbols);

/* Function used by deterministic_fsm() to recursively build the DFA */
void build_dfa_from_metastate(struct FSM *dfa, struct State *metastate,
    void **alphabet, int num_symbols);

int are_state_arrays_equal(struct StateArray *left, struct StateArray *right);

/* Do the epsilon closure for a set of possible states */
void epsilon_closure(struct State ***possible_states, int *num_possible_states);

struct StateArray *possible_states_from(struct StateArray *states, void *input);

int add_if_not_present(void ***ref_array, int *size, void *item);


#endif
