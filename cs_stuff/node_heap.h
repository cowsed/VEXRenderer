//
// file: node_heap.h
// author: ben k steele, bksvcs@rit.edu
//
// The heap is an application-specific, minimum-ordered heap, whose 
// implementation is tailored to the variable-length coding application.
#ifndef NODE_HEAP_H
#define NODE_HEAP_H
#include <stdlib.h>

/// NUL is the 0, or null character, value in the ASCII code representation.
/// NUL is the value of an 'unused' symbol.
///
#define NUL   '\0'

/// NUM_BITS is the number of bits in the extended ASCII code representation.
///
#define NUM_BITS   8

/// MAX_SYMS is the maximum number of distinct symbols in a byte.
///
#define MAX_SYMS   256

/// MAX_CODE is the worst case code length
/// NOTE: this hard limit is based on empirical testing (longer than needed).
///
#define MAX_CODE  32

/// The Symbol structure stores:
/// <ul><li>the <code>symbol</code> a byte (character),
/// <li>its <code>codeword</code> representation, 
/// <li>the next usable <code>bit</code> in (AKA length of)
/// the <code>codeword</code> representation, and
/// <li>its <code>frequency</code> of occurrence in the source.</ul>
/// <p>Entries in a Symbol's codeword string with (index >= bit) are unused.
/// Each character symbol in a codeword that is <b>unused</b>
/// must be set/initialized to indicate that it is unused and a NUL byte.
/// <p>Each Symbol instance needs initialization before it can be used.
///
typedef struct Symbol_S {
    /// frequency is the frequency of this symbol.
    size_t frequency;

    /// index of the next bit in the codeword. range [0...MAX_CODE-1].
    size_t bit;

    /// symbol stores the character symbol being encoded (e.g. 'A').
    unsigned char symbol;

    /// codeword is the variable length encoding string (e.g. "1011").
    char codeword[MAX_CODE];
} Symbol;

/// The Node structure stores:
/// <ul><li> <code>frequency</code>, the cumulative frequency of occurrence
/// of symbols in the source text,
/// <li><code>num_valid</code>, the count of symbols that are valid/distinct,
/// and <li><code>syms</code>, the collection (an array) of symbols.</ul>
/// <p>Entries in a Node's syms array with (index > num_valid) are unused.
/// Any Symbol in a Node structure that is <b>unused</b>
/// must be set/initialized to indicate that it is unused and empty.
///
typedef struct Node_S {
    /// cumulative frequency of the symbol objects inside this Node.
    size_t frequency;

    /// number of distinct, valid symbols in syms or index of next to add.
    size_t num_valid;

    /// syms is an array whose capacity is the number of distinct symbols.
    Symbol syms[MAX_SYMS];

} Node;

/// The Heap struct holds a fixed-capacity array of Node structures
/// whose ordering value field is called 'frequency' (see struct Node_S).
/// <p>The heap structure stores:
/// <ul><li> <code>capacity</code>, the initialized maximum capacity,
/// <li><code>size</code>, the current number of valid entries in the heap,
/// and <li><code>array</code>, the Node structures.</ul>
/// <p>Entries in a heap's array between size and index capacity-1 are unused.
/// Any entry in a Heap structure that is <b>unused</b>
/// must be set/initialized to indicate that it is unused and empty.
///
typedef struct Heap_S {
    /// capacity is the capacity of the heap's array.
    size_t capacity;

    /// size is the current number of valid entries in the heap array.
    size_t size;

    /// array holds Node structures. MAX_SYMS nodes;
    Node array[MAX_SYMS];
} Heap;

// // // // // // // // // // // // // // // // // // // // // // // //
// // // // // // public Symbol, Node and Heap related functions
// // // // // // // // // // // // // // // // // // // // // // // //

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
int read_symbols( size_t maxcount, Symbol syms[] );

/// heap_init initializes the heap storage with all unused Node entries.
/// heap is a <em>pointer</em>, a reference to a heap structure.
/// @param heap a valid pointer to a Heap structure
/// @pre  heap is a valid pointer and heap->capacity is uninitialized.
/// @post  heap->array has been initialized with unused Node entries.
/// @post  heap->size == 0.
/// @post  heap->capacity is initialized to dimension of heap->array (q.v).
///
void heap_init( Heap * heap );

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
void heap_make( Heap * heap, size_t length, Symbol symlist[] );

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
void heap_add( Heap * heap, Node node );

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
Node heap_remove( Heap * heap );

#endif // NODE_HEAP_H
