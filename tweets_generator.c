#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "markov_chain.h"
#include "linked_list.h"

#define NONE 0
#define DECIMAL_BASE 10
#define ARGC_ALL_WORDS 4
#define ARGC_SOME_WORDS 5
#define SEED_ARGV argv[1]
#define TWEETS_NUM_ARGV argv[2]
#define CORPUS_PATH_ARGV argv[3]
#define WORDS_TO_READ_ARGV argv[4]
#define TWEET_LENGTH 1000
#define READ_ALL_WORDS (-1)
#define WORDS_TO_ADD 19
#define END_WORD_LEN 1
#define WRONG_ARGC "USAGE: <seed> <num of tweets> <path> <num of words>\n"
#define WRONG_PATH "ERROR: The given file path doesn't exist, or you do not "\
"have permission.\n"
#define READ "r"
#define SEPARATOR " \n\r"
#define TWEET_NUM "Tweet %d: "

///This function creates a new markov chain.
static void create_markov_chain(MarkovChain* markov_chain, LinkedList* list);

///This function checks whether a given path is valid
static bool path_validator(FILE * tweets_corpus);

///This function checks whether the provided number of arguments is valid
static int argc_validator(int argc, char* argv[]);

///This function fills the words database with words from a given file
///(the LEARNING STAGE)
static int fill_database(FILE* fp, int words_to_read,MarkovChain
*markov_chain);

///This function adds all words of a given line into the database (is used
/// by fill_database).
static int add_all_words(MarkovChain * markov_chain, char* word,\
int*added_words,int words_to_read);

///This function generates random tweets from the database, and prints them.
static void tweets_generator(MarkovChain* markov_chain, long tweets_num);

///This function prints a single word that is part of a created tweet
static void print_str(void* to_print);

///This function compares between two given strings
static int comp_str(void* a, void* b);

///This function frees a markov_node data
static void free_str(void* to_free);

///This word copies a string using strcpy
static void* copy_str(void* to_copy);

///This function checks if a given word is a last word (containing a dot)
static bool checkdot(void* data);

int main(int argc,char *argv[])
{
  FILE* tweets_corpus = fopen(CORPUS_PATH_ARGV,READ);
  if(path_validator(tweets_corpus) == false)
  {
    return EXIT_FAILURE;
  }
  int words_to_read = argc_validator(argc, argv);
  if(words_to_read == NONE)
  {
    fclose(tweets_corpus);
    return EXIT_FAILURE;
  }
  LinkedList *appearance_list = malloc(sizeof(LinkedList));
  MarkovChain* markov_chain = malloc (sizeof(MarkovChain));
  if(appearance_list == NULL || markov_chain == NULL)
  {
    free(markov_chain);
    free(appearance_list);
    fclose(tweets_corpus);
    return EXIT_FAILURE;
  }
  create_markov_chain (markov_chain, appearance_list);
  if(fill_database(tweets_corpus,words_to_read,markov_chain) == EXIT_FAILURE)
  {
    printf (ALLOCATION_ERROR_MASSAGE);
    free_markov_chain (&markov_chain);
    fclose (tweets_corpus);
    return EXIT_FAILURE;
  }
  srand(strtol(SEED_ARGV,NULL,DECIMAL_BASE));
  tweets_generator(markov_chain,strtol(TWEETS_NUM_ARGV,NULL,DECIMAL_BASE));
  free_markov_chain(&markov_chain);
  fclose(tweets_corpus);
  return EXIT_SUCCESS;
}

static void create_markov_chain(MarkovChain* markov_chain, LinkedList* list)
{
  markov_chain->database = list;
  markov_chain->print_func = print_str;
  markov_chain->comp_func = comp_str;
  markov_chain->free_data = free_str;
  markov_chain->copy_func =copy_str;
  markov_chain->is_last = checkdot;
}

bool path_validator(FILE *tweets_corpus)
{
  if(tweets_corpus == NULL)
  {
    printf(WRONG_PATH);
    return false;
  }
  return true;
}

static int argc_validator(int argc, char* argv[])
{
  if(argc != ARGC_ALL_WORDS  && argc != ARGC_SOME_WORDS)
  {
    printf(WRONG_ARGC);
    return NONE;
  }
  return (argc == ARGC_ALL_WORDS) ? READ_ALL_WORDS : (int) strtol
      (WORDS_TO_READ_ARGV,NULL,DECIMAL_BASE);
}

static int fill_database(FILE* fp, int words_to_read,MarkovChain *markov_chain)
{
  char new_tweet[TWEET_LENGTH];
  int added_words = 0;
  while(fgets(new_tweet,TWEET_LENGTH,fp) != NULL && (words_to_read < 0 || \
  added_words < words_to_read))
  {
    char* word =strtok(new_tweet,SEPARATOR);
    if(word == NULL)
    {
      continue;
    }

    if(add_all_words (markov_chain,word,&added_words,words_to_read)\
    ==EXIT_FAILURE)
    {
      return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}

static int add_all_words(MarkovChain * markov_chain, char* word, int*
added_words,\
int words_to_read)
{
  Node* previous_node = NULL;
  while(word != NULL && (words_to_read == READ_ALL_WORDS || \
  *added_words <words_to_read))
  {
    Node *new_node = add_to_database(markov_chain,word);
    if(new_node == NULL)
    {
      return EXIT_FAILURE;
    }
    if(previous_node != NULL && markov_chain->is_last\
    (previous_node->data->data) ==false)
    {
      if(add_node_to_counter_list (previous_node->data,\
      new_node->data,markov_chain)==false)
      {
        return EXIT_FAILURE;
      }
    }
    (*added_words)++;
    if(*added_words == words_to_read)
    {
      return EXIT_SUCCESS;
    }
    char *word_ptr =strtok(NULL,SEPARATOR);
    word = word_ptr;
    previous_node = new_node;
  }
  return EXIT_SUCCESS;
}

static void tweets_generator(MarkovChain* markov_chain, long tweets_num)
{
  for(int index = 1;index <= tweets_num;index++)
  {
    printf(TWEET_NUM, index);
    generate_random_sequence (markov_chain,NULL,WORDS_TO_ADD);
  }
}

static void print_str(void* to_print)
{
    if(checkdot(to_print) == true)
    {
        printf ("%s", (char*) to_print);
    }
    else
    {
        printf ("%s ", (char*) to_print);
    }
}

static int comp_str(void* a, void* b)
{
  return strcmp((char*) a,(char*) b);
}

static void free_str(void* to_free)
{
  free(to_free);
  to_free = NULL;
}

static void* copy_str(void* to_copy)
{
  char *new_data = (char*) to_copy;
  char* dest = malloc(strlen(new_data)+1);
  if(dest == NULL)
  {
    return NULL;
  }
  strcpy(dest,to_copy);
  return dest;
}

static bool checkdot(void* data)
{
  char* as_str = (char*) data;
  return (as_str[strlen(as_str)-END_WORD_LEN] == '.');
}