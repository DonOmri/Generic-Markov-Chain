#include <stdlib.h>
#include <string.h>
#include "markov_chain.h"

Node* add_to_database(MarkovChain *markov_chain, void *data_ptr)
{
  void* data_cpy = markov_chain->copy_func(data_ptr);
  Node* is_data_here = get_node_from_database(markov_chain, data_cpy);
  if(is_data_here == NULL)
  {
    MarkovNode *new_markov = malloc(sizeof(MarkovNode));
    if(new_markov == NULL)
    {
      free(data_cpy);
      return NULL;
    }
    new_markov->data = data_cpy;
    new_markov->counter_list = NULL;
    new_markov->different_next = INIT_COUNTER_LIST;
    new_markov->total_size = INIT_COUNTER_LIST;
    add(markov_chain->database, new_markov);
    is_data_here = markov_chain->database->last;
  }
  else
  {
    markov_chain->free_data(data_cpy);
  }
  return is_data_here;
}

Node* get_node_from_database(MarkovChain *markov_chain, void *data_ptr)
{
  Node *cur = markov_chain->database->first;
  for(; cur != NULL; cur = cur->next)
  {
    if(markov_chain->comp_func(cur->data->data, data_ptr) ==0)
    {
      return cur;
    }
  }
  return NULL;
}

bool add_node_to_counter_list(MarkovNode *first_node, MarkovNode
*second_node, MarkovChain *markov_chain)
{
  for(int index = 0; index < first_node->different_next; index++)
  {
    if(markov_chain->comp_func(\
    first_node->counter_list[index].markov_node->data,second_node->data) ==0)
    {
      first_node->counter_list[index].frequency++;
      first_node->total_size++;
      return true;
    }
  }
  NextNodeCounter * temp = realloc(first_node->counter_list,\
  sizeof(NextNodeCounter)*(first_node->different_next+1));
  if(temp == NULL)
  {
    return false;
  }
  first_node->counter_list = temp;
  first_node->counter_list[first_node->different_next].markov_node=second_node;
  first_node->counter_list[first_node->different_next].frequency = 1;
  first_node->different_next++, first_node->total_size++;
  return true;
}

void free_markov_chain(MarkovChain ** ptr_chain)
{
  Node *cur = (*ptr_chain)->database->first;
  while (cur != NULL)
  {
    (*ptr_chain)->free_data(cur->data->data);
    free (cur->data->counter_list);
    cur->data->counter_list = NULL;
    free (cur->data);
    cur->data = NULL;
    Node *prev = cur;
    cur = cur->next;
    free (prev);
    prev = NULL;
  }
  free((*ptr_chain)->database);
  (*ptr_chain)->database = NULL;
  free(*ptr_chain);
  *ptr_chain = NULL;
}

MarkovNode* get_first_random_node(MarkovChain *markov_chain)
{
  Node *cur;
  do
  {
    int first_word_index = get_random_number ((markov_chain->database->size));
    cur = markov_chain->database->first;
    for(;first_word_index>0;first_word_index--)
    {
      cur= cur->next;
    }
  } while(markov_chain->is_last(cur->data->data) == true);
  return cur->data;
}

MarkovNode* get_next_random_node(MarkovNode *state_struct_ptr)
{
  int word_index = get_random_number (state_struct_ptr->total_size);
  NextNodeCounter *cur = state_struct_ptr->counter_list;
  while(word_index>0)
  {
    word_index -= cur->frequency;
    if(word_index<0)
    {
      break;
    }
    cur++;
  }
  return cur->markov_node;
}

void generate_random_sequence(MarkovChain *markov_chain, MarkovNode *
first_node, int max_length)
{
  if(first_node == NULL)
  {
    first_node = get_first_random_node (markov_chain);
    markov_chain->print_func(first_node->data);
  }
  for(int added = 0; added < max_length; added++)
  {
    first_node = get_next_random_node(first_node);
    markov_chain->print_func(first_node->data);
    if(markov_chain->is_last (first_node->data) == true)
    {
      break;
    }
  }
  printf("\n");
}

int get_random_number(int max_number)
{
  return rand() % max_number;
}