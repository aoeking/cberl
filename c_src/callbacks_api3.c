#ifdef API3
#include <stdio.h>
#include <string.h>
#include <libcouchbase/couchbase.h>
#include <libcouchbase/n1ql.h>
#include <libcouchbase/api3.h>
#include "erl_nif.h"
#include "callbacks_api3.h"

void get_callback(lcb_t instance,
                  int type,
                  const lcb_RESPBASE *rb)
{
    struct libcouchbase_callback_m *cbm;
    cbm = (struct libcouchbase_callback_m *) ((lcb_RESPBASE *)rb)->cookie;
    const lcb_RESPGET *resp = (const lcb_RESPGET *)rb;
    cbm->ret[cbm->currKey] = malloc(sizeof(struct libcouchbase_callback));
    cbm->ret[cbm->currKey]->key = malloc(resp->nkey);
    memcpy(cbm->ret[cbm->currKey]->key, resp->key, resp->nkey);
    cbm->ret[cbm->currKey]->nkey = resp->nkey;
    cbm->ret[cbm->currKey]->error = resp->rc;
    cbm->ret[cbm->currKey]->flag = resp->itmflags == 0 ? 1 : resp->itmflags;
    cbm->ret[cbm->currKey]->cas = resp->cas;
    if (resp->nvalue > 0) {
        cbm->ret[cbm->currKey]->data = malloc(resp->nvalue);
        memcpy(cbm->ret[cbm->currKey]->data, resp->value, resp->nvalue);
        cbm->ret[cbm->currKey]->size = resp->nvalue;
    }
    cbm->currKey += 1;
}

void unlock_callback(lcb_t instance,
                     int type,
                     const lcb_RESPBASE *rb)
{
    struct libcouchbase_callback *cb;
    cb = (struct libcouchbase_callback *) ((lcb_RESPBASE *)rb)->cookie;
    cb->error = rb->rc;
}

void store_callback(lcb_t instance,
                    int type,
                    const lcb_RESPBASE *rb)
{
    struct libcouchbase_callback *cb;
    cb = (struct libcouchbase_callback *) ((lcb_RESPBASE *)rb)->cookie;
    const lcb_RESPSTOREDUR *resp = (lcb_RESPSTOREDUR *)rb;
    cb->cas = resp->cas;
    cb->error = resp->rc;
}

void counter_callback(lcb_t instance,
                      int type,
                      const lcb_RESPBASE *rb)
{
    struct libcouchbase_callback *cb;
    const lcb_RESPCOUNTER *resp = (const lcb_RESPCOUNTER *)rb;
    cb = (struct libcouchbase_callback *)rb->cookie;
    cb->error = rb->rc;
    cb->flag = 1;
    if (rb->rc == LCB_SUCCESS) {
        cb->cas = resp->cas;
        cb->data = malloc(20*sizeof(char));
        memset(cb->data, 0, 20);
        sprintf(cb->data, "%llu", (long long unsigned) resp->value);
        cb->size = strlen(cb->data);
    }
}

void touch_callback(lcb_t instance,
                    int type,
                    const lcb_RESPBASE *rb)
{
    struct libcouchbase_callback_m *cbm;
    const lcb_RESPTOUCH *resp = (const lcb_RESPTOUCH *)rb;
    cbm = (struct libcouchbase_callback_m *)rb->cookie;
    cbm->ret[cbm->currKey] = malloc(sizeof(struct libcouchbase_callback));
    cbm->ret[cbm->currKey]->key = malloc(resp->nkey);
    memcpy(cbm->ret[cbm->currKey]->key, resp->key, resp->nkey);
    cbm->ret[cbm->currKey]->nkey = resp->nkey;
    cbm->ret[cbm->currKey]->error = rb->rc;
    cbm->currKey += 1;
}

void remove_callback(lcb_t instance,
                     int type,
                     const lcb_RESPBASE *rb)
{
    struct libcouchbase_callback *cb;
    cb = (struct libcouchbase_callback *)rb->cookie;
    cb->error = rb->rc;
}

void http_callback(lcb_t instance,
                   int type,
                   const lcb_RESPBASE *rb)
{
    struct libcouchbase_callback_http *cbh;
    const lcb_RESPHTTP *resp = (const lcb_RESPHTTP *)rb;
    cbh = (struct libcouchbase_callback_http *)rb->cookie;
    cbh->ret.error = rb->rc;
    cbh->status = resp->htstatus;
    if(rb->rc == LCB_SUCCESS) {
        cbh->ret.data = malloc(resp->nbody);
        cbh->ret.size = resp->nbody;
        memcpy(cbh->ret.data, resp->body, resp->nbody);
    }
}

void n1ql_callback(lcb_t instance,
                   int cbtype,
                   const lcb_RESPN1QL *resp)
{
    struct libcouchbase_callback_n1ql *cb;
    cb = (struct libcouchbase_callback_n1ql *) ((lcb_RESPBASE *)resp)->cookie;

    if (! (resp->rflags & LCB_RESP_F_FINAL)) {
        if (cb->currrow == cb->size) {
            cb->size *= 2;
            cb->ret = realloc(cb->ret, sizeof(struct libcouchbase_callback*) * cb->size);
        }

        cb->ret[cb->currrow] = malloc(sizeof(struct libcouchbase_callback));
        cb->ret[cb->currrow]->data = malloc(resp->nrow);
        cb->ret[cb->currrow]->size = resp->nrow;
        memcpy(cb->ret[cb->currrow]->data, resp->row, resp->nrow);
        cb->ret[cb->currrow]->error = LCB_SUCCESS;
        cb->currrow++;
    } else {
        cb->meta->data = malloc(resp->nrow);
        cb->meta->size = resp->nrow;
        memcpy(cb->meta->data, resp->row, resp->nrow);
    }
}
#endif
