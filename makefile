COMPILE = gcc
DEFAULT_FLAGS = -g -Wvla -Werror -Wall

tweets: tweets_generator.c markov_chain.o linked_list.o
	$(COMPILE) $(DEFAULT_FLAGS) tweets_generator.c markov_chain.o linked_list.o -o tweets

markov_chain.o: markov_chain.c markov_chain.h linked_list.c linked_list.h
	$(COMPILE) -c markov_chain.c linked_list.c

linked_list.o: linked_list.c linked_list.h
	$(COMPILE) -c linked_list.c

snakes: snakes_and_ladders.c markov_chain.o linked_list.o
	$(COMPILE) $(DEFAULT_FLAGS) snakes_and_ladders.c markov_chain.o linked_list.o -o snakes
