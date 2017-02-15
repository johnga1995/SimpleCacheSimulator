

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <bitset>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


#include "CS_BinaryTree.h"

#define DEFAULT_ASSOCIATIVITY 1
#define DEFAULT_BLOCKSIZE_B 32
#define DEFAULT_CACHESIZE_KB (64*1024)
#define MAX_CACHE_SIZE (64*1024) // kb

#define MEMORY_ADDRESS_SIZE_BITS 32
#define MEMORY_ADDRESS_SIZE_HEX (MEMORY_ADDRESS_SIZE_BITS/4)

#define ADDRESS_LENGHT 32

#define ASSOCIATIVITY_FLAG "-a"
#define BLOCKSIZE_FLAG "-b"
#define UNIFIED_FLAG "-r"
#define TRACE_FLAG "-f"

#define DATA_LOAD 0
#define DATA_STORE 1
#define INSTRUCTION_LOAD 2


typedef struct CacheBlock{
	int validationBit;
	int tag;
	int data;
	int lruCounterVal; // zero value denots least recently useds

}cacheData;

/* Implementation of CacheSimulator class */
class CacheSimulator{

public:
	CacheSimulator(void){ initializeAllDefaultVariables(); };
	~CacheSimulator(void){};

	void decodeAndSetParameters(int argc, char **argv);
	void printCacheState();
	bool createCacheAndSetUpAddressVariables();
	void readAndProcessFile();
	void updateCacheLRU(std::string line);
	void updateCachePseudoLRU(std::string line);

	std::string cs_traceFilename;

	unsigned int cs_numberOfMisses;
	unsigned int cs_totalInstructions;

private:
	// Fields used for user input
	int cs_cacheSize; // total size of cache
	int cs_associativity; //number of blocks in set 
	int cs_blockSize; // in bytes
	bool cs_unified; // false is split


	// main cache memory variables
	cacheData ** cs_cacheMemory; // array representing cache components
	int cs_numberOfBlocks; // total number of blocks
	int cs_numberOfSets; // total number of sets

	node *** cs_plruBinaryTrees; // required for pseudo lru replacement policy

	// address variables
	unsigned int cs_addressOffsetNumBits; // Number of Offset bits
	unsigned int cs_addressIndexNumBits; // Number of Index bits
	unsigned int cs_addressTagNumBits; // Number of Tag bits

	// used for extracting from address
	unsigned int cs_tagMask;
	unsigned int cs_setIndexMask;
	unsigned int cs_setOffsetMask;



	bool setAssiciativity(int argc, int indx, char **argv);
	bool setBlockSize(int argc, int indx, char **argv);
	bool setUnified(int argc, int indx, char **argv);
	bool setTraceFilename(int argc, int indx, char **argv);
	bool setCacheSize(char *argv);


	int getSetIndex(int address);
	int getSetTag(int address);
	int getSetOffset(int address);



	void initializeAllDefaultVariables();

	unsigned int getNumberFromHexString(std::string num);

};

void CacheSimulator::decodeAndSetParameters(int argc, char **argv)
{
	if (argc > 1)
	{
		int i = 1;
		for (i = 1; i < argc; i++)
		{
			if (strncmp(ASSOCIATIVITY_FLAG, argv[i], strlen(ASSOCIATIVITY_FLAG)) == 0)
			{
				setAssiciativity(argc, i, argv);
				i++;
			}
			else if (strncmp(BLOCKSIZE_FLAG, argv[i], strlen(BLOCKSIZE_FLAG)) == 0)
			{
				setBlockSize(argc, i, argv);
				i++;
			}
			else if (strncmp(UNIFIED_FLAG, argv[i], strlen(UNIFIED_FLAG)) == 0)
			{
				setUnified(argc, i, argv);
				i++;
			}
			else if (strncmp(TRACE_FLAG, argv[i], strlen(TRACE_FLAG)) == 0)
			{
				setTraceFilename(argc, i, argv);
				i++;
			}
			else if (isdigit(argv[i][0]))
			{
				setCacheSize(argv[i]);
			}

		}
	}



}
bool CacheSimulator::setAssiciativity(int argc, int indx, char **argv)
{
	indx = indx + 1;
	if (indx < argc && isdigit(argv[indx][0]))
	{
		cs_associativity = atoi(argv[indx]);

		return 1;
	}
	else
	{
		return 0;
	}
}
bool CacheSimulator::setBlockSize(int argc, int indx, char **argv)
{
	indx = indx + 1;
	if (indx < argc && isdigit(argv[indx][0]))
	{
		cs_blockSize = atoi(argv[indx]);

		return 1;
	}
	else
	{
		return 0;
	}
}
bool CacheSimulator::setUnified(int argc, int indx, char **argv)
{
	indx = indx + 1;
	if (indx < argc)
	{
		if (argv[indx][0] == 'U')
		{
			cs_unified = true;//unified
		}
		else if (argv[indx][0] == 'S')
		{
			cs_unified = false; //split
		}

		return 1;
	}
	else
	{
		return 0;
	}
}
bool CacheSimulator::setTraceFilename(int argc, int indx, char **argv)
{
	indx = indx + 1;
	if (indx < argc)
	{

		cs_traceFilename = std::string(argv[indx]);

		return 1;
	}
	else
	{
		return 0;
	}

}
bool CacheSimulator::setCacheSize(char *argv)
{
	cs_cacheSize = atoi(argv); cs_cacheSize = cs_cacheSize * 1024;

	return 1;
}
void CacheSimulator::printCacheState()
{
	std::cout << "-----------------------_CACHE STATUS_-----------------------\n";
	std::cout << "Cache Size (bytes): " << cs_cacheSize << " (KB):" << cs_cacheSize / 1024 << std::endl;
	if (!cs_unified){
		std::cout << "Associativity: " << cs_associativity / 2 << std::endl;
	}
	else
	{
		std::cout << "Associativity: " << cs_associativity << std::endl;
	}
	std::cout << "Block Size (bytes): " << cs_blockSize << std::endl;
	std::cout << "Unified: " << cs_unified << std::endl;
	if (!cs_traceFilename.empty()){ std::cout << "File Name: " << cs_traceFilename << std::endl; }
	std::cout << "OffsetNumBits:" << cs_addressOffsetNumBits << std::endl;
	std::cout << "IndexNumBits:" << cs_addressIndexNumBits << std::endl;
	std::cout << "TagNumBits:" << cs_addressTagNumBits << std::endl;
	std::cout << "Offset Mask:" << std::bitset<32>(cs_setOffsetMask) << std::endl;
	std::cout << "Index Mask:" << std::bitset<32>(cs_setIndexMask /*>> cs_addressOffsetNumBits*/) << std::endl;
	std::cout << "Tag Mask:" << std::bitset<32>(cs_tagMask /*>> (cs_addressOffsetNumBits + cs_addressIndexNumBits)*/) << std::endl;
	std::cout << "------------------------------------------------------------\n";
}
bool CacheSimulator::createCacheAndSetUpAddressVariables()
{
	if (cs_blockSize <= 0 || cs_associativity <= 0)
	{
		std::cout << "Block size of associativity are non positive!" << std::endl;
		return false;
	}

	//setting address variables
	cs_numberOfBlocks = cs_cacheSize / cs_blockSize;

	if (!cs_unified){
		cs_associativity = cs_associativity * 2; // divide the cache into two blocks, one for inst, and other for data
	}

	cs_numberOfSets = cs_numberOfBlocks / cs_associativity;

	cs_addressOffsetNumBits = (int)log2((double)cs_blockSize);
	cs_addressIndexNumBits = (int)log2((double)cs_numberOfSets);
	cs_addressTagNumBits = ADDRESS_LENGHT - cs_addressOffsetNumBits - cs_addressIndexNumBits;

	// Creating the cache
	cs_cacheMemory = (cacheData **)malloc(sizeof(cacheData*)*(cs_numberOfSets + 1));

	int i; int j; // used for loops

	for (i = 0; i < cs_numberOfSets; i++)
	{
		cs_cacheMemory[i] = (cacheData*)malloc(sizeof(cacheData)*(cs_associativity + 1));
	}
	

	int halfAssociativity = cs_associativity / 2; // used when cache is not unified

	for (i = 0; i < cs_numberOfSets; i++)
	{
		for (j = 0; j < cs_associativity; j++)
		{
			// setting the validation bit
			cs_cacheMemory[i][j].validationBit = 0;

			// setting up the lruCounter correctly depending on unified/split
			if (!cs_unified){
				cs_cacheMemory[i][j].lruCounterVal = j % (halfAssociativity);
			}
			else
			{
				cs_cacheMemory[i][j].lruCounterVal = j;
			}

		}
	}


	// allocate memory for each of the bynary tree used for the pseudo LRU ------------------
	int numBitsForPlruTree = (int)log2(cs_associativity);
	if (!cs_unified){
		// in this case, there are two, for data and instruction
		cs_plruBinaryTrees = (node ***)malloc(sizeof(node**)*(2));
		cs_plruBinaryTrees[0] = (node **)malloc(sizeof(node*)*(cs_numberOfSets + 1));
		cs_plruBinaryTrees[1] = (node **)malloc(sizeof(node*)*(cs_numberOfSets + 1));

		// initialize tree of height numBitsForPlruTree for each set in the cache
		for (i = 0; i < cs_numberOfSets; i++)
		{
			cs_plruBinaryTrees[0][i] = createBynaryTree(numBitsForPlruTree, 0); // 0 is for data
			cs_plruBinaryTrees[1][i] = createBynaryTree(numBitsForPlruTree, 0); // 1 is for instruction
		}
	}
	else
	{
		cs_plruBinaryTrees = (node ***)malloc(sizeof(node**));
		cs_plruBinaryTrees[0] = (node **)malloc(sizeof(node*)*(cs_numberOfSets + 1));

		for (i = 0; i < cs_numberOfSets; i++)
		{
			cs_plruBinaryTrees[0][i] = createBynaryTree(numBitsForPlruTree, 0);
		}
	}
	//------------------------------------------------------------------------


	// set the masks for extracting parts of address
	for (i = 0; i < cs_addressOffsetNumBits; i++){ cs_setOffsetMask += 1 << i; }
	for (i = 0; i < cs_addressIndexNumBits; i++){ cs_setIndexMask += 1 << (i + cs_addressOffsetNumBits); }
	for (i = 0; i < cs_addressTagNumBits; i++){ cs_tagMask += 1 << (i + cs_addressOffsetNumBits + cs_addressIndexNumBits); }


	return true;

}
void CacheSimulator::initializeAllDefaultVariables()
{
	// Fields used for user input
	cs_cacheSize = DEFAULT_CACHESIZE_KB; // total size of cache
	cs_associativity = DEFAULT_ASSOCIATIVITY; //number of blocks in set 
	cs_blockSize = DEFAULT_BLOCKSIZE_B; // in bytes
	cs_unified = true; // false is split

	// main cache memory variables
	cs_cacheMemory = NULL; // array representing cache components
	cs_numberOfBlocks = 0; // total number of blocks
	cs_numberOfSets = 0; // total number of sets

	// address variables
	cs_addressOffsetNumBits = 0; // Number of Offset bits
	cs_addressIndexNumBits = 0; // Number of Index bits
	cs_addressTagNumBits = 0; // Number of Tag bits

	// used for extracting from address
	cs_tagMask = 0;
	cs_setIndexMask = 0;
	cs_setOffsetMask = 0;

	cs_traceFilename = "trace.txt";

	// variables for counting miss rate
	cs_numberOfMisses = 0;
	cs_totalInstructions = 0;
}

void CacheSimulator::updateCachePseudoLRU(std::string line)
{
	cs_totalInstructions++;

	size_t pos = line.find(' ');
	std::string inst = line.substr(0, pos);
	std::string address = line.substr(pos + 1);

	// extract the instruction type

	int INSTRUCTION_TYPE = atoi(inst.c_str());
	unsigned int ADDRESS_32BIT = getNumberFromHexString(address);

	//std::cout << "inst: '" << inst << "' :" << INSTRUCTION_TYPE << std::endl;
	//std::cout << "address: '" << address << "' :" << ADDRESS_32BIT << std::endl;

	int offset = (ADDRESS_32BIT & cs_setOffsetMask);
	int setIndex = (ADDRESS_32BIT & cs_setIndexMask) >> cs_addressOffsetNumBits;
	int tag = (ADDRESS_32BIT & cs_tagMask) >> (cs_addressOffsetNumBits + cs_addressIndexNumBits);


	int startIndex = 0;
	int endIndex = cs_associativity;

	int plruIndex = 0;

	if (!cs_unified){

		if (INSTRUCTION_TYPE == INSTRUCTION_LOAD) // look on the other half for instructions
		{
			startIndex = cs_associativity / 2;// start at half
			endIndex = cs_associativity; // end at the end

			plruIndex = 1; // select the plru of the instruction space in the cache
		}
		else  // look on first other half for instructions
		{
			startIndex = 0; // start from zero
			endIndex = cs_associativity / 2; // end at middle

			plruIndex = 0; //select the plru of the data space in the cache
		}
	}


	cacheData* currentSet = cs_cacheMemory[setIndex];

	bool foundInCache = false;

	int indexOfHit = -1;

	// first look to see if the block is in the cache
	int i;
	for (i = startIndex; i < endIndex; i++)
	{
		if (currentSet[i].validationBit == 1) // data here is valid
		{
			if (currentSet[i].tag == tag) //this is right data with matching tag
			{
				// its a hit
				foundInCache = true;

				// must record where exactly the the block was found in order to properly update the LRU counters
				indexOfHit = i;

				break;
			}
		}
	}


	/* This is where the replacement policy is implemented! */
	if (!foundInCache)
	{
		cs_numberOfMisses++;

		// for plru, we must replace the block defined by the binary tree

		// get the index of the place to replace
		// this function gets the index of block to replace aswell it flips the bits
		int indexToReplace = getTreeValueAndFlip(cs_plruBinaryTrees[plruIndex][setIndex]);

		// replace that block with the right tag
		currentSet[indexToReplace].tag = tag;
		currentSet[indexToReplace].validationBit = 1;
	}
	else // update pLRU counters after a hit. Note here we know there was a HIT and where it occurred 
	{
		// simply flip the bits of binary tree that lead to hit index
		setTreeValueAndFlip(cs_plruBinaryTrees[plruIndex][setIndex], indexOfHit);
	}
}

void CacheSimulator::updateCacheLRU(std::string line)
{
	cs_totalInstructions++;

	size_t pos = line.find(' ');
	std::string inst = line.substr(0, pos);
	std::string address = line.substr(pos + 1);

	// extract the instruction type

	int INSTRUCTION_TYPE = atoi(inst.c_str());
	unsigned int ADDRESS_32BIT = getNumberFromHexString(address);

	//std::cout << "inst: '" << inst << "' :" << INSTRUCTION_TYPE << std::endl;
	//std::cout << "address: '" << address << "' :" << ADDRESS_32BIT << std::endl;

	int offset = (ADDRESS_32BIT & cs_setOffsetMask);
	int setIndex = (ADDRESS_32BIT & cs_setIndexMask) >> cs_addressOffsetNumBits;
	int tag = (ADDRESS_32BIT & cs_tagMask) >> (cs_addressOffsetNumBits + cs_addressIndexNumBits);


	int startIndex = 0;
	int endIndex = cs_associativity;

	if (!cs_unified){

		if (INSTRUCTION_TYPE == INSTRUCTION_LOAD) // look on the other half for instructions
		{
			startIndex = cs_associativity / 2;// start at half
			endIndex = cs_associativity; // end at the end
		}
		else  // look on first other half for instructions
		{
			startIndex = 0; // start from zero
			endIndex = cs_associativity / 2; // end at middle
		}
	}


	cacheData* currentSet = cs_cacheMemory[setIndex];

	bool foundInCache = false;

	int indexOfHit = -1;
	// used for loop
	int i;
	for (i = startIndex; i < endIndex; i++)
	{
		if (currentSet[i].validationBit == 1) // data here is valid
		{
			if (currentSet[i].tag == tag) //this is right data with matching tag
			{
				// its a hit
				foundInCache = true;

				// must record where exactly the the block was found in order to properly update the LRU counters
				indexOfHit = i;
				break;
			}
		}
	}


	/* This is where the replacement policy is implemented! */
	if (!foundInCache)
	{
		cs_numberOfMisses++;

		for (i = startIndex; i < endIndex; i++)
		{
			if (currentSet[i].lruCounterVal == 0) // this is the least recently used block
			{
				currentSet[i].validationBit = 1;
				currentSet[i].tag = tag;
				break;
			}
		}

		// Now we should update the lru counter for all the blocks in that set
		for (i = startIndex; i < endIndex; i++)
		{
			currentSet[i].lruCounterVal = currentSet[i].lruCounterVal - 1;
			// least recently used becomes the new most recently used after changing the 
			// must account for both split/unified cache
			if (currentSet[i].lruCounterVal < 0) { currentSet[i].lruCounterVal = endIndex - startIndex - 1; }
		}

	}
	else // update LRU counters after a hit. Note here we know there was a HIT and where it occurred 
	{
		// MUST! update the LRU counters such that the counters have DIFFERENT values
		for (i = startIndex; i < endIndex; i++)
		{
			if (currentSet[i].lruCounterVal > currentSet[indexOfHit].lruCounterVal)
			{// all counters that are greater than the counter of the block that contained the requested data must be decreased
				currentSet[i].lruCounterVal = currentSet[i].lruCounterVal - 1; // decrease this counter 
			}
			// all others should stay the same
		}
		// Now assign the most recent value to the block that was HIT
		currentSet[indexOfHit].lruCounterVal = endIndex - startIndex - 1;
	}
	//getTreeValueAndFlip(node *rt);


}

unsigned int CacheSimulator::getNumberFromHexString(std::string num)
{
	unsigned int number;
	std::stringstream ss;
	ss << std::hex << num;
	ss >> number;

	return number;
}
/* -----------------------------------------*/

int main(int argc, char **argv) 
{

	std::cout << "************************************************************************************" << std::endl;

	CacheSimulator cs = CacheSimulator();

	cs.decodeAndSetParameters(argc, argv);
	if (!cs.createCacheAndSetUpAddressVariables()){
		std::cout << "Passed invalid parameters!" << std::endl;
		//system("pause"); 
		return -1;
	}

	cs.printCacheState();

	std::ifstream fileInput;
	std::string line;
	fileInput.open(cs.cs_traceFilename.c_str());

	if (!fileInput.is_open()){
		std::cout << "File:" << cs.cs_traceFilename << " not found! " << std::endl;
		//system("pause"); 
		return -1;
	}
	
	while (std::getline(fileInput, line))
	{
		// cs.updateCacheLRU(line);  // pseudo LRU or LRU cache replacement policy
		cs.updateCachePseudoLRU(line);
	}
	std::cout << "Finished reading file: " << cs.cs_traceFilename << std::endl; //cs_totalInstructions
	std::cout << "Total instructions: " << cs.cs_totalInstructions << std::endl;
	std::cout << "Total misses: " << cs.cs_numberOfMisses << std::endl;
	std::cout << "Miss rate :" << 100 * (float)cs.cs_numberOfMisses / (float)cs.cs_totalInstructions << "%" << std::endl;
	std::cout << "************************************************************************************" << std::endl;
	system("pause");
	return 0;
}
