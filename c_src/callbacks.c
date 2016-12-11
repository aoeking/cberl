#include <stdio.h>
#include <string.h>
#include <libcouchbase/couchbase.h>
#include <libcouchbase/n1ql.h>
#include <libcouchbase/api3.h>
#include "erl_nif.h"
#include "callbacks.h"

void api3_get_callback(lcb_t instance,
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
        printf("Result is: %.*s\r\n", (int)resp->nvalue, resp->value);
        cbm->ret[cbm->currKey]->data = malloc(resp->nvalue);
        memcpy(cbm->ret[cbm->currKey]->data, resp->value, resp->nvalue);
        cbm->ret[cbm->currKey]->size = resp->nvalue;
    }
    cbm->currKey += 1;
}

void get_callback(lcb_t instance,
                   const void *cookie,
                  lcb_error_t error,
                  const lcb_get_resp_t *item)
{
    struct libcouchbase_callback_m *cbm;
    cbm = (struct libcouchbase_callback_m *) cookie;
    cbm->ret[cbm->currKey] = malloc(sizeof(struct libcouchbase_callback));
    cbm->ret[cbm->currKey]->key = malloc(item->v.v0.nkey);
    memcpy(cbm->ret[cbm->currKey]->key, item->v.v0.key, item->v.v0.nkey);
    cbm->ret[cbm->currKey]->nkey = item->v.v0.nkey;
    cbm->ret[cbm->currKey]->error = error;
    cbm->ret[cbm->currKey]->flag = item->v.v0.flags == 0 ? 1 : item->v.v0.flags;
    cbm->ret[cbm->currKey]->cas = item->v.v0.cas;
    if (error == LCB_SUCCESS) {
        cbm->ret[cbm->currKey]->data = malloc(item->v.v0.nbytes);
        memcpy(cbm->ret[cbm->currKey]->data, item->v.v0.bytes, item->v.v0.nbytes);
        cbm->ret[cbm->currKey]->size = item->v.v0.nbytes;
    }
    cbm->currKey += 1;
}

void arithmetic_callback(lcb_t instance,
                         const void *cookie,
                         lcb_error_t error,
                         const lcb_arithmetic_resp_t *resp)
{
    struct libcouchbase_callback *cb;
    cb = (struct libcouchbase_callback *)cookie;
    cb->error = error;
    cb->flag = 1;
    if (error == LCB_SUCCESS) {
        cb->cas = resp->v.v0.cas;
        cb->data = malloc(20*sizeof(char));
        memset(cb->data, 0, 20);
        sprintf(cb->data, "%llu", (long long unsigned) resp->v.v0.value);
        cb->size = strlen(cb->data);
    }
}

void unlock_callback(lcb_t instance,
                     const void *cookie,
                     lcb_error_t error,
                     const lcb_unlock_resp_t *resp)
{
    (void)instance;
    struct libcouchbase_callback *cb;
    cb = (struct libcouchbase_callback *)cookie;
    cb->error = error;
}

void touch_callback(lcb_t instance,
                    const void *cookie,
                    lcb_error_t error,
                    const lcb_touch_resp_t *resp)
{
    (void)instance;
    struct libcouchbase_callback_m *cbm;
    cbm = (struct libcouchbase_callback_m *)cookie;
    cbm->ret[cbm->currKey] = malloc(sizeof(struct libcouchbase_callback));
    cbm->ret[cbm->currKey]->key = malloc(resp->v.v0.nkey);
    memcpy(cbm->ret[cbm->currKey]->key, resp->v.v0.key, resp->v.v0.nkey);
    cbm->ret[cbm->currKey]->nkey = resp->v.v0.nkey;
    cbm->ret[cbm->currKey]->error = error;
    cbm->currKey += 1;
}

void store_callback(lcb_t instance,
                    const void *cookie,
                    lcb_storage_t operation,
                    lcb_error_t error,
                    const lcb_store_resp_t *item)
{

    (void)instance; (void)operation;
    struct libcouchbase_callback *cb;
    cb = (struct libcouchbase_callback *)cookie;
    cb->error = error;
    cb->cas = item->v.v0.cas;
}

void remove_callback(lcb_t instance,
                     const void *cookie,
                     lcb_error_t error,
                     const lcb_remove_resp_t *resp)
{
    (void)instance;
    struct libcouchbase_callback *cb;
    cb = (struct libcouchbase_callback *)cookie;
    cb->error = error;
}

void http_callback(lcb_http_request_t request,
                   lcb_t instance,
                   const void* cookie,
                   lcb_error_t error,
                   const lcb_http_resp_t *resp)
{
    (void)instance;
    struct libcouchbase_callback_http *cbh;
    cbh = (struct libcouchbase_callback_http *)cookie;
    cbh->ret.error = error;
    cbh->status = resp->v.v0.status;
    if(error == LCB_SUCCESS) {
        cbh->ret.data = malloc(resp->v.v0.nbytes);
        cbh->ret.size = resp->v.v0.nbytes;
        memcpy(cbh->ret.data, resp->v.v0.bytes, resp->v.v0.nbytes);
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

void api3_subdoc_get_callback(lcb_t instance,
                              int type,
                              const lcb_RESPBASE *rb)
{
    printf("Got callback for %s\n", lcb_strcbtype(type));

    if (rb->rc != LCB_SUCCESS && rb->rc != LCB_SUBDOC_MULTI_FAILURE) {
        printf("Failure: 0x%x\n", rb->rc);
        return;
    }

    if (type == LCB_CALLBACK_GET) {
        const lcb_RESPGET *rg = (const lcb_RESPGET *)rb;
        printf("Result is: %.*s\n", (int)rg->nvalue, rg->value);
    } else if (type == LCB_CALLBACK_SDLOOKUP || type == LCB_CALLBACK_SDMUTATE) {
        lcb_SDENTRY ent;
        size_t iter = 0;
        size_t oix = 0;
        const lcb_RESPSUBDOC *resp = (lcb_RESPSUBDOC*)rb;
        while (lcb_sdresult_next(resp, &ent, &iter)) {
            size_t index = oix++;
            if (type == LCB_CALLBACK_SDMUTATE) {
                index = ent.index;
            }
            printf("element value :- [%lu]: 0x%x. %.*s\n",
                   index, ent.status, (int)ent.nvalue, ent.value);
        }
    }
}
