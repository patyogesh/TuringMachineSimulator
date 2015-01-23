/************************************************************
 *
 *  Author: Yogesh Patil
 *  Email: patyogesh@gmail.com
 *
 *  Graduate Student, 
 *  University of Florida, 
 *  Gainesville, FL USA
 *
 ************************************************************/

#ifndef TURING_SIM_H
#define TURING_SIM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define uint32 	unsigned int
#define int32 	int
#define uint16 	unsigned short
#define int16 	short
#define uchar 	unsigned char


/* Buffer size for Tape Date */
#define TAPE_DATA_BUFFER 1024

typedef enum MOVE_DIRECTION 
{
    LEFT,	/* Move to left */
    STAY,	/* Don't move */
    RIGHT,	/* move to right */
    INVALID_DIR	/* Invalid direction */
}direction_e;

typedef enum parse_status 
{
    PARSE_SUCCESS,	/* Action line parsing successfull */
    PARSE_FAIL,		/* Action line parsing failed */
}parse_status_e;

typedef enum halt_status 
{
    HALT,	/* System Halts */
    NO_HALT,	/* System Does NOT halt */
    INVALID	/* Invalid */
}halt_status_e;

/* 
 *
 */
typedef struct wild_char_bitmap
{
    /*
     * 32 bit bitmap, where each bit indicates if that numbered
     * state has wild-character action in table
     *
     * 	1 - Wild character action present
     * 	0 - No Wild character action present
     *
     * Current 32 bit long bitmap will support 32 states.
     * Purpose to use struct here is for future use, perhaps to support
     * more number of states, a character array may be needed as number of
     * states go beyong integer bit size
     */

    uint32 wild_char_bit_pos;

}wild_char_bitmap_t;

/*
 * Action/rule format
 *
 *   @cur_state : State Index
 *   @read_char: If character
 *   @write_char: Write this character in place of 'read_char'
 *   @dir: Direction in which head is moved
 *	-1  : Left
 *	 0  : Stay
 *	 1  : Right
 *   @new_statea: New State Index
 *   @next: This is internal to this implementation. 
 *	    Pointer to next action/rule with same cur_state
 */
typedef struct action_table action_table_t;
struct action_table
{
    uint16 	cur_state;
    uchar 	read_char;
    uchar 	write_char;
    direction_e dir;
    uint16 	new_state;
    action_table_t *next;
};

/*
 * Input Format as given in problem description
 *
 * 	@tape_date : STarting content on tape
 *	@head_pos : Index into above string determining start position of head
 *	@start_ind : Start State Index
 *	@halt_ind : Halting State Index
 *	@act_tbl: Action table with following format
 *		<State index> <If character> <Write> <Direction> <New state index>
 *		
 *		This is an array of pointers. Array size if bounded by number of actions in tables
 * 		and each index has one-to-one mapping with State Index.
 *		Since some states can have multiple transitions based on character read,
 *		A Singly Linked List of actions is maintained for each such State Index 
 *		act_tbl[index] pointing to head of a list.
 */
typedef struct io_data
{
    uchar 	tape_data[TAPE_DATA_BUFFER];
    uint32 	head_pos;
    uint32 	start_ind;
    uint32 	halt_ind;
    action_table_t **act_tbl;
}io_data_t;


#endif
