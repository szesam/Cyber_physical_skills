
typedef enum {		// Set of states enumerated
    NO_MOLE,
    ONE_MOLE
} state_e;

typedef enum {			// Set of events enumerated
    WHACK_WRONG,
    WHACK_RIGHT,
    IDLE
} event_e;

state_e state = NO_MOLE;	// Starting state
state_e next_state;
event_e event;
int points = 0;
int total_time;

int main()
{
    while(total_time > 0)			// When event occurs, event handler does some stuff
    {
        event = read_event();
        if (state == NO_MOLE)
        {
            int random_time = random_time_generator();
            // event handler returns NO_MOLE state
            if (event == WHACK_WRONG) next_state = NO_MOLE;
            // event handler returns NO_MOLE state
            if (event == WHACK_RIGHT) next_state = NO_MOLE;
            //event handler returns ONE_MOLE only if random_time is met, else return NO_MOLE
            if (event == IDLE) next_state = IDLE_event_handler(random_time); 
        }
        else if (state == ONE_MOLE)
        {
            int random_time = random_time_generator();
            // event handler returns NO_MOLE state and no points
            if (event == WHACK_WRONG) next_state = NO_MOLE;
            // event handler returns NO_MOLE state and add points
            if (event == WHACK_RIGHT) next_state = NO_MOLE;
            //event handler returns NO_MOLE only if random_time is met, else return ONE_MOLE
            if (event == IDLE) next_state = IDLE_event_handler(random_time); 
        }
        total_time--; //decrease time
    }
    return 0;
}