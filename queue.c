#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define LINUX_SORT
typedef unsigned char u8;
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *q = malloc(sizeof(struct list_head));

    if (!q)
        return NULL;

    INIT_LIST_HEAD(q);
    return q;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (l) {
        element_t *entry = NULL, *safe = NULL;
        list_for_each_entry_safe (entry, safe, l, list)
            q_release_element(entry);
        free(l);
    }
}

/* Create a new element to put into list */
element_t *q_new_ele(char *s)
{
    if (!s)
        return NULL;

    element_t *new_ele = malloc(sizeof(element_t));
    if (!new_ele)
        return NULL;

    INIT_LIST_HEAD(&new_ele->list);

    int len = strlen(s) + 1;
    new_ele->value = malloc(sizeof(char) * len);
    if (!new_ele->value) {
        free(new_ele);
        return NULL;
    }

    strncpy(new_ele->value, s, len);
    new_ele->value[len - 1] = '\0';
    return new_ele;
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new_node = q_new_ele(s);
    if (!new_node)
        return false;

    list_add(&new_node->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new_node = q_new_ele(s);
    if (!new_node)
        return false;

    list_add_tail(&new_node->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    struct list_head *r_node = head->next;
    element_t *r_ele = list_first_entry(head, element_t, list);

    if (sp && bufsize) {
        int min_len = strlen(r_ele->value) + 1 > bufsize
                          ? bufsize
                          : strlen(r_ele->value) + 1;
        strncpy(sp, r_ele->value, min_len);
        sp[min_len - 1] = '\0';
    }
    list_del_init(r_node);
    return r_ele;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    struct list_head *r_node = head->prev;
    element_t *r_ele = list_last_entry(head, element_t, list);

    if (sp && bufsize) {
        int min_len = strlen(r_ele->value) + 1 > bufsize
                          ? bufsize
                          : strlen(r_ele->value) + 1;
        strncpy(sp, r_ele->value, min_len);
        sp[min_len - 1] = '\0';
    }
    list_del_init(r_node);
    return r_ele;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    int cnt = 0;
    struct list_head *tmp = NULL;

    list_for_each (tmp, head)
        cnt++;
    return cnt;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;

    int idx = q_size(head) / 2;
    int cnt = 0;
    struct list_head *tmp = NULL;
    element_t *d_node = NULL;

    list_for_each (tmp, head) {
        if (cnt == idx)
            break;
        cnt++;
    }

    d_node = list_entry(tmp, element_t, list);
    list_del_init(tmp);
    q_release_element(d_node);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (!head)
        return false;

    int marked = 0;
    element_t *entry = NULL, *safe = NULL;

    list_for_each_entry_safe (entry, safe, head, list) {
        if (&safe->list == head) {
            if (marked) {
                list_del_init(&entry->list);
                q_release_element(entry);
            }
            break;
        }

        if (strcmp(entry->value, safe->value) == 0) {
            list_del_init(&entry->list);
            q_release_element(entry);
            marked = 1;
        } else {
            if (marked) {
                list_del_init(&entry->list);
                q_release_element(entry);
            }
            marked = 0;
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *cur = NULL;

    for (cur = head->next; cur != head && cur->next != head; cur = cur->next)
        list_move_tail(cur->next, cur);
}


/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *cur = NULL;

    for (cur = head->next; cur != head && cur->next != head;)
        list_move(cur->next, head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    if (!head || list_empty(head))
        return;

    struct list_head *cur = NULL, *tmp_head = head;
    int i, j;

    for (i = 0; i < q_size(head) && q_size(head) - i >= k; i += k) {
        for (j = 0, cur = tmp_head->next; j < k - 1; j++)
            list_move(cur->next, tmp_head);
        tmp_head = cur;
    }
}

/**************************** My linked list Sort utils
 * ****************************/
/***********************************************************************************/
/***********************************************************************************/

/* Merge two unsorted list into one sorted list in ascending order */
void my_merge(struct list_head *head,
              struct list_head *q1,
              struct list_head *q2)
{
    element_t *cur_q1 = NULL, *cur_q2 = NULL;

    while (!list_empty(q1) && !list_empty(q2)) {
        cur_q1 = list_entry(q1->next, element_t, list);
        cur_q2 = list_entry(q2->next, element_t, list);

        if (strcmp(cur_q1->value, cur_q2->value) <= 0)
            list_move_tail(q1->next, head);
        else
            list_move_tail(q2->next, head);
    }

    if (!list_empty(q1))
        list_splice_tail_init(q1, head);
    if (!list_empty(q2))
        list_splice_tail_init(q2, head);
}

/**************************** Linux kernel Sort utils
 * ****************************/
/*********************************************************************************/
/*********************************************************************************/
typedef int
    __attribute__((nonnull(2, 3))) (*list_cmp_func_t)(void *,
                                                      const struct list_head *,
                                                      const struct list_head *);

// static int linux_cmp(void *priv , const struct list_head *a, const struct
// list_head *b) {

//     char *a_str = list_entry(a, element_t, list)->value;
//     char *b_str = list_entry(b, element_t, list)->value;

//     return strcmp(a_str, b_str);
// }

/*
 * Returns a list organized in an intermediate format suited
 * to chaining of merge() calls: null-terminated, no reserved or
 * sentinel head node, "prev" links not maintained.
 */
__attribute__((nonnull(2, 3, 4))) static struct list_head *
merge(void *priv, list_cmp_func_t cmp, struct list_head *a, struct list_head *b)
{
    struct list_head *head = NULL, **tail = &head;

    for (;;) {
        /* if equal, take 'a' -- important for sort stability */
        if (cmp(priv, a, b) <= 0) {
            *tail = a;
            tail = &a->next;
            a = a->next;
            if (!a) {
                *tail = b;
                break;
            }
        } else {
            *tail = b;
            tail = &b->next;
            b = b->next;
            if (!b) {
                *tail = a;
                break;
            }
        }
    }
    return head;
}

/*
 * Combine final list merge with restoration of standard doubly-linked
 * list structure.  This approach duplicates code from merge(), but
 * runs faster than the tidier alternatives of either a separate final
 * prev-link restoration pass, or maintaining the prev links
 * throughout.
 */
__attribute__((nonnull(2, 3, 4, 5))) static void merge_final(
    void *priv,
    list_cmp_func_t cmp,
    struct list_head *head,
    struct list_head *a,
    struct list_head *b)
{
    struct list_head *tail = head;
    u8 count = 0;

    for (;;) {
        /* if equal, take 'a' -- important for sort stability */
        if (cmp(priv, a, b) <= 0) {
            tail->next = a;
            a->prev = tail;
            tail = a;
            a = a->next;
            if (!a)
                break;
        } else {
            tail->next = b;
            b->prev = tail;
            tail = b;
            b = b->next;
            if (!b) {
                b = a;
                break;
            }
        }
    }

    /* Finish linking remainder of list b on to tail */
    tail->next = b;
    do {
        /*
         * If the merge is highly unbalanced (e.g. the input is
         * already sorted), this loop may run many iterations.
         * Continue callbacks to the client even though no
         * element comparison is needed, so the client's cmp()
         * routine can invoke cond_resched() periodically.
         */
        if (unlikely(!++count))
            cmp(priv, b, b);
        b->prev = tail;
        tail = b;
        b = b->next;
    } while (b);

    /* And the final links to make a circular doubly-linked list */
    tail->next = head;
    head->prev = tail;
}

/**
 * list_sort - sort a list
 * @priv: private data, opaque to list_sort(), passed to @cmp
 * @head: the list to sort
 * @cmp: the elements comparison function
 *
 * The comparison function @cmp must return > 0 if @a should sort after
 * @b ("@a > @b" if you want an ascending sort), and <= 0 if @a should
 * sort before @b *or* their original order should be preserved.  It is
 * always called with the element that came first in the input in @a,
 * and list_sort is a stable sort, so it is not necessary to distinguish
 * the @a < @b and @a == @b cases.
 *
 * This is compatible with two styles of @cmp function:
 * - The traditional style which returns <0 / =0 / >0, or
 * - Returning a boolean 0/1.
 * The latter offers a chance to save a few cycles in the comparison
 * (which is used by e.g. plug_ctx_cmp() in block/blk-mq.c).
 *
 * A good way to write a multi-word comparison is::
 *
 *	if (a->high != b->high)
 *		return a->high > b->high;
 *	if (a->middle != b->middle)
 *		return a->middle > b->middle;
 *	return a->low > b->low;
 *
 *
 * This mergesort is as eager as possible while always performing at least
 * 2:1 balanced merges.  Given two pending sublists of size 2^k, they are
 * merged to a size-2^(k+1) list as soon as we have 2^k following elements.
 *
 * Thus, it will avoid cache thrashing as long as 3*2^k elements can
 * fit into the cache.  Not quite as good as a fully-eager bottom-up
 * mergesort, but it does use 0.2*n fewer comparisons, so is faster in
 * the common case that everything fits into L1.
 *
 *
 * The merging is controlled by "count", the number of elements in the
 * pending lists.  This is beautifully simple code, but rather subtle.
 *
 * Each time we increment "count", we set one bit (bit k) and clear
 * bits k-1 .. 0.  Each time this happens (except the very first time
 * for each bit, when count increments to 2^k), we merge two lists of
 * size 2^k into one list of size 2^(k+1).
 *
 * This merge happens exactly when the count reaches an odd multiple of
 * 2^k, which is when we have 2^k elements pending in smaller lists,
 * so it's safe to merge away two lists of size 2^k.
 *
 * After this happens twice, we have created two lists of size 2^(k+1),
 * which will be merged into a list of size 2^(k+2) before we create
 * a third list of size 2^(k+1), so there are never more than two pending.
 *
 * The number of pending lists of size 2^k is determined by the
 * state of bit k of "count" plus two extra pieces of information:
 *
 * - The state of bit k-1 (when k == 0, consider bit -1 always set), and
 * - Whether the higher-order bits are zero or non-zero (i.e.
 *   is count >= 2^(k+1)).
 *
 * There are six states we distinguish.  "x" represents some arbitrary
 * bits, and "y" represents some arbitrary non-zero bits:
 * 0:  00x: 0 pending of size 2^k;           x pending of sizes < 2^k
 * 1:  01x: 0 pending of size 2^k; 2^(k-1) + x pending of sizes < 2^k
 * 2: x10x: 0 pending of size 2^k; 2^k     + x pending of sizes < 2^k
 * 3: x11x: 1 pending of size 2^k; 2^(k-1) + x pending of sizes < 2^k
 * 4: y00x: 1 pending of size 2^k; 2^k     + x pending of sizes < 2^k
 * 5: y01x: 2 pending of size 2^k; 2^(k-1) + x pending of sizes < 2^k
 * (merge and loop back to state 2)
 *
 * We gain lists of size 2^k in the 2->3 and 4->5 transitions (because
 * bit k-1 is set while the more significant bits are non-zero) and
 * merge them away in the 5->2 transition.  Note in particular that just
 * before the 5->2 transition, all lower-order bits are 11 (state 3),
 * so there is one list of each smaller size.
 *
 * When we reach the end of the input, we merge all the pending
 * lists, from smallest to largest.  If you work through cases 2 to
 * 5 above, you can see that the number of elements we merge with a list
 * of size 2^k varies from 2^(k-1) (cases 3 and 5 when x == 0) to
 * 2^(k+1) - 1 (second merge of case 5 when x == 2^(k-1) - 1).
 */
__attribute__((nonnull(2, 3))) void list_sort(void *priv,
                                              struct list_head *head,
                                              list_cmp_func_t cmp)
{
    struct list_head *list = head->next, *pending = NULL;
    size_t count = 0; /* Count of pending */

    if (list == head->prev) /* Zero or one elements */
        return;

    /* Convert to a null-terminated singly-linked list. */
    head->prev->next = NULL;

    /*
     * Data structure invariants:
     * - All lists are singly linked and null-terminated; prev
     *   pointers are not maintained.
     * - pending is a prev-linked "list of lists" of sorted
     *   sublists awaiting further merging.
     * - Each of the sorted sublists is power-of-two in size.
     * - Sublists are sorted by size and age, smallest & newest at front.
     * - There are zero to two sublists of each size.
     * - A pair of pending sublists are merged as soon as the number
     *   of following pending elements equals their size (i.e.
     *   each time count reaches an odd multiple of that size).
     *   That ensures each later final merge will be at worst 2:1.
     * - Each round consists of:
     *   - Merging the two sublists selected by the highest bit
     *     which flips when count is incremented, and
     *   - Adding an element from the input as a size-1 sublist.
     */
    do {
        size_t bits;
        struct list_head **tail = &pending;

        /* Find the least-significant clear bit in count */
        for (bits = count; bits & 1; bits >>= 1)
            tail = &(*tail)->prev;
        /* Do the indicated merge */
        if (likely(bits)) {
            struct list_head *a = *tail, *b = a->prev;

            a = merge(priv, cmp, b, a);
            /* Install the merged result in place of the inputs */
            a->prev = b->prev;
            *tail = a;
        }

        /* Move one element from input list to pending */
        list->prev = pending;
        pending = list;
        list = list->next;
        pending->next = NULL;
        count++;
    } while (list);

    /* End of input; merge together all the pending lists. */
    list = pending;
    pending = pending->prev;
    for (;;) {
        struct list_head *next = pending->prev;

        if (!next)
            break;
        list = merge(priv, cmp, pending, list);
        pending = next;
    }
    /* The final merge, rebuilding prev links */
    merge_final(priv, cmp, head, pending, list);
}

/* Sort elements of queue in ascending order */
void q_sort(struct list_head *head)
{
#ifndef LINUX_SORT
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *fast = head->next->next, *slow = head->next;

    // split list
    while (fast != head && fast->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }

    // generate left & right head
    LIST_HEAD(left);
    LIST_HEAD(right);
    list_cut_position(&left, head, slow);
    list_splice_init(head, &right);
    q_sort(&left);
    q_sort(&right);
    my_merge(head, &left, &right);
#else
    list_sort(NULL, head, linux_cmp);
#endif
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    struct list_head *cur = NULL, *max = head->prev;
    element_t *cur_ele = NULL,
              *max_ele = list_entry(head->prev, element_t, list);

    for (cur = max->prev; cur != head; cur = max->prev) {
        cur_ele = list_entry(cur, element_t, list);
        if (strcmp(max_ele->value, cur_ele->value) > 0) {
            list_del_init(cur);
            q_release_element(cur_ele);
        } else {
            max_ele = cur_ele;
            max = cur;
        }
    }
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending order */
int q_merge(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    int k;

    // Total k queue needs ceiling(n/2) times to merge, which means (k + 1) / 2
    for (k = q_size(head); k > 1; k = (k + 1) / 2) {
        struct list_head *cur = head->next, *empty = head->next;
        // k queue needs k/ 2 times merge
        for (int i = 0; i < k / 2; i++) {
            LIST_HEAD(temp);
            my_merge(&temp, list_entry(cur, queue_contex_t, chain)->q,
                     list_entry(cur->next, queue_contex_t, chain)->q);
            list_splice_tail(&temp,
                             list_entry(empty, queue_contex_t, chain)->q);

            cur = cur->next->next;
            empty = empty->next;
        }
        // if there has odd number queues, put the last queue to the next of the
        // last combined queue
        if (k % 2)
            list_splice_tail_init(list_entry(cur, queue_contex_t, chain)->q,
                                  list_entry(empty, queue_contex_t, chain)->q);
    }

    return q_size(list_entry(head->next, queue_contex_t, chain)->q);
}

/* Shuffle elements in queue */
void q_shuffle(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    srand(199);
    struct list_head *cur = NULL, *safe = NULL, *temp = NULL;
    int cnt = q_size(head);

    for (safe = head; cnt >= 1; cnt--, safe = safe->prev) {
        int j = rand() % cnt;
        for (cur = head; j >= 0; j--)
            cur = cur->next;

        if (cur != safe->prev) {
            temp = cur->prev;
            list_move_tail(cur, safe);
            list_move(safe->prev->prev, temp);
        }
    }
}