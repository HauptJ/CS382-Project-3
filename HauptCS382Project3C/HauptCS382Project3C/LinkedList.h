/////////////////////////////////////////////////////////////////
// Class definition file: LinkedList.h                         //
//                                                             //
// This file defines the LinkedList class template, a regular  //
// doubly-linked circular list of nodes containing values of   //
// type E (the template's parameterized data-type).  Its       //
// member functions include constructors, a destructor, and    //
// standard isEmpty, getHeadValue, and getSize functions.      //
// Insertion and removal always occur at the head of the list. //
/////////////////////////////////////////////////////////////////

#ifndef LINKED_LIST_H

#include <assert.h>

////////////////////////////////////////////////////////
// DECLARATION SECTION FOR LINKED LIST CLASS TEMPLATE //
////////////////////////////////////////////////////////

template <class E> class LinkedList
{
	public:
		// Class constructors and destructor
		LinkedList();
		LinkedList(const LinkedList<E> &list);
		~LinkedList();

		// Member functions
		bool isEmpty();
		void insert(E item);
		bool removeHead();
		E getHeadValue();
		E getHeadNextValue();
		int getSize();
		LinkedList<E>& operator ++ ();
	
	protected:
		// Data members

		struct node;
		typedef node *nodePtr;	
		struct node
		{
			E data;
			nodePtr next;
			nodePtr previous;
		};

		nodePtr head;
		int size;

		// Member function
		void* getNode(E item);
};

///////////////////////////////////////////////
// IMPLEMENTATION SECTION FOR CLASS TEMPLATE //
///////////////////////////////////////////////

//////////////////////////////////////////////
// Default constructor: Sets up empty list. //
//////////////////////////////////////////////
template <class E>
LinkedList<E>::LinkedList()
{
	head = NULL;
	size = 0;
}

////////////////////////////////////////////////
// Copy constructor: Makes deep copy of list. //
////////////////////////////////////////////////
template <class E>
LinkedList<E>::LinkedList(const LinkedList<E> &list)
{
	nodePtr copyPreviousPtr, copyCurrentPtr, origCurrentPtr;
	size = list.size;
	if (list.head == NULL)
		head = NULL;
	else
	{
		head = (nodePtr)getNode(list.head->data);
		copyPreviousPtr = head;
		origCurrentPtr = list.head->next;
		while (origCurrentPtr != list.head)
		{
			copyCurrentPtr = (nodePtr)getNode(origCurrentPtr->data);
			copyPreviousPtr->next = copyCurrentPtr;
			copyCurrentPtr->previous = copyPreviousPtr;
			copyPreviousPtr = copyCurrentPtr;
			origCurrentPtr = origCurrentPtr->next;
		}
		copyPreviousPtr->next = head;
		head->previous = copyPreviousPtr;
	}
}

/////////////////////////////////////////////////////////////
// Destructor: Converts entire list back into free memory. //
/////////////////////////////////////////////////////////////
template <class E>
LinkedList<E>::~LinkedList()
{
	nodePtr ptr;
	while (head != NULL)
	{
		ptr = head;
		if (head == head->next)
			head = NULL;
		else
		{
			head = head->next;
			head->previous = ptr->previous;
			head->previous->next = head;
		}
		delete ptr;
	}
}

//////////////////////////////////////////////////////
// Function to determine whether the list is empty. //
//////////////////////////////////////////////////////
template <class E>
bool LinkedList<E>::isEmpty()
{
	return (size == 0);
}

//////////////////////////////////////////////////////////////
// Function to insert value "item" at the head of the list. //
//////////////////////////////////////////////////////////////
template <class E>
void LinkedList<E>::insert(E item)
{
	nodePtr insertPtr;

	size++;
	insertPtr = (nodePtr)getNode(item);
	if (head == NULL)
	{
		head = insertPtr;
		head->next = head;
		head->previous = head;
	}
	else
	{
		insertPtr->next = head;
		insertPtr->previous = head->previous;
		head->previous->next = insertPtr;
		head->previous = insertPtr;
		head = insertPtr;
	}

	return;
}

//////////////////////////////////////////////////
// Function to remove the head element from     //
// the list.  A boolean is returned to indicate //
// whether such an element existed.             //
//////////////////////////////////////////////////
template <class E>
bool LinkedList<E>::removeHead()
{
	nodePtr currentPtr;

	if ( ( size == 0) || (head == NULL) )
		return false;
	else
	{
		currentPtr = head;
		if (head->next == head)
		{
			size = 0;
			head = NULL;
		}
		else
		{
			size--;
			head = currentPtr->next;
			head->previous = currentPtr->previous;
			currentPtr->previous->next = head;
		}
		delete currentPtr;
		return true;
	}
}

////////////////////////////////////////////////////////////////////
// Function getHeadValue returns a copy of the head node's value. //
////////////////////////////////////////////////////////////////////
template <class E>
E LinkedList<E>::getHeadValue()
{
	assert(head != NULL);
	return head->data;
}

/////////////////////////////////////////////////
// Function getHeadValue returns a copy of the //
// value of the node after the head node.      //
/////////////////////////////////////////////////
template <class E>
E LinkedList<E>::getHeadNextValue()
{
	assert(head != NULL);
	return head->next->data;
}

/////////////////////////////////////////
// Function size returns the current   //
// number of nodes in the linked list. //
/////////////////////////////////////////
template <class E>
int LinkedList<E>::getSize()
{
	return size;
}

////////////////////////////////////////////////////////////
// The increment operator moves the head pointer one item //
// further down the linked list (if that's possible).     //
////////////////////////////////////////////////////////////
template <class E>
LinkedList<E>& LinkedList<E>::operator ++ ()
{
	if (head != NULL)
		head = head->next;
	return *this;
}

//////////////////////////////////////////////////////////////////
// Function to generate a new node with the data value provided //
// in parameter item, and returning a pointer to this new node. //
//////////////////////////////////////////////////////////////////
template <class E>
void* LinkedList<E>::getNode(E item)
{
	nodePtr temp = new node;

	assert(temp != NULL);
	temp->data = item;
	temp->next = NULL;
	temp->previous = NULL;
	return temp;
}

#define LINKED_LIST_H
#endif

