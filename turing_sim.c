#include "turing_sim.h"

#define CLEAR_BITMAP(map) {\
    map = 0x00000000;\
}

#define SET_WILD_CHAR_PRESENT_BIT_IN_STATE(pos, map) {\
    map |= (0x01 << pos);\
}

#define GET_WILD_CHAR_PRESENT_BIT_IN_STATE(pos, map) (map & (0x01 << pos))

action_table_t **
alloc_action_table(int line_count)
{
    int i = 0;

    action_table_t **act_tbl;

    act_tbl = (action_table_t **) malloc (sizeof(action_table_t) * line_count);
    memset(act_tbl, 0, sizeof(action_table_t) * line_count);

    printf("\n Alloced ");
    return act_tbl;
}
dealloc_action_table(action_table_t **act_tbl, int line_count)
{
    action_table_t *cur, *next;
    int i = 0;
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
    printf("\nFreed");
}

action_table_t* 
read_action_line(char c, uint32 *wild_char_bit_pos, FILE *fptr)
{
    uchar next;
    action_table_t *act_tbl = (action_table_t *) malloc (sizeof(action_table_t));
    //1 - a -1 3
    act_tbl->cur_state = atoi(&c); //1

    c = fgetc(fptr); // SPACE

    c = fgetc(fptr); //-
    act_tbl->read_char = c;
    if('*' == c) {
	SET_WILD_CHAR_PRESENT_BIT_IN_STATE(act_tbl->cur_state, *wild_char_bit_pos);
    }

    c = fgetc(fptr);//SPACE

    c = fgetc(fptr);//a
    act_tbl->write_char = c;

    c = fgetc(fptr);//SPACE

    c = fgetc(fptr);//-
    if(c == '-') {
	next = fgetc(fptr);
	if(next == '1') {
	    act_tbl->dir = LEFT;
	}
	else {
	    ///error "- shoudl be followd by 1 to indicate LEFT direction"
	    printf("\n Action line paring error: Invalid Direction");
	    act_tbl->dir = INVALID_DIR;
	    free(act_tbl);
	    act_tbl = NULL;
	    return NULL;
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
	printf("\n Action line paring error: Invalid Direction");
	act_tbl->dir = INVALID_DIR;
	free(act_tbl);
	act_tbl = NULL;
	return NULL;
    }
    c = fgetc(fptr);//SPACE

    c = fgetc(fptr);//3
    act_tbl->new_state = atoi(&c);

    act_tbl->next = NULL;

    return act_tbl;
}
parse_status_e 
fill_action_table(FILE *fptr, 
			action_table_t **act_tbl,
			uint32 *wild_char_bit_pos,
			int line_count)
{
    char c;

    int i = 0;
    action_table_t *node = NULL, *temp = NULL;

    while(!feof(fptr) && line_count) {

	while((c = fgetc(fptr)) != '\n') {
	    i = atoi(&c);
	    node = read_action_line(c, wild_char_bit_pos, fptr);
	    if(NULL == node) {
		return PARSE_FAIL; 
	    }
	    if(act_tbl[i] == NULL) {
		//Head
		act_tbl[i] = node;
	    }
	    else {
		temp = act_tbl[i];
		while(temp->next) {
		    temp = temp->next;
		}
		temp->next = node;
		//append at the end assuming all wild character actions come last
	    }
	    
	}
	line_count --;
    }

    return PARSE_SUCCESS;
}
uint32 count_action_lines(uchar *input_file) 
{
    uint32 line_count = 0;
    FILE   *fptr;
    char c;

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
    return line_count - 4;
}
void read_tape_content(uchar *read_buf, FILE *fptr) 
{
    char c;
    int ind = 0 ;

    memset(read_buf, 0, TAPE_DATA_BUFFER);

    while((c = fgetc(fptr)) != '\n') {
	read_buf[ind] = c;
	ind++;
    }
    read_buf[ind] = '\0';
}

void read_line_val(uint32 *read_val, FILE *fptr) 
{
    char c;
    int multiplier = 10;

    *read_val = 0;

    while((c = fgetc(fptr)) != '\n') {
	*read_val = (*read_val * multiplier) + atoi(&c);
    }
}

#define MOVE_LEFT(c) c--

#define MOVE_RIGHT(c) c++

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
    dst = src;\
}
#define CHANGE_STATE(dst, src) {\
    dst = src;\
}
halt_status_e 
turing_test(io_data_t *input_data, action_table_t **act_tbl, uint32 wild_char_map)
{
    uchar *temp = input_data->tape_data;
    temp += input_data->head_pos;
    uint32 start_state = input_data->start_ind;
    uint32 end_state = input_data->halt_ind;

    uint32 curr_state = start_state;
    uint32 next_state;

    action_table_t *action = act_tbl[curr_state];

    while(curr_state != end_state && *temp != '\0') {
	if((action && 
	   action->cur_state == curr_state &&
	   action->read_char == *temp)) { 

	    WRITE_CHAR(*temp, action->write_char);
	    
	    MOVE_HEAD(temp, action->dir);
	    
	    CHANGE_STATE(curr_state, action->new_state);
	    
	    action = act_tbl[curr_state];
	    printf("\n tape date: %s temp: %c state: %d", input_data->tape_data, *temp, curr_state);
	}
	else if(action) {
	    action = action->next;
	}
	else if (GET_WILD_CHAR_PRESENT_BIT_IN_STATE(curr_state, wild_char_map)) {
	    action = act_tbl[curr_state];
	    if(action && 
	       action->cur_state == curr_state) { 

		WRITE_CHAR(*temp, action->write_char);

		MOVE_HEAD(temp, action->dir);

		CHANGE_STATE(curr_state, action->new_state);

		action = act_tbl[curr_state];
	    	printf("\n tape date: %s temp: %c state: %d", input_data->tape_data, *temp, curr_state);
	    }
	}
    }
    return HALT;
}
halt_status_e
simulate_turing_mc(uchar *input_file)
{
    FILE *in_fptr = NULL;
    FILE *temp;

    int line_count = 0;
    int i = 0;

    io_data_t input_data;
    wild_char_bitmap_t wild_char_bitmap;
    action_table_t *t;

    halt_status_e ret_status;
    parse_status_e parse_stat;

    if(NULL == input_file) {
	printf("Invalid Input file\n");
	return INVALID;
    }


    in_fptr = fopen(input_file, "rb+");
    if(NULL == in_fptr) {
	printf("Failed to open file %s\n", input_file);
	return INVALID;
    }

    read_tape_content(input_data.tape_data, in_fptr);

    read_line_val(&input_data.head_pos, in_fptr);
    read_line_val(&input_data.start_ind, in_fptr);
    read_line_val(&input_data.halt_ind, in_fptr);
    temp = in_fptr;
    line_count = count_action_lines(input_file);

    input_data.act_tbl = alloc_action_table(line_count);
    
    CLEAR_BITMAP(wild_char_bitmap.wild_char_bit_pos);

    parse_stat = fill_action_table(in_fptr, 
	    			input_data.act_tbl, 
				&wild_char_bitmap.wild_char_bit_pos, 
				line_count);

    if(PARSE_SUCCESS != parse_stat) {
    	fclose(in_fptr);
    	dealloc_action_table(input_data.act_tbl, line_count);
	return INVALID; 
    }
#if 1
    //printf("\n\naabbbbb\n0\n0\n9\n");
    printf("State\tCharacter Read\tCharacter Written\tDirection\tNew State\n");
    printf("----------------------------------------------------------------------------------\n");
    for(i = 0; i<line_count; i++) {
	if(input_data.act_tbl[i]) {
	    t = input_data.act_tbl[i];
	    while(t) {
		printf("\t%d \t     %c \t\t\t %c \t\t", 
			t->cur_state,
			t->read_char,
			t->write_char);
		if(t->dir == 0)
		    printf("LEFT");//printf("-1 ");
		if(t->dir == 1)
		    printf("STAY");//printf("0 ");
		if(t->dir == 2)
		    printf("RIGHT");//printf("1 ");
		printf("\t\t    %d\n", t->new_state);
		t = t->next;
	    }
	}
    }
#endif

    ret_status = turing_test(&input_data, 
	    		     input_data.act_tbl, 
			     wild_char_bitmap.wild_char_bit_pos); 

    fclose(in_fptr);
    dealloc_action_table(input_data.act_tbl, line_count);
    return ret_status;
}
void usage()
{
    printf("\nPlease Porvide an input file as argument !\n");
    printf("Example: \n");
    printf("\t./a.out input.txt\n");
}
int main(uint32 argc, 
	uchar *argv[])
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
