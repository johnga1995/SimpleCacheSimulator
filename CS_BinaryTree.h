/*
* This header contains functions to create a binary tree, setting values to binary tree, and getting values of binary tree
* Author: Juan Gabriel Loja
*/

#ifndef CS_BYNARY_TREE_H
#define CS_BYNARY_TREE_H

#include <iostream>

typedef struct BinaryNode{

	int bit;
	int shift;
	struct BinaryNode* left;
	struct BinaryNode* right;

}node;

void populateNodes(node *nd, int height, int value)
{
	if (nd != NULL){
		if (height > 1)
		{

			nd->shift = height - 1;
			nd->bit = value;

			nd->left = (node*)malloc(sizeof(node));
			nd->right = (node*)malloc(sizeof(node));

			populateNodes(nd->left, height - 1, value);
			populateNodes(nd->right, height - 1, value);

		}
		else
		{
			nd->shift = height - 1;
			nd->bit = value;
			nd->left = NULL;
			nd->right = NULL;
		}
	}
}

node* createBynaryTree(int height, int value)
{
	node* rt = (node*)malloc(sizeof(node));
	populateNodes(rt, height, value);

	return rt;
}


void printTree(node *rt)
{
	if (rt != NULL){

		printTree(rt->left);
		std::cout << "  rt_s:" << rt->shift;
		printTree(rt->right);
	}
}

void printTreeBits(node *rt)
{
	if (rt != NULL){

		printTreeBits(rt->left);
		std::cout << "  rt_b:" << rt->bit;
		printTreeBits(rt->right);
	}
}

int getTreeValue(node *rt)
{
	if (rt != NULL){

		int value = rt->bit << rt->shift;

		if (rt->bit == 0){
			value += getTreeValue(rt->left);
		}
		else if (rt->bit == 1)
		{
			value += getTreeValue(rt->right);
		}

		return value;
	}
	else
	{
		return 0;
	}

}

void setTreeValue(node *rt, int val)
{
	if (rt != NULL){

		rt->bit = (val >> rt->shift) & 1;;

		if (rt->bit == 0){
			setTreeValue(rt->left, val);
		}
		else if (rt->bit == 1)
		{
			setTreeValue(rt->right, val);
		}

	}
}


// used when replacing
int getTreeValueAndFlip(node *rt){

	if (rt != NULL){

		int value = rt->bit << rt->shift;


		if (rt->bit == 0){
			value += getTreeValueAndFlip(rt->left);
		}
		else if (rt->bit == 1)
		{
			value += getTreeValueAndFlip(rt->right);
		}

		// flip the bit to make it most recently used
		rt->bit = 1 - rt->bit;

		return value;
	}
	else
	{
		return 0;
	}
}

// used when updating the plru tree
void setTreeValueAndFlip(node *rt, int val){

	if (rt != NULL){

		rt->bit = (val >> rt->shift) & 1;;

		if (rt->bit == 0){
			setTreeValueAndFlip(rt->left, val);
		}
		else if (rt->bit == 1)
		{
			setTreeValueAndFlip(rt->right, val);
		}

		// flip the bit to make it most recently used
		rt->bit = 1 - rt->bit;

	}
}

#endif // CS_BYNARY_TREE_H