/*
********************************************************************************
** /file   queue.h
** /author eden barby
** /date   22aug2019
** /brief  a first in first out queue using a circular buffer as the underlying
**         implimentation
********************************************************************************
** external functions
********************************************************************************
** queue_init  initialises the queue
** queue_reset resets the read and write indexes, does not zeroize
** queue_read  dequeues the next element
** queue_write enqueues the element
** queue_used  returns the number of elements in the queue
** queue_free  returns the amount of free space in the queue
********************************************************************************
*/


#ifndef QUEUE_H
#define QUEUE_H


#include <stdint.h>


struct QueueHandler {
    uint32_t size;
    volatile uint32_t read;
    volatile uint32_t write;
    volatile uint8_t *buffer;
};

enum QueueStatus {
    QUEUE_OK        = 0,
    QUEUE_UNDERFLOW = 1,
    QUEUE_OVERFLOW  = 2,
    QUEUE_ERROR     = 3
};


extern enum QueueStatus queue_init(struct QueueHandler *q, uint32_t size, uint8_t *buffer);
extern enum QueueStatus queue_reset(struct QueueHandler *q);
extern enum QueueStatus queue_read(struct QueueHandler *q, uint8_t *item);
extern enum QueueStatus queue_write(struct QueueHandler *q, uint8_t item);
extern uint32_t queue_used(struct QueueHandler *q);
extern uint32_t queue_free(struct QueueHandler *q);


#endif /* QUEUE_H */
