/*
********************************************************************************
** /file   queue.c
** /author eden barby
** /date   22aug2019
** /brief  a first in first out queue using a circular buffer as the underlying
**         implementation. queue sizes of the form 1<<n are more efficient due
**         to the simplification of the modulo operation. queue utilizes size-1
**         of the buffer to simplify distinction between buffer full/empty.
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


/* includes *******************************************************************/

#include "queue.h"

/* private defines ************************************************************/
/* private macros *************************************************************/
/* private variables **********************************************************/
/* private function prototypes ************************************************/


enum QueueStatus queue_init(struct QueueHandler *q, uint32_t size, uint8_t *buffer) {
    if(!q || !size || !buffer) return QUEUE_ERROR;

    q->size = size;
    q->read = 0;
    q->write = 0;
    q->buffer = buffer;

    return QUEUE_OK;
}

enum QueueStatus queue_reset(struct QueueHandler *q) {
    if(!q) return QUEUE_ERROR;

    q->read = 0;
    q->write = 0;

    return QUEUE_OK;
}

enum QueueStatus queue_read(struct QueueHandler *q, uint8_t *item) {
    if(!q || !item) return QUEUE_ERROR;
    if(q->write == q->read) return QUEUE_UNDERFLOW;

    (*item) = q->buffer[q->read++ % q->size];

    return QUEUE_OK;
}

enum QueueStatus queue_write(struct QueueHandler *q, uint8_t item) {
    if(!q) return QUEUE_ERROR;
    if((q->write - q->read) >= (q->size - 1)) return QUEUE_OVERFLOW;

    q->buffer[q->write++ % q->size] = item;

    return QUEUE_OK;
}

uint32_t queue_used(struct QueueHandler *q) {
    return q->write - q->read;
}

uint32_t queue_free(struct QueueHandler *q) {
    return (q->size - 1) - (q->write - q->read);
}
