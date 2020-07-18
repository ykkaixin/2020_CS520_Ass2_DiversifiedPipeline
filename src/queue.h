#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <string.h>
#include "../instruction.h"
#include "pipeLine.h"
typedef struct Queue
{
  size_t memSize;
  size_t size;
  struct QueueNode *head;
  struct QueueNode *tail;
} Queue;

int dequeue(Queue *, reg *);
int enqueue(Queue *, reg *);
void queueClear(Queue *);
int queueEmpty(const Queue *);
int queueInit(Queue *, const size_t);
int queuePeek(const Queue *, reg *);
size_t queueSize(const Queue *);

#endif


