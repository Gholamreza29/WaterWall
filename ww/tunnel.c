#include "tunnel.h"
#include "buffer_pool.h"
#include "string.h" // memset
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

extern line_t        *newLine(uint8_t tid);
extern uint8_t        reserveChainStateIndex(line_t *l);
extern void           destroyLine(line_t *l);
extern void           destroyContext(context_t *c);
extern void           internalUnRefLine(line_t *l);
extern bool           isAlive(line_t *line);
extern void           reuseContextBuffer(context_t *c);
extern bool           isFullyAuthenticated(line_t *line);
extern bool           isAuthenticated(line_t *line);
extern void           lockLine(line_t *line);
extern void           unLockLine(line_t *line);
extern void           markAuthenticated(line_t *line);
extern context_t     *newContext(line_t *line);
extern context_t     *newContextFrom(context_t *source);
extern context_t     *newEstContext(line_t *line);
extern context_t     *newFinContext(line_t *line);
extern context_t     *newFinContextFrom(context_t *source);
extern context_t     *newInitContext(line_t *line);
extern context_t     *switchLine(context_t *c, line_t *line);
extern buffer_pool_t *getThreadBufferPool(uint8_t tid);
extern buffer_pool_t *getLineBufferPool(line_t *l);
extern buffer_pool_t *getContextBufferPool(context_t *c);
extern void           setupLineUpSide(line_t *l, LineFlowSignal pause_cb, void *state, LineFlowSignal resume_cb);
extern void           setupLineDownSide(line_t *l, LineFlowSignal pause_cb, void *state, LineFlowSignal resume_cb);
extern void           doneLineUpSide(line_t *l);
extern void           doneLineDownSide(line_t *l);
extern void           pauseLineUpSide(line_t *l);
extern void           pauseLineDownSide(line_t *l);
extern void           resumeLineUpSide(line_t *l);
extern void           resumeLineDownSide(line_t *l);

// `from` upstreams to `to`
void chainUp(tunnel_t *from, tunnel_t *to)
{
    from->up = to;
}
// `to` downstreams to `from`
void chainDown(tunnel_t *from, tunnel_t *to)
{
    // assert(to->dw == NULL); // 2 nodes cannot chain to 1 exact node
    // such chains are possible by a generic listener adapter
    // but the cyclic refrence detection is already done in node map
    to->dw = from;
}
// `from` <-> `to`
void chain(tunnel_t *from, tunnel_t *to)
{
    chainUp(from, to);
    chainDown(from, to);
    to->chain_index = from->chain_index + 1;
}

tunnel_t *newTunnel(void)
{
    tunnel_t *t = malloc(sizeof(tunnel_t));
    *t          = (tunnel_t){
                 .upStream   = &defaultUpStream,
                 .downStream = &defaultDownStream,
    };
    return t;
}

pool_item_t *allocLinePoolHandle(struct generic_pool_s *pool)
{
    (void) pool;
    return malloc(sizeof(line_t));
}
void destroyLinePoolHandle(struct generic_pool_s *pool, pool_item_t *item)
{
    (void) pool;
    free(item);
}

pool_item_t *allocContextPoolHandle(struct generic_pool_s *pool)
{
    (void) pool;
    return malloc(sizeof(context_t));
}

void destroyContextPoolHandle(struct generic_pool_s *pool, pool_item_t *item)
{
    (void) pool;
    free(item);
}

void defaultUpStream(tunnel_t *self, context_t *c)
{
    if (self->up != NULL)
    {
        self->up->upStream(self->up, c);
    }
}

void defaultDownStream(tunnel_t *self, context_t *c)
{
    if (self->dw != NULL)
    {
        self->dw->downStream(self->dw, c);
    }
}
