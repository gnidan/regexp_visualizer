/* Header file for output of finite state machines
 * (c) 2007 G. Nicholas D'Andrea
 */

#ifndef __DOT_OUTPUT_H__
#define __DOT_OUTPUT_H__

/* Both of these should malloc() the char* they return.
 * Free them when you're done, please.
 */
typedef char * (*symbol_string_func)(void *);
typedef char * (*id_string_func)(void *);

void fprint_fsm(FILE *stream, struct FSM *fsm,
    symbol_string_func symbol_string, id_string_func id_string);

void fprint_transitions(FILE *stream, struct Transition *root,
    struct FSM *fsm, struct State *from, symbol_string_func symbol_string);

#endif
