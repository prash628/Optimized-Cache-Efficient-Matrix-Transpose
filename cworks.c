/*
 * Name: Prashant Srinivasan
 * Andrew ID: psriniv1
*/
#include "cachelab.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>

#define ADDR_SIZE 64


struct node
{
    unsigned tag;
    struct node *nextNode;
};

int inputArg;
int s;             // Set Index
int S;             // Total Sets
int E;             // Associativity or cache lines
int b;             // The block offset bits
char *fileName;    // Trace File name
int hitCount;
int missCount;
int evictCount;
/*
* The following variable can be considered as an array of Sets
* These sets contain a pointer to the set of cache lines.
* These cache lines are linked lists of lines
*/
long **theCacheSet;

FILE *pFile;       //Pointer to trace file


int readCmdLineArgs(int argc, char** argv);  //This reads command line args
int readFromFile();                          //This reads from trace file
void modifyFunc(unsigned address);          //Performs modify operation
void loadFunc(unsigned address);            //Performs load operation
void storeFunc(unsigned address);          //Performs store operation
void setUpTheCache();                      //Allocates memory for cache
void freeTheCache();
int freeTheCacheLines(struct node *cacheLinesAddr);
int presenceInSet(struct node *cacheLinesAddr,struct node *newNodeAddr);
int nodeAccessedAgain(struct node *cacheLinesAddr,struct node *newNodeAddr);
void appendToEndOfSet(struct node *cacheLinesAddr,struct node *newNodeAddr);
int isSetFull(struct node *cacheLinesAddr);


int main(int argc, char** argv)
{
    int validArg;       //Checks the validity of commmand line arguments

    validArg=readCmdLineArgs(argc,  argv);
    if(validArg==0)
    {
        exit(1);  //Invalid Arguments;so exit
    }

    S=1<<s;
    theCacheSet=NULL;
    setUpTheCache();
    pFile = fopen (fileName,"r");  //Open File for reading
    if(pFile==NULL)
    {
        exit(1);  //Invalid File;so exit
    }
    readFromFile();
    freeTheCache();
    
    printSummary(hitCount,missCount,evictCount);
    return(0);


}

/*
* Purpose: It reads from cmd line and stores the result in s,E,b,fileName
* It returns 1 if all the arguments are valid
* It returns 0 if atleast one of them is not valid
*/
int readCmdLineArgs(int argc, char** argv)
{
    fileName = NULL;
    /* Looping over arguments
    * Reference: Recitation 05
    */
    while((inputArg = getopt(argc, argv, "s:E:b:t:v"))!=-1)
    {
        /* Determine which argument it is processing */
        switch(inputArg)
        {
        case 's':
            s=atoi(optarg);
            break;
        case 'E':
            E=atoi(optarg);
            break;
        case 'b':
            b=atoi(optarg);
            break;
        case 't':
            fileName=optarg;
            break;
        case 'v':
            //Verbosity is on
            break;
        default:
            //Wrong Argument
            exit(1);
        }
    } // End of while
    if(s==0 || E==0 || b==0 || fileName==NULL)
    {
        return(0);
    }
    else
    {
        return(1);
    }
}


/*
* Purpose: It reads from file and sets the program to work
* It returns 1 if all the arguments are valid
* It returns 0 if atleast one of them is not valid
*/
int readFromFile()
{
    char identifier;
    unsigned address;
    int size;

    /* Looping over arguments and reading lines like "M 20,1"
    * Reference: Recitation 05
    */
    while(fscanf(pFile,"%c %x,%d", &identifier, &address, &size)>0)
    {
        if(identifier=='M')
        {
            modifyFunc(address);
        }
        else if(identifier=='L')
        {
            loadFunc(address);
        }
        else if(identifier=='S')
        {
            storeFunc(address);
        }
        else
        {
            //Unwanted input
        }

    }
    fclose(pFile); // Close file when done
    return(0);
}

/*
* Purpose:
* 1. The function checks for presence in the set
* 2. If present, it ensures that it is seen that
*    it was accessed recently via linked list
* 3. If not, it adds it to the set and
*    3.1. By evicting LRU one if full
*    3.2. By appending to end of LinkedList if not full
*/
void loadFunc(unsigned address)
{
    struct node *newNodeAddr;
    unsigned setIndex;
    unsigned tag;
    struct node *cacheLinesAddr;
    struct node *tempLine;

    /* Get the setIndex by :
    * 1.masking the tag part
    * 2.Right shifting the block offset
    */
    setIndex=(address &((1<<(b+s))-1))>>b;

    tag=address>>(b+s);
    cacheLinesAddr=(struct node *)theCacheSet[setIndex];
    /*Create new node. It is created beforehand because:
    * it will have to be done anyways eventually
    */
    newNodeAddr=malloc(sizeof(struct node));
    if(newNodeAddr==NULL)
    {
        exit(1);
    }
    newNodeAddr->nextNode=NULL;
    newNodeAddr->tag=tag;
    if(presenceInSet(cacheLinesAddr,newNodeAddr))
    {
        hitCount++;
        if(cacheLinesAddr->tag==newNodeAddr->tag)
        {
            //If the node is the first node there
            if(cacheLinesAddr->nextNode!=NULL)
            {
                //If it is the only node there, do nothing
                tempLine=cacheLinesAddr->nextNode;
                free(cacheLinesAddr);
                theCacheSet[setIndex]=(long *)tempLine;
                appendToEndOfSet(tempLine,newNodeAddr);
            }
        }
        else
        {
            nodeAccessedAgain(cacheLinesAddr,newNodeAddr);
            /* The above code calls a function so that it does the
            *needful to ensure that it is seen that it was accessed recently
            */
        }

    }
    else
    {
        missCount++;
        if(isSetFull(cacheLinesAddr))
        {
            evictCount++;
            if(E>1)
            {
                //Eliminate the first one and then append the node to the end
                tempLine=cacheLinesAddr->nextNode;
                free(cacheLinesAddr);
                theCacheSet[setIndex]=(long *)tempLine;
                appendToEndOfSet(tempLine,newNodeAddr);
            }
            if(E==1)
            {
                cacheLinesAddr->tag=newNodeAddr->tag;
                free(newNodeAddr);
            }
        }
        else
        {
            if(cacheLinesAddr==NULL)
            {
                // If it is empty
                theCacheSet[setIndex]=(long *)newNodeAddr;
            }
            else
            {
                appendToEndOfSet(cacheLinesAddr,newNodeAddr);
            }
        }
    }

}

/*
* storeFunc and loadFunc can be considered to do the same here.
* So, it transfers control to loadFunc
*/
void storeFunc(unsigned address)
{
    loadFunc(address);
}


/*
* modifyFunc can be interpreted as loadFunc and then
* incrementing the hitCount.
*/
void modifyFunc(unsigned address)
{
    loadFunc(address);
    hitCount++;
}


/*
* Purpose: setUpTheCache does the following:
* 1. It sets up an array-this can be seen as Sets
* 2. They contain pointers to lines that are implemented as linked lists
* 3. Initially all these sets point to NULL
*/
void setUpTheCache()
{
    int i;
    theCacheSet=malloc(S*sizeof(long));
    if(theCacheSet==NULL)
    {
        exit(0);
    }
    // For each set, initialize them to point to NULL
    // This simulates the cold boot
    for(i=0; i<S; i++)
    {
        theCacheSet[i]=NULL;
    }
}

void freeTheCache()
{
    int i;
    for(i=0; i<S; i++)
    {
        freeTheCacheLines((struct node *)theCacheSet[i]);
    }
    free(theCacheSet);
}

/*
* Frees up the set of cache lines in a set by freeing them one after another
*/
int freeTheCacheLines(struct node *cacheLinesAddr)
{
    struct node *nextLine;
    struct node *temp;
    if(cacheLinesAddr==NULL)
    {
        return(1);
    }
    else
    {
        nextLine=cacheLinesAddr->nextNode;
        free(cacheLinesAddr);
        while(nextLine!=NULL)
        {
            temp=nextLine->nextNode;
            free(nextLine);
            nextLine=temp;
        }
        return(1);
    }
}

/*
*  If present in a given set, the function returns 1
*  Else 0
*/
int presenceInSet(struct node *cacheLinesAddr,struct node *newNodeAddr)
{
    struct node *nextLine;
    struct node *temp;
    if(cacheLinesAddr==NULL)
    {
        return(0);
    }
    else
    {
        if(cacheLinesAddr->tag==newNodeAddr->tag)
        {
            return(1);
        }
        nextLine=cacheLinesAddr->nextNode;
        while(nextLine!=NULL)
        {
            temp=nextLine->nextNode;
            if(nextLine->tag==newNodeAddr->tag)
            {
                return(1);
            }
            nextLine=temp;
        }
        return(0);
    }
}

/*
* This function is set up so that it is only called when the set is not empty
*/
void appendToEndOfSet(struct node *cacheLinesAddr,struct node *newNodeAddr)
{
    while(1)
    {
        if(cacheLinesAddr->nextNode==NULL)
        {
            cacheLinesAddr->nextNode=newNodeAddr;
            break;
        }
        cacheLinesAddr=cacheLinesAddr->nextNode;
    }
}


/*
* Purpose: The function is called when the location is already in the cache
* This deletes the node at that location and appends the node at the end
*
* Reason: This process simulates the LRU of the cache.
*         Because the node is accessed again, it is put at the end
*
* A precondition is that the node in question is not at the head
*/
int nodeAccessedAgain(struct node *cacheLinesAddr,struct node *newNodeAddr)
{
    struct node *prevNode;
    struct node *tempNode;
    struct node *headNode;
    
    headNode=cacheLinesAddr;
    //Edge case check: When there is just one node, nothing needs to be done
    if(cacheLinesAddr->nextNode!=NULL)
    {
        prevNode=cacheLinesAddr;
        cacheLinesAddr=cacheLinesAddr->nextNode;
        if(cacheLinesAddr->tag==newNodeAddr->tag)
        {
            tempNode=cacheLinesAddr->nextNode;
            free(cacheLinesAddr);
            prevNode->nextNode=tempNode;
            appendToEndOfSet(headNode,newNodeAddr);
            return(1);
        }
    }
    return(1);
}

/*
* The function checks if the number of cache lines in a given set have
* reached their limit or not
*/
int isSetFull(struct node *cacheLinesAddr)
{
    int counter;
    struct node *nextLine;

    counter=0;
    if(cacheLinesAddr!=NULL)
    {
        counter++;
        nextLine=cacheLinesAddr->nextNode;
        while(nextLine!=NULL)
        {
            counter++;
            nextLine=nextLine->nextNode;
        }
    }
    if(counter==E)
    {
        return(1);
    }
    else
    {
        return(0);
    }
}
