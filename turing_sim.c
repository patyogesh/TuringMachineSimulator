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
#include "turing_sim.h"

#define PRINT_TABLE 

#define INPUT_DATA_LINES 4

#define CLEAR_BITMAP(map) {\
    map = 0x00000000;\
}

#define SET_WILD_CHAR_PRESENT_BIT_IN_STATE(pos, map) {\
    map |= (0x01 << pos);\
}

#define GET_WILD_CHAR_PRESENT_BIT_IN_STATE(pos, map) (map & (0x01 << pos))

#define IS_PRINTABLE(c)  (((uint32)c > 32) &&((uint32)c < 255))

/*
 * Allocate action table 
 */
action_table_t **
alloc_action_table(uint32 line_count)
{
    action_table_t **act_tbl;

    act_tbl = (action_table_t **) malloc (sizeof(action_table_t) * line_count);
    memset(act_tbl, 0, sizeof(action_table_t) * line_count);

    return act_tbl;
}
/*
 * Deallocate each entry in action table and finally action table
 */
void
dealloc_action_table(action_table_t **act_tbl, uint32 line_count)
{
    action_table_t *cur, *next;
    uint32 i = 0;
    for(i=0; i < line_count; i++) {

	if(act_tbl[i]) {
	    cur = act_tbl[i];
	
	    while(cur) {
		next = cur->next;
	    	free(cur);
	    	cur = NULL;
		cur = next;
	    }
	}
    }

    if(act_tbl) {
    	free(act_tbl);
    	act_tbl = NULL;
    }
}

/*
 * This function read input action-lines byte by byte.
 *
 * This implementation assumes that 
 *	State Indexes will be Single Digits AND
 * 	Directions will be 1/0/-1 AND
 * 	Read/Write characters will be Printable ASCII
 * 
 * Return Error in following cases
 *	Non-Printable character encountered for 'If Character' or 'Write'
 *	Direction other than 1/0/-1	
 *
 */
action_table_t* 
read_action_line(char c, uint32 *wild_char_bit_pos, FILE *fptr)
{
#define FREE_AND_RETURN_ERROR() {\
    free(act_tbl);\
    act_tbl = NULL;\
    return NULL;\
}
    uchar next;
    action_table_t *act_tbl = (action_table_t *) malloc (sizeof(action_table_t));
    /*
     * Example : 1 - a -1 3
     */

    /* Read current state */
    act_tbl->cur_state = atoi(&c); //1

    /* move forward through white space */
    c = fgetc(fptr); // SPACE

    /* 
     * Read 'If Character' 
     * If Wild-Char, set bit in bitmap corresponding to current state
     */
    c = fgetc(fptr); //-

    if(!IS_PRINTABLE(c)) {
	printf("\n Non-Printable Character encountered as 'If-Character', Aborting !\n");
	FREE_AND_RETURN_ERROR();
    }

    act_tbl->read_char = c;

    if('*' == c) {
	SET_WILD_CHAR_PRESENT_BIT_IN_STATE(act_tbl->cur_state, *wild_char_bit_pos);
    }

    /* move forward through white space */
    c = fgetc(fptr);//SPACE

    /* 
     * Read 'Write' character 
     */
    c = fgetc(fptr);//a

    if(!IS_PRINTABLE(c)) {
	printf("\n Non-Printable Character encountered as 'Write', Aborting !\n");
	FREE_AND_RETURN_ERROR();
    }

    act_tbl->write_char = c;

    /* move forward through white space */
    c = fgetc(fptr);//SPACE

    /*
     * Read Direction
     *	-1 : Left
     *   0 : Stay Still
     *   1 : Right
     *  Anything Other than above are INVALID DIRECTIONS
     */
    c = fgetc(fptr);//-
    if(c == '-') {
	next = fgetc(fptr);
	if(next == '1') {
	    act_tbl->dir = LEFT;
	}
	else {
	    ///error "- shoudl be followd by 1 to indicate LEFT direction"
	    printf("\n Action line paring error: Invalid Direction, Aborting ! \n");
	    act_tbl->dir = INVALID_DIR;
	    FREE_AND_RETURN_ERROR();
	}
    }
    else if(c == '0') {
	act_tbl->dir = STAY;
    }
    else if(c == '1') {
	act_tbl->dir = RIGHT;
    }
    else {
	//error DIRECTION other than Left/Right/Stay
	printf("\n Action line paring error: Invalid Direction, Aborting !\n");
	act_tbl->dir = INVALID_DIR;
	FREE_AND_RETURN_ERROR();
    }
    /* move forward through white space */
    c = fgetc(fptr);//SPACE

    /* Read New state to move to */
    c = fgetc(fptr);//3
    act_tbl->new_state = atoi(&c);

    act_tbl->next = NULL;

    return act_tbl;
}
/*
 * Read action-lines from file and update act_tble
 * entries.
 */
parse_status_e 
fill_action_table(FILE *fptr, 
			action_table_t **act_tbl,
			uint32 *wild_char_bit_pos,
			uint32 line_count)
{
    char c;

    uint32 i = 0;
    action_table_t *node = NULL, *temp = NULL;

    while(!feof(fptr) && line_count) {

	while((c = fgetc(fptr)) != '\n') {

	    /* Read Initial State */
	    i = atoi(&c);
	    
	    node = read_action_line(c, wild_char_bit_pos, fptr);
	    
	    /* If reading fails, report parsing failure */
	    if(NULL == node) {
		return PARSE_FAIL; 
	    }
	    
	    /* update entry in action table based on input read */
	    if(act_tbl[i] == NULL) {
		/* If this is first action for this state, keep it @ Head */
		act_tbl[i] = node;
	    }
	     /*
	      * Append at the end of list 
	      * (Assuming all wild character actions come last)
	      */
	    else {
		temp = act_tbl[i];
		while(temp->next) {
		    temp = temp->next;
		}
		temp->next = node;
	    }
	    
	}
	line_count --;
    }

    return PARSE_SUCCESS;
}
/*
 * This is a utility function to count the number of action lines
 * line_count will be used to dynamically allocate memory for action table
 *
 */
uint32 count_action_lines(char *input_file) 
{
    uint32 line_count = 0;
    FILE   *fptr;
    uchar c;

    fptr = fopen(input_file, "rb+");
    if(NULL == fptr) {
	//perror
	return 0;
    }

    while(! feof(fptr))
    {
	c = fgetc(fptr);
	if('\n' == c) {
	    line_count ++;
	}
    }

    fclose(fptr);

    /* DO NOT count input, head_pos, start-state-ind & halt-state-index */
    return (line_count - INPUT_DATA_LINES);
}
/*
 * Read initial tape content in *read_buf
 * Append '\0' at the end of input. This will be useful 
 * to identify END of input while turing test
 */
void read_tape_content(uchar *read_buf, FILE *fptr) 
{
    uchar c;
    uint32 ind = 0 ;

    memset(read_buf, 0, TAPE_DATA_BUFFER);

    while((c = fgetc(fptr)) != '\n') {
	read_buf[ind] = c;
	ind++;
    }
    read_buf[ind] = '\0';
}

/*
 * Read 'If Character' from action-rule in *read_val
 */
void read_line_val(uint32 *read_val, FILE *fptr) 
{
    char c;
    uint32 multiplier = 10;

    *read_val = 0;

    while((c = fgetc(fptr)) != '\n') {
	*read_val = (*read_val * multiplier) + atoi(&c);
    }
}

#define MOVE_LEFT(c) (c--)

#define MOVE_RIGHT(c) (c++)

#define MOVE_HEAD(c, dir) {\
    if(LEFT == dir) {\
    	MOVE_LEFT(c);\
    }\
    else if(STAY == dir) {}\
    else {\
    	MOVE_RIGHT(c);\
    }\
}

#define WRITE_CHAR(dst, src) {\
    /* Write only if 'Write' char is NOT '*' */\
    if('*' != src) {\
    	dst = src;\
    }\
}
#define CHANGE_STATE(dst, src) {\
    dst = src;\
}

#define PRINT_ITERATION(tape_data, head, new_state) {\
    printf("\n tape date: %s ",tape_data);\
    if('\0' != head) {\
    	printf("\tHead: %c ", head);\
    } else {\
    	printf("\tHead: End");\
    }\
    printf("\tNew State: %d", new_state);\
}

/*
 *
 *
 *
 */
halt_status_e 
_turing_test(io_data_t *input_data, 
	     action_table_t **act_tbl, 
	     uint32 wild_char_map)
{
    uchar *temp = input_data->tape_data;
    /* make temp point to head position */
    temp += input_data->head_pos;
    /* start index */
    uint32 start_state = input_data->start_ind;
    /* end index */
    uint32 halt_state = input_data->halt_ind;

    uint32 curr_state = start_state;

    /* Pointer to action table, pointing to first entry in table */
    action_table_t *action = act_tbl[curr_state];

    /* Print inital status */
    PRINT_ITERATION(input_data->tape_data, *temp, curr_state);
    
    /*
     * Read input tape content and apply action in table until
     *	End State is reached OR End of Tape is reached
     */
    while(curr_state != halt_state && *temp != '\0') {

	/* If current state matches the action-line current state AND
	 *    char at head position is same as read char in action-line
	 */
	if((action && 
	   action->cur_state == curr_state &&
	   action->read_char == *temp)) { 

	    /* Wrire 'Write Char' to Tape */
	    WRITE_CHAR(*temp, action->write_char);
	    
	    /* Move based on Direction */
	    MOVE_HEAD(temp, action->dir);
	    
	    /* Change current state to New state */
	    CHANGE_STATE(curr_state, action->new_state);
	    
	    /* update action pointer point to action line for New state */
	    action = act_tbl[curr_state];

	    /* Print Tape COntents, Head position and Current State after this iteration */
    	    PRINT_ITERATION(input_data->tape_data, *temp, curr_state);
	}
	/* 
	 * If no specific match could be found AND
	 *    Wild-Char rule present, apply Wild Char rule.
	 * (Last Resort)
	 */
	else if(action &&
	       (NULL == action->next) && 
	       (GET_WILD_CHAR_PRESENT_BIT_IN_STATE(curr_state, wild_char_map))) {

		if(action->cur_state == curr_state) { 

	    	    /* Wrire 'Write Char' to Tape */
		    WRITE_CHAR(*temp, action->write_char);

	    	    /* Move based on Direction */
		    MOVE_HEAD(temp, action->dir);

	    	    /* Change current state to New state */
		    CHANGE_STATE(curr_state, action->new_state);

	    	    /* update action pointer point to action line for New state */
		    action = act_tbl[curr_state];

	    	    /* Print Tape COntents, Head position and Current State after this iteration */
    	    	    PRINT_ITERATION(input_data->tape_data, *temp, curr_state);
		}
	}
	/* 
	 * If no specific match could be found AND
	 *    Wild-Char rule is also NOT present, return Error.
	 */
	else if(action &&
	       (NULL == action->next) && 
	       (!GET_WILD_CHAR_PRESENT_BIT_IN_STATE(curr_state, wild_char_map))) {
	    printf("\n\nNO Action Possible due to no matching rule, Aborting ! \n\n");
	    return INVALID;
	}
    	/*
     	 * Advance to next rule in same state
     	 */
	else {
	    if(action) {
	    	action = action->next;
	    }
	}
    }

    /*
     * Print final Tape Content
     */
    printf("\n\n%s \n", input_data->tape_data);
    return HALT;
}
/*
 * This function simulates Turing Test.
 * It performs following tasks sequentially:
 *	
 *	1. Read following input data from input file
 *		Starting Tape Content
 *		Start Position of Head on Tape COntent
 *		Start State
 *		halting State
 *		
 *		Action Table: Action table in stored in an array of linked lists
 *			      Each location in array correspond to each state.
 *			      Since there can be multiple actions/rules in one state, 
 *			      they are stored in SIngly Linked List
 */
halt_status_e
simulate_turing_mc(char *input_file)
{
    FILE *in_fptr = NULL;

    uint32 line_count = 0;
    uint32 i = 0;

    io_data_t 		input_data;
    wild_char_bitmap_t 	wild_char_bitmap;
    action_table_t 	*t;

    halt_status_e 	ret_status;
    parse_status_e 	parse_stat;

    if(NULL == input_file) {
	printf("Invalid Input file\n");
	return INVALID;
    }


    in_fptr = fopen(input_file, "rb+");
    if(NULL == in_fptr) {
	printf("Failed to open file %s\n", input_file);
	return INVALID;
    }

    /* read Initial Tape Content */
    read_tape_content(input_data.tape_data, in_fptr);

    /* Read initial head position */
    read_line_val(&input_data.head_pos, in_fptr);
    /* Read start State Index */
    read_line_val(&input_data.start_ind, in_fptr);
    /* Read halt State Index */
    read_line_val(&input_data.halt_ind, in_fptr);

    /*
     * Count number of action lines.
     * This will be used to allocate memory for action table
     */
    line_count = count_action_lines(input_file);

    /*
     * Allocate memory for action table.
     * This is an array of pointers. This function only allocates memory for an array.
     * Individual elements in array are allocated as an when required depending on if
     * respective action is present or not.
     *
     */
    input_data.act_tbl = alloc_action_table(line_count);
    
    /*
     * This bitmap (32 bits) will indiate if 'Wild-Char' rule is present for a state or not.
     * bit   1 : Wild-Char rule present
     *	     0 : Wild-Char rule NOT present
     */
    CLEAR_BITMAP(wild_char_bitmap.wild_char_bit_pos);

    /*
     * Fill action table based on rules found in input file
     */
    parse_stat = fill_action_table(in_fptr, 
	    			input_data.act_tbl, 
				&wild_char_bitmap.wild_char_bit_pos, 
				line_count);

    /*
     * Report parsig failure and exit with non-zero code
     */
    if(PARSE_SUCCESS != parse_stat) {
    	fclose(in_fptr);
    	dealloc_action_table(input_data.act_tbl, line_count);
	return INVALID; 
    }

    /* Print Action table */
#ifdef PRINT_TABLE
    printf("\n\tState\tCharacter Read\tCharacter Written\tDirection\tNew State\n");
    printf("     -----------------------------------------------------------------------------\n");
    for(i = 0; i<line_count; i++) {
	if(input_data.act_tbl[i]) {
	    t = input_data.act_tbl[i];
	    while(t) {
		printf("\t%d \t     %c \t\t\t %c \t\t", 
			t->cur_state,
			t->read_char,
			t->write_char);
		if(t->dir == 0)
		    printf("LEFT");
		if(t->dir == 1)
		    printf("STAY");
		if(t->dir == 2)
		    printf("RIGHT");
		printf("\t\t    %d\n", t->new_state);
		t = t->next;
	    }
	}
    }
#endif

    /* 
     * Run Turing Test 
     */
    ret_status = _turing_test(&input_data, 
	    		     input_data.act_tbl, 
			     wild_char_bitmap.wild_char_bit_pos); 

    /* CLose input file */
    fclose(in_fptr);
    
    /* Free all dynamically allocated memory */
    dealloc_action_table(input_data.act_tbl, line_count);

    return ret_status;
}
/*
 * This function prints the error if input in incorrect.
 * Also suggests the correct input format
 */
void usage()
{
    printf("\nPlease Porvide an input file as argument !\n");
    printf("Example: \n");
    printf("\t./a.out input.txt\n");
}
/*
 * main driver function for running Turin Machine Simulator
 */
int main(int argc, 
	 char *argv[])
{
    halt_status_e turing_test_status;

    if(argc < 2) {
	usage();
	exit(1);
    }

    turing_test_status = simulate_turing_mc(argv[1]);

    if(NO_HALT == turing_test_status ||
       INVALID == turing_test_status) {
	
	printf("Turing test Exiting with error\n");
	exit(1);

    }

    return 0;
}
