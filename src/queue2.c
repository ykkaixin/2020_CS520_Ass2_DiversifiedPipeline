//
// Created by kai on 4/10/20.
//
#include "queue2.h"

typedef struct QueueNode
{
    reg2*data;
    struct QueueNode *next;
} Node;


//------------------------------------------------------------------------------
// Returns by reference the data at the front of the queue, and removes the
// front node from the queue.
// Returns 0 on success, -1 on failure.
//------------------------------------------------------------------------------
int dequeue1(WBQueue *q, reg2 *data)
{
    if (q->size > 0)
    {
        memcpy(data, q->head->data, q->memSize);

        Node *temp = q->head->next;
        free(q->head->data);
        free(q->head);

        if (q->size > 1)
            q->head = temp;
        else
            q->head = q->tail = NULL;

        q->size--;

        return 0;
    }

    return -1;
}


//------------------------------------------------------------------------------
// Adds the given data to the back of the queue.
// Returns 0 on success, -1 on failure.
//------------------------------------------------------------------------------
int enqueue1(WBQueue *q, reg2 *data)
{
    if (q->memSize > 0)
    {
        Node *node = (Node *) malloc(sizeof(Node));
        if (node == NULL)
            return -1;

        node->data = malloc(q->memSize);
        if (node->data == NULL)
        {
            free(node);
            return -1;
        }

        node->next = NULL;
        memcpy(node->data, data, q->memSize);

        if (q->size == 0)
            q->head = q->tail = node;
        else
            q->tail = q->tail->next = node;

        q->size++;

        return 0;
    }

    return -1;
}


//------------------------------------------------------------------------------
// Frees all the nodes and data. Head and tail point to NULL, size is reset to
// zero, but memSize is kept intact.
//------------------------------------------------------------------------------
void queueClear1(WBQueue *q)
{
    while(q->size > 0)
    {
        Node *temp = q->head;
        q->head = q->head->next;
        free(temp->data);
        free(temp);
        q->size--;
    }

    q->head = q->tail = NULL;
}


//------------------------------------------------------------------------------
// Returns 1 if the queue is empty, 0 otherwise.
//------------------------------------------------------------------------------
int queueEmpty1(const WBQueue *q)
{
    return (q->size == 0);
}


//------------------------------------------------------------------------------
// Initializes the queue.
// Returns 0 on success, -1 on failure.
//------------------------------------------------------------------------------
int queueInit1(WBQueue *q, const size_t memSize)
{
    if (memSize > 0)
    {
        q->memSize = memSize;
        q->size = 0;
        q->head = q->tail = NULL;
        return 0;
    }

    return -1;
}


//------------------------------------------------------------------------------
// Returns by reference the data at the front of the queue.
// Returns 0 on success, -1 on failure.
//------------------------------------------------------------------------------
int queuePeek1(const WBQueue *q, reg2  *data)
{
    if (q->size > 0)
    {
        memcpy(data, q->head->data, q->memSize);
        return 0;
    }

    return -1;
}


//------------------------------------------------------------------------------
// Returns the size of the queue.
//------------------------------------------------------------------------------
size_t queueSize1(const WBQueue *q)
{
    return q->size;
}
