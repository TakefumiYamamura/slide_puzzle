#include "./pq.h"
#include "./utils.h"

#include <assert.h>
#include <stdint.h>

typedef struct pq_entry_tag
{
    State state;
    int   p;
} PQEntryData;
typedef PQEntryData *PQEntry;

/*
 * NOTE:
 * This priority queue is implemented doubly reallocated array.
 * It will only extend and will not shrink, for now.
 * It may be improved by using array of layers of iteratively widened array
 */
struct pq_tag
{
    size_t       n_elems;
    size_t       capa;
    PQEntryData *array;
};

static inline size_t
calc_init_capa(size_t capa_hint)
{
    size_t capa = 1;
    assert(capa_hint > 0);

    while (capa < capa_hint)
        capa <<= 1;
    return capa - 1;
}

PQ
pq_init(size_t init_capa_hint)
{
    PQ pq = palloc(sizeof(*pq));

    pq->n_elems = 0;
    pq->capa    = calc_init_capa(init_capa_hint);

    assert(pq->capa <= SIZE_MAX / sizeof(State));
    pq->array = palloc(sizeof(PQEntry) * pq->capa);

    return pq;
}

void
pq_fini(PQ pq)
{
    for (size_t i = 0; i < pq->n_elems; ++i)
        state_fini(pq->array[i].state);

    pfree(pq->array);
    pfree(pq);
}

static inline bool
pq_is_full(PQ pq)
{
    assert(pq->n_elems <= pq->capa);
    return pq->n_elems == pq->capa;
}

static inline void
pq_extend(PQ pq)
{
    pq->capa = (pq->capa << 1) + 1;
    assert(pq->capa <= SIZE_MAX / sizeof(PQEntryData));

    pq->array = repalloc(pq->array, sizeof(PQEntryData) * pq->capa);
}

static inline void
pq_swap_entry(PQ pq, size_t i, size_t j)
{
    PQEntryData tmp = pq->array[i];
    pq->array[i]    = pq->array[j];
    pq->array[j]    = tmp;
}

static inline size_t
pq_up(size_t i)
{
    /* NOTE: By using 1-origin, it may be written more simply, i >> 1 */
    return (i >> 1) - 1;
}

static inline size_t
pq_left(size_t i)
{
    return (i << 1) + 1;
}

static void
heapify_up(PQ pq)
{
    for (size_t i = pq->n_elems;;)
    {
        size_t ui = pq_up(i);
        if (pq->array[i].p >= pq->array[ui].p)
            break;

        pq_swap_entry(pq, i, ui);
        i = ui;
    }
}

void
pq_put(PQ pq, State state, int priority)
{
    if (pq_is_full(pq))
        pq_extend(pq);

    pq->array[pq->n_elems].state = state_copy(state);
    pq->array[pq->n_elems].p     = priority;
    heapify_up(pq);
    ++pq->n_elems;
}

static void
heapify_down(PQ pq)
{
    size_t sentinel = pq->n_elems;

    for (size_t i = 0;;)
    {
        size_t ri, li = pq_left(i);
        if (li >= sentinel)
            break;

        ri = li + 1;
        if (ri >= sentinel)
        {
            if (pq->array[i].p > pq->array[li].p)
                pq_swap_entry(pq, i, li);
            break;
        }

        /* NOTE: If ri == li, it may be good to go right
         * since the filling order is from left */
        if (ri <= li)
        {
            if (pq->array[i].p > pq->array[ri].p)
                pq_swap_entry(pq, i, ri);
            i = ri;
        }
        else
        {
            if (pq->array[i].p > pq->array[li].p)
                pq_swap_entry(pq, i, li);
            i = li;
        }
    }
}

State
pq_pop(PQ pq)
{
    State ret_state;

    if (pq->n_elems == 0)
        return NULL;

    ret_state = pq->array[0].state;

    --pq->n_elems;
    pq->array[0] = pq->array[pq->n_elems];
    heapify_down(pq);

    return ret_state;
}

void
pq_dump(PQ pq)
{
    elog("%s: n_elems=%zu, capa=%zu\n", __func__, pq->n_elems, pq->capa);
    for (size_t i = 0, cr_required = 1; i < pq->n_elems; i++)
    {
        if (i == cr_required)
        {
            elog("\n");
            cr_required = (cr_required << 1) + 1;
        }
        elog("%d ", pq->array[i].p);
    }
    elog("\n");
}
