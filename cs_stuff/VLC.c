/// Variable Length Coding using a heap to effeciently encode arbitrary data
/// Author: Richie Sommers
/// Email: res3453@rit.edu
/// Class: CSCI243
/// Project 1
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "node_heap.h"

// List of all possible symbols we could read
Symbol symbols[MAX_SYMS];

// The heap we use to find the least frequent node and create the VLC
Heap heap;


/// prepend_bit adds the specified char (0 or 1) to the codeword
/// used exclusively in compute_vlc
/// @param codeword where we are adding the new character
/// @param to_add the character to prepend
static void prepend_bit(char * codeword, char to_add){
	char cur = to_add;
	for (int i = 0; i < MAX_CODE; i++){
		char tmp = codeword[i];
		codeword[i] = cur;
		cur = tmp;
		if (tmp == NUL){
			codeword[i+1] = NUL;
			break;
		}
	}
}


/**
Helper function that performs one iteration of the merging process outlined below
@param heap is the heap we are operating on
While there remains more than one node in the collection
    Identify the node, n1, with the lowest cumulative frequency
    Identify the node, n2, with the second lowest cumulative frequency

    Remove those nodes from the collection

    For each symbol object in the symbol object list of n1
        prepend a "0" to its existing codeword
    For each symbol object in the symbol object list of n2
        prepend a "1" to its existing codeword

    Create a new node with the cumulative frequency equal to the
        combined cumulative frequencies of n1 and n2, 
        and with the symbol object list equal to the concatenation 
        of the symbol object lists from n1 and n2,
    Add the new node back into the collection.
*/
static void compute_vlc_1step(Heap * heap){
	Node n1 = heap_remove(heap);
	Node n2 = heap_remove(heap);

	// prepend 0 to all of n1
	for (size_t i = 0; i < n1.num_valid; i++){
		prepend_bit(n1.syms[i].codeword, '0');
		n1.syms[i].bit++;
	}

	// prepend 1 to all of n2
	for (size_t i = 0; i < n2.num_valid; i++){
		prepend_bit(n2.syms[i].codeword, '1');
		n2.syms[i].bit++;
	}


	Node combined = 
		{ .frequency = n1.frequency + n2.frequency
		, .num_valid = n1.num_valid + n2.num_valid
		};

	// Combine list of symbols
	for (size_t i = 0; i < n1.num_valid; i++){
		combined.syms[i] = n1.syms[i];
	}
	for (size_t j = 0; j < n2.num_valid; j++){
		combined.syms[n1.num_valid + j] = n2.syms[j];
	}


	heap_add(heap, combined);
}
/// applies the vlc calculation scheme until it is minimally encoded
/// compute_vlc computes the variable length codes for symbols already put into the heap using the algorithm for combining the two lowest frequency nodes.
///
/// @param heap contains the nodes that contain the symbols to analyze.
/// @pre heap is valid, initialized and filled with symbol information.
static void compute_vlc(Heap * heap){
	while(heap->size > 1){
		compute_vlc_1step(heap);
	}
}

/// print codeword right-justified in a fixed width field to standard output.
/// @param width field width of the codeword
/// @param codeword	string of char values that are '0' or '1' for a bit string
void display_codeword(int width, const char codeword[]){
	printf( "codeword: %*s ", width, codeword);
}

/// helper function to format the symbol as a character, escape code, or hex code as applicable
/// @param c the character we are displaying
static void display_char_safe(char c){
	putchar('\'');
	if (c == '\t'){
		printf("\\t");
	} else if (c == '\n'){
		printf("\\n");
	} else if (isprint(c)) {
		putchar(c);
	} else if (c == 0x00){
		printf("0x00");
	}else {
		printf("%#04hhx", c);
	}
	putchar('\'');
	putchar(' ');
}

/// print the symbol information to standard output as a formatted line entry.
/// @param sym pointer to a Symbol object
/// @param maxcode_len	number of 'bits' in Symbol's computed maximum codeword
void display_symbol(const Symbol * s, size_t maxcode_len){

	printf("symbol: "); 
	display_char_safe(s->symbol);
	printf("\tfrequency:\t\t%ld\t", s->frequency);
	display_codeword(maxcode_len, s->codeword);
	printf("\n");
}

/// display_node draws a table describing the codewords
/// and symbols of a VLC node.
/// it should be called on the base VLC tree node
/// in order to print all symbols
/// @param node the root node of the VLC tree
void display_node(const Node * node){
	for (size_t i = 0; i < node->num_valid; i++){
		display_symbol(&(node->syms[i]), 4);
	}
}

/// vlc_stats is a holder for all of the stats we need to print out at the end
/// it is returned by calculate_stats
typedef struct vlc_stats_s{
	double average_vlc_code_len;
	double fixed_length_code_len;
	int longest_variable_code_len;
	int node_cumul_frequency;
} vlc_stats;


/// avg_code_length returns average code length for Symbols stored in the node.
/// @param node	a single node containing the results of analyzing an input stream of symbols.
double avg_code_length(const Node * node){
	double length_avg = 0.0;
	for (size_t i = 0; i<node->num_valid; i++){
		Symbol sym = node->syms[i];
		int size = sym.bit;
		length_avg += size * sym.frequency;
	}
	return length_avg / node->frequency;
}


/// helper function to calculate all of the stats we need to display at the end
/// @param node the root node of the VLC 'tree'
/// @return the calculated stats enclosed in a vlc_stats struct
static vlc_stats calculate_stats(Node * node){
	vlc_stats ret;
	ret.longest_variable_code_len = 0;
	for (size_t i = 0; i<node->num_valid; i++){
		Symbol sym = node->syms[i];
		int size = sym.bit;
		if (size > ret.longest_variable_code_len){
			ret.longest_variable_code_len = size;
		}
	}
	ret.node_cumul_frequency = node->frequency;
	ret.average_vlc_code_len = avg_code_length(node);
	ret.fixed_length_code_len = (int)ceil(log2((double)node->num_valid));
	return ret;
}

/// main reads all the symbols from stdin, creates a Variable length encoding, then outputs its statistics
int main( void ){
	// setup
	heap_init(&heap);
	int num_unique = read_symbols(MAX_SYMS, symbols);
	heap_make(&heap, num_unique, symbols);

	compute_vlc(&heap);

	printf("Variable Length Code Information\n");
	printf("================================\n");

	display_node(&heap.array[0]);
	printf("\n");
	vlc_stats stats = calculate_stats(&heap.array[0]);
	printf("Average VLC code length:\t%.4f\n", stats.average_vlc_code_len);
	printf("Fixed length code length:\t%.4f\n", stats.fixed_length_code_len);
	printf("Longest variable code length:\t%d\n", stats.longest_variable_code_len);
	printf("Node cumulative frequency:\t%d\n", stats.node_cumul_frequency);
	printf("Number of distinct symbols:\t%d\n", num_unique);


	return EXIT_SUCCESS;
}
