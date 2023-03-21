// 
// File: test_heap.c 
// 
// test_heap.c tests the node_heap module using random numbers for
// frequency values in the Node structures added/removed from the node heap.
// 
// @author bks: ben k steele
// 
// // // // // // // // // // // // // // // // // // // // // // // // 

#define _DEFAULT_SOURCE    // for srandom

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "node_heap.h"

/// returns the index of the parent of location i.
#define Parent_Index( i ) ( i - 1 ) / 2

/// returns the index of the left child of location i.
#define Left_Child_Index( i )     ( i * 2 ) + 1

/// returns the index of the right child of location i.
#define Right_Child_Index( i )    ( i * 2 ) + 2

/// generate_randoms fills values with randoms between 0 and 8 times the count.

static void generate_randoms( size_t count, size_t values[] ) {

    size_t max = count << 3;
    for ( size_t i = 0; i < count; ++i ) {
        values[i] = (size_t)(random() % max );
    }
}

/// add_random adds count Nodes containing given frequencies

static void add_random( Heap * heap, size_t count, size_t freqs[] ) {

    assert( heap );
    assert( count < heap->capacity );

    Node node ;

    for ( size_t i = 0; i < count; ++i ) {

        node.frequency = freqs[i];  // all the heap cares about!
        heap_add( heap, node );
    }
}

/// Display the heap as a tree with each child value indented
/// from its parent value. Traverse the heap tree in preorder.

static void display_heap( Heap * heap, size_t idx, int indent) {

    if ( idx < heap->size ) {

        printf( "%*c %zu\n", indent, ' ', heap->array[idx].frequency );

        display_heap( heap, Left_Child_Index( idx ), indent + 4 );
        display_heap( heap, Right_Child_Index( idx ), indent + 4 );
    }
}

/// Test_heap is the singleton global heap for the tester.

static Heap Test_heap;  // capitalized to identify as a module local value

/// test_heap1 tests filling and emptying a node Heap.
/// Test creates a heap, heap_adds random test data, displays it,
/// then heap_removes all the data and displays again (empty).
///
static void test_heap1( size_t count ) {

    // Tester does not test heap_make, which manipulates Symbols inside Node.
    // Instead the test uses heap_add and heap_remove, which only process
    // the Node fields.
    // Making the heap's Node instances hold symbols correctly is left
    // as part of the assignment exercise.

    printf( "begin test_heap1( %zu ):=============================\n", count );

    heap_init( &Test_heap );

    size_t values[count];

    generate_randoms( count, values );

    printf( "adding... " );
    add_random( &Test_heap, count, values );

    printf( "Heap size is now %zu\n", Test_heap.size );
    printf( "display_heap...\n" );
    display_heap( &Test_heap, 0, 0 );

    Node node;

    printf( "Removing node frequencies in order... " );
    size_t n = Test_heap.size;
    for ( size_t i = 0; i < n; ++i ) {

        node = heap_remove( &Test_heap );
        if ( i % 10 == 0 ) printf( "\n" );
        printf( "%zu ", node.frequency );
    }
    printf( "\n" );
    printf( "\nHeap size is now %zu\n", Test_heap.size );
    printf( "display_heap...\n" );
    display_heap( &Test_heap, 0, 0 );
    printf( "\nend test_heap1()\n" );
}

/// test_heap2 tests filling and partly emptying a node Heap.
///
static void test_heap2( size_t count ) {

    printf( "begin test_heap2( %zu ):=============================\n", count );

    heap_init( &Test_heap );

    size_t values[count];

    generate_randoms( count, values );

    printf( "adding... " );
    add_random( &Test_heap, count, values );

    printf( "Heap size is now %zu\n", Test_heap.size );
    printf( "display_heap...\n" );
    display_heap( &Test_heap, 0, 0 );

    Node node;

    size_t n = (size_t)(random() % ( count ) );

    printf( "Removing %zu nodes ... ", n );
    for ( size_t i = 0; i < n; ++i ) {

        node = heap_remove( &Test_heap );
        if ( i % 10 == 0 ) printf( "\n" );
        printf( "%zu ", node.frequency );
    }
    printf( "\n" );
    printf( "\nHeap size is %zu. ", Test_heap.size );
    printf( " and should be %zu\n", count - n );

    printf( "added 2 more entries, Big and Small.\n" );
    node.frequency = 8888;
    heap_add( &Test_heap, node );
    node.frequency = 0;  // smallest value for size_t (unsigned)
    heap_add( &Test_heap, node );
    printf( "display_heap...\n" );
    display_heap( &Test_heap, 0, 0 );

    node = heap_remove( &Test_heap );
    printf( "\nRemoved entry frequency is %zu. ", node.frequency );
    printf( " and should be %d\n", 0 );
    printf( "\nend test_heap2()\n" );
}

/// main function runs a test suite on the node_heap module implementation.
/// @returns 0 for no error

int main( void ) {

    srandom( 63 ); // seed the generator
    int cases[] = { 1, 3, 8, 21, 255 };
    for ( size_t j = 0; j < sizeof( cases ) / sizeof( int ); ++j ) {

        test_heap1( cases[j] );
        test_heap2( cases[j] );
    }
    return 0 ;
}

