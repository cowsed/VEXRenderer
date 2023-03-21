/// Variable Length Coding using a heap to effeciently encode arbitrary data
/// Author: Richie Sommers
/// Email: res3453@rit.edu
/// Class: CSCI243
/// Project 1
#include <stdlib.h> //for exit()
#include <stdio.h>
#include "node_heap.h"

/// returns the index of the parent of location i.
#define Parent_Index( i ) ( i - 1 ) / 2

/// returns the index of the left child of location i.
#define Left_Child_Index( i )     ( i * 2 ) + 1

/// returns the index of the right child of location i.
#define Right_Child_Index( i )    ( i * 2 ) + 2


Node unused_node = {.frequency = 0, .num_valid = 0};


/// read_symbols reads characters from standard input, calculates
/// each symbol's frequency of appearance, and counts the number of
/// distinct symbols it sees.
/// @param maxcount the unsigned dimension of the syms array
/// @param syms array of Symbol structures initialized and filled
/// @pre  maxcount is a the capacity of the syms array.
/// @post  syms array is initialized and filled with histogram information.
/// @post  syms is ordered so that all used entries are before unused.
/// @post  unused syms array entries are initialized as unused.
///
int read_symbols( size_t maxcount, Symbol syms[] ){
	int c;
	size_t num_unique = 0;
	while((c = fgetc(stdin)) != -1){
		// search through list
		int updated = 0;
		for (size_t i = 0; i<num_unique; i++){
			if (i > maxcount){
				printf("Fatal error. More unique symbols read than we have space for\n");
				return -1;
			}
			// found it already
			if (syms[i].symbol == (char)c){
				syms[i].frequency++;
				updated = 1;
				break;
			}
		}

		// we found a new one
		if (!updated){
			syms[num_unique].frequency = 1;
			syms[num_unique].symbol = (char)c;
			num_unique++;
		}
	}
	return num_unique;
}


/// heap_init initializes the heap storage with all unused Node entries.
/// heap is a <em>pointer</em>, a reference to a heap structure.
/// @param heap a valid pointer to a Heap structure
/// @pre  heap is a valid pointer and heap->capacity is uninitialized.
/// @post  heap->array has been initialized with unused Node entries.
/// @post  heap->size == 0.
/// @post  heap->capacity is initialized to dimension of heap->array (q.v).
///
void heap_init( Heap * heap){
	heap->capacity = MAX_SYMS;
	heap->size = 0;


	for (size_t i = 0; i<MAX_SYMS; i++){
		heap->array[i] = unused_node;
	}
}


/// heap_make fills heap with symbols from symlist and <em>heapifies</em> it.
/// heap is a <em>pointer</em>, a reference to an initialized heap.
/// length is the length of the symlist.
/// <p>algorithm:
/// Take each symbol from symlist and put it into the
/// next note in the heap, initializing the node's frequency
/// to that of the symbol, and setting the node's num_valid to 1.
/// @param heap pointer to an initialized and unused heap
/// @param length unsigned length of symlist
/// @param symlist array of Symbol structures
/// @pre  entries in symlist[0, ..., length-1] are valid Symbol objects.
/// @pre  heap is an initialized, unused heap of capacity >= length.
/// @pre  length <= heap->capacity. symlist is array of Symbol structs.
/// @post  heap->array is filled with Node entries in min-heap order.
/// @post  heap->size == length.
/// @post  heap->array[i].syms[0] is distinct symbol for i in [0, ..., length-1].
///
void heap_make( Heap * heap, size_t length, Symbol symlist[] ){

	for (size_t i = 0; i< length; i++){
		Symbol sym = symlist[i];
		Node n = {.frequency = sym.frequency, .num_valid = 1, .syms = {sym}};
		heap_add(heap, n);
	}

}
/// compareFunc is the function used by the heap to order
/// for our purposes, it finnds the lowest frequency
/// @param a the first node to compare
/// @param b the second node to compare
/// @return true if a is less frequent than b
static int compareFunc(Node a, Node b){
	return a.frequency < b.frequency;
}

/// Move the value at start_index up to its proper spot in the given heap. Assume the value does not have to move down
/// @param heap the heap upon which we are operating
/// @param start_index the index at which the node we need to sift lies
static void sift_up(Heap * heap, size_t start_index){
	size_t i = start_index;
	Node * a = heap->array;

	while( i > 0 && !(compareFunc(a[Parent_Index(i)], a[i])) ){
		// swap up
		Node t = a[Parent_Index(i)];
		a[Parent_Index(i)] = a[i];
		a[i] = t;
		i = Parent_Index(i);
	}
}

/// heap_add adds one more node to the current heap.
/// heap_add replaces what is expected to be an unused Node entry.
/// @param heap pointer to an initialized heap structure being built
/// @param node a Node structure instance to add into the heap
/// @pre  heap->size < heap->capacity (fatal error to exceed heap capacity)
/// @post  heap->size has increased by 1.
/// @post  heap->array is in heap order.
/// <p>algorithm:
/// Since the heap->array is fixed capacity, heap_add cannot enlarge it;
/// instead the add overwrites an unused node in the heap structure.
/// The add puts the node's values into the next location in the heap's array.
/// <p>
/// After assigning the values, heap_add restores heap order
/// by sifting the last entry up the heap from the last location.
/// Finally it increments the size of the heap.
/// <p>
/// If there is no room for more nodes, then it is a fatal error.
/// However, the array is sized to hold enough nodes for all the symbols
/// it can encounter.
///
void heap_add( Heap * heap, Node node ){
	if (heap->size >= heap->capacity){
		printf("FATAL ERROR. ADDING TO FULL HEAP");
		exit(1);
	}

	heap->array[heap->size] = node;

	sift_up(heap, heap->size);

	// increment size of heap
	heap->size = heap->size + 1;
}


/// Helper function to find if the parent, left child, or right child is first
/// used for sifting down
/// @param heap the heap upon which we are operating
/// @param index the index of the parent node
/// @return the index of the node that would go first
static int first_of_3(Heap * heap, int index){
	size_t lt = Left_Child_Index(index);
	size_t rt = Right_Child_Index(index);
	Node thisVal = heap->array[index];

	if (rt < heap->size){ // both left and right children
		Node lVal = heap->array[lt];
		Node rVal = heap->array[rt];
		if (compareFunc(lVal, thisVal) || compareFunc(rVal, thisVal)){
			if (compareFunc(lVal, rVal)){
				return lt; // left child first
			} else {
				return rt; // right child first
			}
		} else {
			return index; // this one goes first
		}
	} else if (lt < heap->size){
		Node lVal = heap->array[lt];
		if (compareFunc(lVal, thisVal)){
			return lt; // left child first
		} else {
			return index; // i go first
		}

	} else {
		return index; // just me :)
	}
	return index; // just to make the compiler happy.
	// shouldn't actually get here. if returns early
}

/// Move the value at start_index down to its proper spot in the given heap. Assume the value does not have to move down
/// @param heap the heap upon which we are operating
/// @param start_index the index at which the node we need to sift lies
///
void siftDown(Heap * heap, int start_index){
	int curIndex = start_index;
	Node * a = heap->array;
	int swapIndex = first_of_3(heap, curIndex);
	while (swapIndex != curIndex){
		// swap
		Node tmp = a[curIndex];
		a[curIndex] = a[swapIndex];
		a[swapIndex] = tmp;
		curIndex = swapIndex;
		swapIndex = first_of_3(heap, curIndex);
	}
}

/// heap_remove removes and returns a node structure.
/// heap is a <em>pointer</em>, a reference to an initialized heap.
/// @param heap a pointer to the heap data structure
/// @return a Node structure that is the saved top entry
/// @pre  heap->size > 0 (fatal error to remove from an empty heap)
/// @post  heap->size has decreased by 1.
/// @post  remaining heap is in proper heap order.
///
/// <p>Algorithm:
/// <ul><li>
/// Save the top of the heap at index 0.
/// </li><li>
/// Reduce the size of the heap by
/// copying the last heap entry into the top location and
/// replacing the last location's entry with values representing an
/// unused Node object.
/// </li><li>
/// Restore the heap order by sifting the top entry down the heap.
/// </li><li>
/// Return the saved top entry.
/// </li></ul>
///
Node heap_remove( Heap * heap ){
	Node res = heap->array[0];
	heap->size = heap->size - 1;
	heap->array[0] = heap->array[heap->size];
	heap->array[heap->size] = unused_node;
	siftDown(heap, 0);
	return res;
}
