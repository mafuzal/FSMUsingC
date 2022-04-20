#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "generateFSM.h"


/*
*createNewFSM
*/

fsm_t* create_new_fsm(const char* fsm_name)
{
	fsm_t* fsm = calloc(1, sizeof(fsm_t));
	strncpy(fsm->fsm_name, fsm_name, MAX_FSM_NAME_SIZE -1);
	fsm->fsm_name[MAX_FSM_NAME_SIZE -1] = '\0';
	return fsm;
}


/*
 * setFSMInitialState
 */
void set_fsm_initial_state(fsm_t* fsm, state_t* state)
{
	assert(!fsm->initial_state);
	fsm->initial_state = state;
}


/*
 *setFSMInputBufferSize
 */
void set_fsm_input_buffer_size(fsm_t* fsm, unsigned int size)
{
	fsm->input_buffer[size] = '\0';
	fsm->input_buffer_size = size;
}

/*
*createNewState
*/
state_t* create_new_state(char* state_name, fsm_bool_t is_final)
{
	assert(state_name);
	state_t* state = calloc(1, sizeof(state_t));
	strncpy(state->state_name, state_name, MAX_STATE_NAME_SIZE -1);
	state->state_name[MAX_STATE_NAME_SIZE -1] = '\0';
	state->is_final = is_final;
	return state;
}

/*
 * isTransitionTableEntryEmpty
 */

static fsm_bool_t is_tt_entry_empty(tt_entry_t* tt_entry)
{
	if(!tt_entry->next_state)
	{
		return FSM_TRUE;
	}
	return FSM_FALSE;
}


/*
 * getNextEmptyTransitionTableEntry
 */

tt_entry_t* get_next_empty_tt_entry(tt_t* trans_table)
{
	tt_entry_t* tt_entry_ptr = NULL;
	assert(trans_table);
	FSM_ITERATE_TRANS_TABLE_BEGIN(trans_table, tt_entry_ptr){

	}FSM_ITERATE_TRANS_TABLE_END(trans_table, tt_entry_ptr);

	if(is_tt_entry_empty(tt_entry_ptr) == FSM_TRUE)
		return tt_entry_ptr;

	return NULL;
}


/*
*createInsertNewTransitionTableEntry
*/
tt_entry_t* create_and_insert_new_tt_entry(tt_t* trans_table,
		char* transition_key,
		unsigned int sizeof_key,
		state_t* next_state,
		output_fn outp_fn)
{
	assert(sizeof_key < MAX_TRANSITION_KEY_SIZE);
	tt_entry_t* tt_entry_ptr = get_next_empty_tt_entry(trans_table);

	if(!tt_entry_ptr)
	{
		printf("FATAL : Transition Table is Full.\n");
		return tt_entry_ptr;
	}

	memcpy(tt_entry_ptr->transition_key, transition_key, sizeof_key);
	tt_entry_ptr->transition_key[sizeof_key] = '\0';
	tt_entry_ptr->transition_key_size = sizeof_key;
	tt_entry_ptr->next_state = next_state;
	tt_entry_ptr->outp_fn = outp_fn;
	return tt_entry_ptr;

}

/*
 * FSMDefaultInputMatching
 */
static fsm_bool_t fsm_default_input_matching_fn(char* transition_key,
									unsigned int size, char* user_data)
{
	if(memcmp(transition_key, user_data, size))
	{
		return FSM_FALSE;
	}
	return FSM_TRUE;
}

/*
 * FSMApplyTransition
 */
static state_t* fsm_apply_transition(fsm_t* fsm, state_t* state,  /* */
		char* input_buffer,
		unsigned int size,
		unsigned int* length_read,
		fsm_output_buff_t* output_buffer)
{
	tt_entry_t* tt_entry = NULL;
	state_t* next_state = NULL;

	FSM_ITERATE_TRANS_TABLE_BEGIN((&state->state_trans_table), tt_entry){
		if((tt_entry->transition_key_size <= size) &&
				fsm_default_input_matching_fn(tt_entry->transition_key,
						tt_entry->transition_key_size,
						input_buffer))
		{   //printf("Entered:fsm_apply_transition");
			next_state = tt_entry->next_state;
			if(tt_entry->outp_fn)
			{
				tt_entry->outp_fn(state, next_state, input_buffer,
						tt_entry->transition_key_size, output_buffer);
			}
			*length_read = tt_entry->transition_key_size;
			return next_state;
		}
	}FSM_ITERATE_TRANS_TABLE_END(&state->state_trans_table, tt_entry);
	return NULL;
}


/*
*execute_fsm
*/

fsm_error_t execute_fsm(fsm_t* fsm,
		char* input_buffer,  				/* Input buffer containing user data*/
		unsigned int size,  				/* Size of the data in the buffer*/
		fsm_output_buff_t* output_buffer, 	/* Output buffer to write data into*/
		fsm_bool_t* fsm_result)				/* A flag to track if data parsing ends in final state or not.*/
{
	state_t* initial_state = fsm->initial_state;
	assert(initial_state);

	state_t* current_state = initial_state;
	state_t* next_state = NULL;
	fsm->input_buffer_cursor = 0;
	unsigned int length_read = 0;
	unsigned int input_buffer_len = size;

	if(fsm_result)
	{
		*fsm_result = FSM_FALSE;
	}

	while(fsm->input_buffer_cursor < MAX_INP_BUFFER_LEN)
	{
		length_read = 0;
		next_state = fsm_apply_transition(fsm, current_state,
				input_buffer + fsm->input_buffer_cursor,
				(input_buffer_len - fsm->input_buffer_cursor),
				&length_read, output_buffer);

		if(length_read)
		{
			fsm->input_buffer_cursor += length_read;

			if(!next_state)
			{
				return FSM_NO_TRANSITION;
			}
			current_state = next_state;
			continue;
		}
		break;
	}

	if(fsm_result)
	{
		*fsm_result = current_state->is_final;
	}

	return FSM_NO_ERROR;
}

/*
 * FSMOutputBuffer
 */
void init_fsm_output_buffer(fsm_output_buff_t* fsm_output_buff)
{
	memset(fsm_output_buff->output_buffer, 0, MAX_FSM_OUTPUT_BUFFER);
	fsm_output_buff->curr_pos = 0;
}
