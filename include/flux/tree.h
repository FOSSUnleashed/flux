#pragma once

#include <dill/util_rbtree.h>

typedef struct dill_rbtree TreeRoot;
typedef struct dill_rbtree_item Tree;

#define treeInit(root) dill_rbtree_init((root))

#define treeSafeInit(root) do {\
	if ((root)->nil.up != &(root)->nil\
		 || (root)->nil.left != &(root)->nil\
		 || (root)->nil.right != &(root)->nil\
	) {\
		treeInit(root);\
	}\
} while (0)
