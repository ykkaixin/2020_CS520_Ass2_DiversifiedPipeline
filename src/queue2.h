//
// Created by kai on 4/10/20.
//
#include <stdlib.h>
#include <string.h>
#include "pipeLine.h"

#ifndef PIPELINE_QUEUE2_H
#define PIPELINE_QUEUE2_H

typedef struct
{
    size_t memSize;
    size_t size;
    struct QueueNode *head;
    struct QueueNode *tail;
} WBQueue;

int dequeue1(WBQueue *, reg2 *);
int enqueue1(WBQueue   *, reg2 *);
void queueClear1(WBQueue *);
int queueEmpty1(const WBQueue *);
int queueInit1(WBQueue *, const size_t);
int queuePeek1(const WBQueue *, reg2 *);
size_t queueSize1(const WBQueue *);

#endif //PIPELINE_QUEUE2_H
