#include <string.h> // For strlen(), strcmp(), strcpy()
#include "markov_chain.h"

#define MAX(X, Y) (((X) < (Y)) ? (Y) : (X))
#define EMPTY -1
#define BOARD_SIZE 100
#define MAX_GENERATION_LENGTH 60
#define DICE_MAX 6
#define NUM_OF_TRANSITIONS 20
#define LAST_CELL 100
#define DECIMAL_BASE 10
#define ROUNDS strtol (argv[2],NULL,DECIMAL_BASE)
#define SEED strtol(argv[1],NULL,DECIMAL_BASE)
#define CORRECT_ARGC 3
#define EQUAL 0
#define NOT_EQUAL (-1)
#define WRONG_ARGC_MSG "Usage: The program require the following parameters"\
"\n1) Seed number.\n 2) Number of lines to generate.\n"
#define TO_PRINT "[%d] -> "
#define TO_PRINT_WITH_SNAKE "[%d]-snake to %d -> "
#define TO_PRINT_WITH_LADDER "[%d]-ladder to %d -> "
#define TO_PRINT_LAST "[%d]"

///This function validates that the given argc is 2
static int validate_argc(int argc);

///This function create the markov chain with the relevant inner functions.
static void create_markov_chain(MarkovChain* markov_chain, LinkedList* list);

///This function prints a given cell number, and a snake/ladder from it
/// (if exists).
static void print_cell(void* to_print);

///This function compares between 2 cells (by their number)
static int comp_cell(void* a, void* b);

///This function frees a malloced cell
static void free_cell(void* to_free);

///This function copy data to a cell
static void* copy_cell(void* to_copy);

///This function checks whether a given cell is the last on board
static bool is_last_cell(void* data);

/**
 * represents the transitions by ladders and snakes in the game
 * each tuple (x,y) represents a ladder from x to if x<y or a snake otherwise
 */
const int transitions[][2] = {{13, 4},
                              {85, 17},
                              {95, 67},
                              {97, 58},
                              {66, 89},
                              {87, 31},
                              {57, 83},
                              {91, 25},
                              {28, 50},
                              {35, 11},
                              {8,  30},
                              {41, 62},
                              {81, 43},
                              {69, 32},
                              {20, 39},
                              {33, 70},
                              {79, 99},
                              {23, 76},
                              {15, 47},
                              {61, 14}};

/**
 * struct represents a Cell in the game board
 */
typedef struct Cell {
    int number;
    int ladder_to;
    int snake_to;
} Cell;

/** Error handler **/
static int handle_error(char *error_msg, MarkovChain **database)
{
  printf("%s", error_msg);
  if (database != NULL)
  {
    free_markov_chain(database);
  }
  return EXIT_FAILURE;
}

static int create_board(Cell *cells[BOARD_SIZE])
{
  for (int i = 0; i < BOARD_SIZE; i++)
  {
    cells[i] = malloc(sizeof(Cell));
    if (cells[i] == NULL)
    {
      for (int j = 0; j < i; j++) {
        free(cells[j]);
      }
      handle_error(ALLOCATION_ERROR_MASSAGE,NULL);
      return EXIT_FAILURE;
    }
    *(cells[i]) = (Cell) {i + 1, EMPTY, EMPTY};
  }

  for (int i = 0; i < NUM_OF_TRANSITIONS; i++)
  {
    int from = transitions[i][0];
    int to = transitions[i][1];
    if (from < to)
    {
      cells[from - 1]->ladder_to = to;
    }
    else
    {
      cells[from - 1]->snake_to = to;
    }
  }
  return EXIT_SUCCESS;
}

/**
 * fills database
 * @param markov_chain
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
static int fill_database(MarkovChain *markov_chain)
{
  Cell* cells[BOARD_SIZE];
  if(create_board(cells) == EXIT_FAILURE)
  {
    return EXIT_FAILURE;
  }
  MarkovNode *from_node = NULL, *to_node = NULL;
  size_t index_to;
  for (size_t i = 0; i < BOARD_SIZE; i++)
  {
    add_to_database(markov_chain, cells[i]);
  }

  for (size_t i = 0; i < BOARD_SIZE; i++)
  {
    from_node = get_node_from_database(markov_chain,cells[i])->data;

    if (cells[i]->snake_to != EMPTY || cells[i]->ladder_to != EMPTY)
    {
      index_to = MAX(cells[i]->snake_to,cells[i]->ladder_to) - 1;
      to_node = get_node_from_database(markov_chain, cells[index_to])
          ->data;
      add_node_to_counter_list(from_node, to_node, markov_chain);
    }
    else
    {
      for (int j = 1; j <= DICE_MAX; j++)
      {
        index_to = ((Cell*) (from_node->data))->number + j - 1;
        if (index_to >= BOARD_SIZE)
        {
          break;
        }
        to_node = get_node_from_database(markov_chain, cells[index_to])
            ->data;
        add_node_to_counter_list(from_node, to_node, markov_chain);
      }
    }
  }
  // free temp arr
  for (size_t i = 0; i < BOARD_SIZE; i++)
  {
    free(cells[i]);
  }
  return EXIT_SUCCESS;
}

/**
 * @param argc num of arguments
 * @param argv 1) Seed
 *             2) Number of sentences to generate
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */

int main(int argc, char *argv[])
{
  if(validate_argc (argc) == EXIT_FAILURE)
  {
    return EXIT_FAILURE;
  }
  LinkedList* board = malloc (sizeof(LinkedList));
  MarkovChain* markov_chain = malloc (sizeof(Cell)*BOARD_SIZE);
  if(board == NULL || markov_chain == NULL)
  {
    free(board);
    free(markov_chain);
    printf (ALLOCATION_ERROR_MASSAGE);
    return EXIT_FAILURE;
  }
  create_markov_chain(markov_chain,board);
  if(fill_database(markov_chain) == EXIT_FAILURE)
  {
    printf (ALLOCATION_ERROR_MASSAGE);
    free_markov_chain (&markov_chain);
    return EXIT_FAILURE;
  }
  srand(SEED);
  for(int walk = 1; walk <= ROUNDS; walk++)
  {
    printf("Random Walk %d: [1] -> ",walk);
    generate_random_sequence(markov_chain,\
    markov_chain->database->first->data,MAX_GENERATION_LENGTH-1);
  }
  free_markov_chain (&markov_chain);
  return EXIT_SUCCESS;
}

static int validate_argc(int argc)
{
  if(argc != CORRECT_ARGC)
  {
    printf (WRONG_ARGC_MSG);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

static void create_markov_chain(MarkovChain* markov_chain, LinkedList* list)
{
  markov_chain->database = list;
  markov_chain->print_func = print_cell;
  markov_chain->comp_func = comp_cell;
  markov_chain->free_data = free_cell;
  markov_chain->copy_func =copy_cell;
  markov_chain->is_last = is_last_cell;
}

static void print_cell(void* to_print)
{
  Cell *cell = (Cell *) to_print;
  if(cell->number == LAST_CELL)
  {
    printf (TO_PRINT_LAST, cell->number);
  }
  else if(cell->snake_to != EMPTY)
  {
    printf (TO_PRINT_WITH_SNAKE,cell->number,cell->snake_to);
  }
  else if(cell->ladder_to != EMPTY)
  {
    printf (TO_PRINT_WITH_LADDER,cell->number,cell->ladder_to);
  }
  else
  {
    printf (TO_PRINT, cell->number);
  }
}

static int comp_cell(void* a, void* b)
{
  return (((Cell*) a)->number == ((Cell*) b)->number)? EQUAL : NOT_EQUAL;
}

static void free_cell(void* to_free)
{
  free(to_free);
  to_free = NULL;
}

static void* copy_cell(void* to_copy)
{
  Cell* new_data = (Cell *) to_copy;
  Cell * dest = malloc(sizeof(Cell));
  if(dest == NULL)
  {
    return NULL;
  }
  dest->number = new_data->number;
  dest->ladder_to = new_data->ladder_to;
  dest->snake_to = new_data->snake_to;
  return dest;
}

static bool is_last_cell(void* data)
{
  Cell* cell = (Cell*) data;
  return(cell->number == LAST_CELL);
}