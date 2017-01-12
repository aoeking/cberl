#ifndef CALLBACKS_API3_H
#define CALLBACKS_API3_H

struct libcouchbase_callback {
    lcb_error_t error;
    size_t size;
    void *data;
    void *key;
    size_t nkey;
    lcb_uint32_t flag;
    lcb_cas_t cas;
};

struct libcouchbase_callback_http {
    lcb_http_status_t status;
    struct libcouchbase_callback ret;
};

struct libcouchbase_callback_m {
    int currKey;
    struct libcouchbase_callback** ret;
};

struct libcouchbase_callback_n1ql {
    int currrow;
    int size;
    struct libcouchbase_callback** ret;
    struct libcouchbase_callback* meta;
};

void get_callback(lcb_t instance,
                  int type,
                  const lcb_RESPBASE *rb);

void counter_callback(lcb_t instance,
                      int type,
                      const lcb_RESPBASE *rb);

void unlock_callback(lcb_t instance,
                     int type,
                     const lcb_RESPBASE *rb);

void touch_callback(lcb_t instance,
                    int type,
                    const lcb_RESPBASE *rb);

void store_callback(lcb_t instance,
                    int type,
                    const lcb_RESPBASE *rb);

void remove_callback(lcb_t instance,
                     int type,
                     const lcb_RESPBASE *rb);

void http_callback(lcb_t instance,
                   int type,
                   const lcb_RESPBASE *resp);

void n1ql_callback(lcb_t instance,
                   int type,
                   const lcb_RESPN1QL *resp);

#endif
