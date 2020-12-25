//The program below illustrates a Garbage Collection Programme.
//Most of the tests are also written below

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ObjectManager.h"
#include <assert.h>

//This is the refference to the next node
static Ref nextRef = 1;

//The node holds the data of memory
typedef struct NODE Node;


//Below is the different data for the garbage collection
//The size of the memory, reference, total references and next node pointer
//The data in the memory is followed up over here
//Struct Node Which stores information on where the memory is allocated:
//initialNodePoint - where the current block of memory starts in the buffer.
//byteSize - how much memory is allocated to the particular block .
//count - checks if the user is done or not with the use of the assigned block
//Ref - The reference of the particular block
//theNextNode - Details of where the next block of memory is.

struct NODE{

  int byteSize;    
  int initialNodePoint;   
  Ref ref;         
  int count;       
  Node *theNextNode;  

};


// The starting point, ending point and total number of allocated blocks
static Node *nodeStartingPoint = NULL; 
static Node *nodeFinish = NULL;   
static int allocatedBlocks = 0;           

//Below are the buffers, the active one and the one used in the compact part of the code
static unsigned char activeBuffer[MEMORY_SIZE];
static unsigned char compactBuffer[MEMORY_SIZE];

//The current buffer and the next available position for memory allocation
static unsigned char *currBuffer = activeBuffer;
static int blankPosition = 0;

//In the function below the memory block is initialized
//The count is incremented and address and reference are updated
static void initMemBlock( Node * const nodeP, const int bytes, const int addr, const Ref ref, Node * const theNext ){

  assert( bytes > 0 );
  
  assert( ref >= 0 );
  
  assert( addr >= 0 );
  
  assert( addr < MEMORY_SIZE );
  
  nodeP->byteSize = bytes;
  nodeP->initialNodePoint = addr;
  nodeP->ref = ref;
  nodeP->theNextNode = theNext;
  nodeP->count = 1;

}

//Below we are at the start of the list and continue updating the reference and address and returning the position of the node.
static Node *find( Ref ref ){
  
  Node *nodePosition = NULL;
  Node *myNode;
  myNode = nodeStartingPoint; 
  while ((myNode != NULL)  && (nodePosition == NULL)){

    if (myNode->ref == ref){

      nodePosition = myNode;
    }

    else{

      myNode=myNode->theNextNode;
    
    }
  }
  
  return nodePosition;
  
}

// My initPool function is as follows
//The program is set up here
void initPool(){
 
  nodeStartingPoint = NULL;
  nodeFinish = NULL;
  allocatedBlocks = 0;

}

// Destroying the nodes
void destroyPool(){
  
  Node *currentNode;
  Node *theNextNode;
  currentNode = nodeStartingPoint;
  
  while (currentNode != NULL){

    theNextNode = currentNode->theNextNode;
    free(currentNode);
    currentNode = theNextNode;
  }
  
  nodeStartingPoint = NULL;
  nodeFinish = NULL;
  allocatedBlocks = 0;
}

//In the function below, most of the garbage collection is done
//The data is iterated through entirely and garbage collection is done efficiently
//The Node position is determined and the buffer is allocated, updating the position along.
//And the Garbage collection information is shown
void compact(){
  
  unsigned char *initializeBuffer = (currBuffer==activeBuffer) ? compactBuffer : activeBuffer;
  int totalBytes = blankPosition;
  int totalNodes = 0;
  blankPosition = 0;
  Node *currentNode;  
  currentNode = nodeStartingPoint;

  while (currentNode != NULL){

    assert( currentNode->initialNodePoint >= 0 && currentNode->initialNodePoint < MEMORY_SIZE );
    assert( currentNode->byteSize >= 0 && currentNode->byteSize < (MEMORY_SIZE-blankPosition) );
    memcpy( &initializeBuffer[blankPosition], &currBuffer[currentNode->initialNodePoint], currentNode->byteSize );
    currentNode->initialNodePoint = blankPosition;
    blankPosition += currentNode->byteSize;
    currentNode = currentNode->theNextNode;
    totalNodes++;
  }
  
#ifndef NDEBUG

  printf( "\nHere is the Garbage Collection information:\n" );
  printf( "Number of objects: %d  Amount of bytes being used: %d  Amount of bytes that are freed: %d\n\n", totalNodes, blankPosition, (totalBytes-blankPosition) );

#endif

  currBuffer = initializeBuffer;
}

//Below the garbage collector is fed the data and a new object is made
//Depending on whether the garbage collection worked, the reference number is given out
Ref insertObject( ulong size ){

  Ref new_ref = NULL_REF;
  assert( size > 0 );
  if ( size >= (MEMORY_SIZE-blankPosition) )
    compact();
  
  if ( size < (MEMORY_SIZE-blankPosition) ){

    Node *myNode = (Node *)malloc(sizeof(Node));
    assert (myNode != NULL);

    if (myNode != NULL){

      new_ref = nextRef++;
      initMemBlock( myNode, size, blankPosition, new_ref, NULL );
      allocatedBlocks++;
      
      if (nodeFinish != NULL)
        nodeFinish->theNextNode = myNode;
      nodeFinish = myNode;
      
      if (allocatedBlocks == 1){

        nodeStartingPoint = myNode;
      
      }
      
      memset( &currBuffer[blankPosition], 0x00, size );
      blankPosition += size;
      assert( blankPosition <= MEMORY_SIZE );
    }
    
  }
  
  else
  {
#ifndef NDEBUG
    printf( "Garbage Collection was unsuccessful. Please try again.\n" );
#endif
  }
  return new_ref;
}

//In the function below we are tracking the data that is retrieved.
//And observing the position and address
void *retrieveObject( const Ref ref ){

  Node *nodePosition;
  void *nodeObject;
  assert( ref >= 0 );
  assert( ref < nextRef );
  nodePosition = find(ref);
  
  if (nodePosition != NULL){

    nodeObject = &currBuffer[nodePosition->initialNodePoint];
  
  }
  
  else{

    printf( "The reference %ld is invalid please try again.\n", ref );
    exit( EXIT_FAILURE );
    nodeObject = NULL;
  
  }
  
  return nodeObject;
}

//Below the position of the reference is updated
//The count is incremented
void addReference( Ref ref ){

  Node *nodePosition;
  assert( ref >= 0 );
  assert( ref < nextRef );
  nodePosition = find( ref );
  
  if (nodePosition != NULL){

    nodePosition->count = nodePosition->count + 1;
  
  }

}

//In the function below we are dropping the reference of a node that has already been passed
//The precondition and postconditions need to be met
//The position of the node is dropped
void dropReference( Ref ref ){

  Node *nodePosition;
  Node *previousNodee;
  Node *currentNode;
  assert( ref >= 0 );
  assert( ref < nextRef );
  nodePosition = find( ref );
  
  if (nodePosition != NULL){

    nodePosition->count = nodePosition->count - 1;
    
    if (nodePosition->count == 0){
    
      if (nodePosition == nodeStartingPoint){

        nodeStartingPoint = nodeStartingPoint->theNextNode;
        free(nodePosition);
      
      }
      
      else{

        previousNodee = nodeStartingPoint;
        currentNode = nodeStartingPoint->theNextNode;

        while (currentNode != nodePosition){

          previousNodee = currentNode;
          currentNode = currentNode->theNextNode;
        
        }
        
        if (currentNode == nodeFinish)
          nodeFinish= previousNodee;
        
        previousNodee->theNextNode = nodePosition->theNextNode;
        
        free(nodePosition);
      
      }
      
      allocatedBlocks--;
      if (allocatedBlocks <= 1)
        nodeFinish = nodeStartingPoint;
      
    }
  }
}

//In the function below we are cleaning up the object manager before exiting
//The nodes are cleared and the buffer is freed.
void dumpPool(){

  Node *currentNode;  
  currentNode = nodeStartingPoint;
  
  while (currentNode != NULL){

    printf( "The reference for the node is = %lu, The starting point of the node is = %d, The total bytes are = %d, The total count is = %d\n", currentNode->ref, currentNode->initialNodePoint, currentNode->byteSize, currentNode->count );
    currentNode=currentNode->theNextNode;
  
  }

  printf( "The possible position for the next node may be = %d\n", blankPosition );  

}