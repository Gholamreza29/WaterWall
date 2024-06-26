#include "context_queue.h"
#include "buffer_pool.h"
#include "hloop.h"
#include "stc/common.h"
#include "tunnel.h"
#include <stddef.h>
#include <stdlib.h>

#define i_TYPE queue, context_t * // NOLINT
#include "stc/deq.h"
enum
{
    kQCap = 16
};

struct context_queue_s
{
    queue          q;
};

context_queue_t *newContextQueue(void)
{
    context_queue_t *cb = malloc(sizeof(context_queue_t));
    cb->q               = queue_with_capacity(kQCap);
    return cb;
}
void destroyContextQueue(context_queue_t *self)
{
    c_foreach(i, queue, self->q)
    {
        if ((*i.ref)->payload != NULL)
        {
            reuseBuffer(getContextBufferPool((*i.ref)), (*i.ref)->payload);
            CONTEXT_PAYLOAD_DROP((*i.ref));
        }
        destroyContext((*i.ref));
    }

    queue_drop(&self->q);
    free(self);
}

void contextQueuePush(context_queue_t *self, context_t *context)
{
    queue_push_back(&self->q, context);
}

context_t *contextQueuePop(context_queue_t *self)
{
    context_t *context = queue_pull_front(&self->q);
    return context;
}
size_t contextQueueLen(context_queue_t *self)
{
    return queue_size(&self->q);
}
