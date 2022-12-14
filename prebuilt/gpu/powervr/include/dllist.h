/*************************************************************************/ /*!
@File
@Title          Double linked list header
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Double linked list interface
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef DLLIST_H
#define DLLIST_H

#include "img_types.h"
#include "img_defs.h"

/*!
	Pointer to a linked list node
*/
typedef struct DLLIST_NODE_	*PDLLIST_NODE;


/*!
	Node in a linked list
*/
/*
 * Note: the following structure's size is architecture-dependent and clients
 * may need to create a mirror of the structure definition if it needs to be
 * used in a structure shared between host and device.
 * Consider such clients if any changes are made to this structure.
 */
typedef struct DLLIST_NODE_
{
	struct DLLIST_NODE_	*psPrevNode;
	struct DLLIST_NODE_	*psNextNode;
} DLLIST_NODE;


/*!
	Static initialiser
*/
#define DECLARE_DLLIST(n) \
DLLIST_NODE (n) = {&(n), &(n)}


/*************************************************************************/ /*!
@Function       dllist_init

@Description    Initialize a new double linked list

@Input          psListHead             List head Node

*/
/*****************************************************************************/
static INLINE
void dllist_init(PDLLIST_NODE psListHead)
{
	psListHead->psPrevNode = psListHead;
	psListHead->psNextNode = psListHead;
}

/*************************************************************************/ /*!
@Function       dllist_is_empty

@Description    Returns whether the list is empty

@Input          psListHead             List head Node

*/
/*****************************************************************************/
static INLINE
bool dllist_is_empty(PDLLIST_NODE psListHead)
{
	return ((psListHead->psPrevNode == psListHead)
			&& (psListHead->psNextNode == psListHead));
}

/*************************************************************************/ /*!
@Function       dllist_add_to_head

@Description    Add psNewNode to head of list psListHead

@Input          psListHead             Head Node
@Input          psNewNode              New Node

*/
/*****************************************************************************/
static INLINE
void dllist_add_to_head(PDLLIST_NODE psListHead, PDLLIST_NODE psNewNode)
{
	PDLLIST_NODE psTmp;

	psTmp = psListHead->psNextNode;

	psListHead->psNextNode = psNewNode;
	psNewNode->psNextNode = psTmp;

	psTmp->psPrevNode = psNewNode;
	psNewNode->psPrevNode = psListHead;
}


/*************************************************************************/ /*!
@Function       dllist_add_to_tail

@Description    Add psNewNode to tail of list psListHead

@Input          psListHead             Head Node
@Input          psNewNode              New Node

*/
/*****************************************************************************/
static INLINE
void dllist_add_to_tail(PDLLIST_NODE psListHead, PDLLIST_NODE psNewNode)
{
	PDLLIST_NODE psTmp;

	psTmp = psListHead->psPrevNode;

	psListHead->psPrevNode = psNewNode;
	psNewNode->psPrevNode = psTmp;

	psTmp->psNextNode = psNewNode;
	psNewNode->psNextNode = psListHead;
}

/*************************************************************************/ /*!
@Function       dllist_node_is_in_list

@Description    Returns true if psNode is in a list

@Input          psNode                 List node

*/
/*****************************************************************************/
static INLINE
bool dllist_node_is_in_list(PDLLIST_NODE psNode)
{
	return (psNode->psNextNode != NULL);
}

/*************************************************************************/ /*!
@Function       dllist_get_next_node

@Description    Returns the list node after psListHead or NULL psListHead is
                the only element in the list.

@Input          psListHead             List node to start the operation

*/
/*****************************************************************************/
static INLINE
PDLLIST_NODE dllist_get_next_node(PDLLIST_NODE psListHead)
{
	if (psListHead->psNextNode == psListHead)
	{
		return NULL;
	}
	else
	{
		return psListHead->psNextNode;
	}
}

/*************************************************************************/ /*!
@Function       dllist_get_prev_node

@Description    Returns the list node preceding psListHead or NULL if
                psListHead is the only element in the list.

@Input          psListHead             List node to start the operation

*/
/*****************************************************************************/
static INLINE
PDLLIST_NODE dllist_get_prev_node(PDLLIST_NODE psListHead)
{
	if (psListHead->psPrevNode == psListHead)
	{
		return NULL;
	}
	else
	{
		return psListHead->psPrevNode;
	}
}

/*************************************************************************/ /*!
@Function       dllist_remove_node

@Description    Removes psListNode from the list where it currently belongs

@Input          psListNode             List node to be removed

*/
/*****************************************************************************/
static INLINE
void dllist_remove_node(PDLLIST_NODE psListNode)
{
	psListNode->psNextNode->psPrevNode = psListNode->psPrevNode;
	psListNode->psPrevNode->psNextNode = psListNode->psNextNode;

	/* Clear the node to show it's not in a list */
	psListNode->psPrevNode = NULL;
	psListNode->psNextNode = NULL;
}

/*************************************************************************/ /*!
@Function       dllist_replace_head

@Description    Moves the list from psOldHead to psNewHead

@Input          psOldHead              List node to be replaced. Will become a
                                       head node of an empty list.
@Input          psNewHead              List node to be inserted. Must be an
                                       empty list head.

*/
/*****************************************************************************/
static INLINE
void dllist_replace_head(PDLLIST_NODE psOldHead, PDLLIST_NODE psNewHead)
{
	if (dllist_is_empty(psOldHead))
	{
		psNewHead->psNextNode = psNewHead;
		psNewHead->psPrevNode = psNewHead;
	}
	else
	{
		/* Change the neighbouring nodes */
		psOldHead->psNextNode->psPrevNode = psNewHead;
		psOldHead->psPrevNode->psNextNode = psNewHead;

		/* Copy the old data to the new node */
		psNewHead->psNextNode = psOldHead->psNextNode;
		psNewHead->psPrevNode = psOldHead->psPrevNode;

		/* Remove links to the previous list */
		psOldHead->psNextNode = psOldHead;
		psOldHead->psPrevNode = psOldHead;
	}
}

/**************************************************************************/ /*!
@Function       dllist_insert_list_at_head

@Description    Inserts psInHead list into the head of the psOutHead list.
                After this operation psOutHead will contain psInHead at the
                head of the list and the remaining elements that were
                already in psOutHead will be places after the psInList (so
                at a tail of the original list).

@Input          psOutHead       List node psInHead will be inserted to.
@Input          psInHead        List node to be inserted to psOutHead.
                                After this operation this becomes an empty list.
*/ /***************************************************************************/
static INLINE
void dllist_insert_list_at_head(PDLLIST_NODE psOutHead, PDLLIST_NODE psInHead)
{
	PDLLIST_NODE psInHeadNextNode = psInHead->psNextNode;
	PDLLIST_NODE psOutHeadNextNode = psOutHead->psNextNode;

	if (!dllist_is_empty(psInHead))
	{
		psOutHead->psNextNode = psInHeadNextNode;
		psInHeadNextNode->psPrevNode = psOutHead;

		psInHead->psPrevNode->psNextNode = psOutHeadNextNode;
		psOutHeadNextNode->psPrevNode = psInHead->psPrevNode;

		dllist_init(psInHead);
	}
 }

/*************************************************************************/ /*!
@Function       dllist_foreach_node

@Description    Walk through all the nodes on the list.
                Safe against removal of (node).

@Input          list_head              List node to start the operation
@Input          node                   Current list node
@Input          next                   Node after the current one

*/
/*****************************************************************************/
#define dllist_foreach_node(list_head, node, next)						\
	for ((node) = (list_head)->psNextNode, (next) = (node)->psNextNode;		\
		 (node) != (list_head);											\
		 (node) = (next), (next) = (node)->psNextNode)

#define dllist_foreach_node_backwards(list_head, node, prev)			\
	for ((node) = (list_head)->psPrevNode, (prev) = (node)->psPrevNode;		\
		 (node) != (list_head);											\
		 (node) = (prev), (prev) = (node)->psPrevNode)


/*************************************************************************/ /*!
@Function       dllist_foreach

@Description    Simplification of dllist_foreach_node.
                Walk through all the nodes on the list.
                Safe against removal of currently-iterated node.

                Adds utility-macro dllist_cur() to typecast the current node.

@Input          list_head              List node to start the operation

*/
/*****************************************************************************/
#define dllist_foreach(list_head)	\
	for (DLLIST_NODE *_DllNode = (list_head).psNextNode, *_DllNext = _DllNode->psNextNode;		\
		 _DllNode != &(list_head);																\
		 _DllNode = _DllNext, _DllNext = _DllNode->psNextNode)

#define dllist_foreach_backwards(list_head)	\
	for (DLLIST_NODE *_DllNode = (list_head).psPrevNode, *_DllPrev = _DllNode->psPrevNode;		\
		 _DllNode != &(list_head);																\
		 _DllNode = _DllPrev, _DllPrev = _DllNode->psPrevNode)

#define dllist_cur(type, member)	IMG_CONTAINER_OF(_DllNode, type, member)

#endif /* DLLIST_H */
