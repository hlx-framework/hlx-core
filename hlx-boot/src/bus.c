#include "bus.h"
#include "boot.h"
#include "hlx_common.h"
#include "reflection.h"
#include "log.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define BUS_TOPIC_CAP 1024

/* Heap-allocated, singly-linked - same rationale as registry.c's RegistryEntry: hl_add_root
 * roots the FIXED address &entry->handler, so entries must never relocate (rules out a
 * realloc-based array). Unlike the Registry's unique (bucket,key), a topic can have MULTIPLE
 * subscribers, so this is its own list shape rather than reusing RegistryEntry. */
typedef struct BusSubscriber {
    struct BusSubscriber *next;
    char topic[BUS_TOPIC_CAP];
    void *handler; /* rooted hlx_vclosure_mirror_t* - see bus_subscribe */
} BusSubscriber;

static BusSubscriber *g_subscribers = NULL;

/* Two independently-allocated vclosures for "the same" bound Haxe method (e.g. a mod
 * re-evaluating `this.onCommand` on every subscribe() call, which is NOT cached by Haxe
 * itself) are never pointer-identical, but always share the same code address (.fun) and
 * receiver (.value) - compare those instead of the raw closure pointer. This is what lets
 * bus_subscribe recognize (and replace, not accumulate) a re-subscription - see that
 * function's own comment for why this matters. */
static bool SameHandler(void *a, void *b)
{
    if (a == b) return true;
    if (!a || !b) return false;
    hlx_vclosure_mirror_t *ca = (hlx_vclosure_mirror_t *)a;
    hlx_vclosure_mirror_t *cb = (hlx_vclosure_mirror_t *)b;
    return ca->fun == cb->fun && ca->value == cb->value;
}

/* Shared by bus_subscribe (replace, don't duplicate, an existing identical (topic,handler)
 * subscription) and bus_unsubscribe - removes at most the one matching entry, leaving every
 * OTHER subscriber on this topic untouched. */
static void EraseSubscriber(const char *topic, void *handler)
{
    BusSubscriber *prev = NULL;
    for (BusSubscriber *e = g_subscribers; e; prev = e, e = e->next) {
        if (strcmp(e->topic, topic) == 0 && SameHandler(e->handler, handler)) {
            hlx_remove_root(&e->handler);
            if (prev) prev->next = e->next;
            else g_subscribers = e->next;
            free(e);
            return;
        }
    }
}

/* No unsubscribe was specified by the plan doc, but subscribe() still needs SOME
 * hot-reload/re-init safety net: a bare "always append" would let a mod that calls
 * subscribe() again for the same topic+handler (e.g. its own main() re-running) accumulate
 * duplicate subscriptions forever, each firing independently on every future publish() -
 * unlike the Registry, which already gets this for free from its unique (bucket,key)
 * replace-on-register semantics. Bus has no such key, so (topic, handler-identity) is used
 * instead: replace, don't duplicate. An explicit unsubscribe() is ALSO exposed below for a
 * mod that wants to proactively stop listening (e.g. on its own panel/window close) rather
 * than wait for a resubscribe to replace it - the natural symmetric counterpart to
 * subscribe(), same as registry.c's register()/unregister() pair. */
void bus_subscribe(const unsigned short *topic, void *handler)
{
    if (!topic || !handler) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] bus_subscribe: called with a null topic/handler - ignoring");
        return;
    }
    if (!hlx_gc_ready()) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] bus_subscribe: hl_add_root not resolved - ignoring");
        return;
    }

    char narrowTopic[BUS_TOPIC_CAP];
    hlx_narrow_utf16(topic, narrowTopic, sizeof(narrowTopic));

    EraseSubscriber(narrowTopic, handler);

    BusSubscriber *entry = (BusSubscriber *)malloc(sizeof(BusSubscriber));
    if (!entry) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] bus_subscribe: allocation failed for topic '%s' - ignoring", narrowTopic);
        return;
    }
    strcpy_s(entry->topic, sizeof(entry->topic), narrowTopic);
    entry->handler = handler;
    entry->next = g_subscribers;
    g_subscribers = entry;
    hlx_add_root(&entry->handler); /* keeps this mod-owned closure alive - nothing else in the process references it */

    hlx_log(HLX_LOG_DEBUG, "[hlx-boot] bus_subscribe: topic '%s' -> handler %p", narrowTopic, handler);
}

HLX_NATIVE_EXPORT(hlp_hlx_bus_subscribe, "PBD_v", bus_subscribe)

void bus_unsubscribe(const unsigned short *topic, void *handler)
{
    if (!topic || !handler) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] bus_unsubscribe: called with a null topic/handler - ignoring");
        return;
    }
    char narrowTopic[BUS_TOPIC_CAP];
    hlx_narrow_utf16(topic, narrowTopic, sizeof(narrowTopic));
    EraseSubscriber(narrowTopic, handler);
}

HLX_NATIVE_EXPORT(hlp_hlx_bus_unsubscribe, "PBD_v", bus_unsubscribe)

void bus_publish(const unsigned short *topic, void *payload)
{
    if (!topic) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] bus_publish: called with a null topic - ignoring");
        return;
    }
    char narrowTopic[BUS_TOPIC_CAP];
    hlx_narrow_utf16(topic, narrowTopic, sizeof(narrowTopic));

    void *args[1];
    args[0] = payload;
    int delivered = 0;
    /* next is captured before dispatch, not read off `e` afterward - a handler that reacts to
     * its OWN delivery by calling bus_unsubscribe (a plausible, legitimate one-shot-listener
     * pattern) would otherwise free `e` out from under this loop. This does NOT protect
     * against a handler mutating some OTHER not-yet-visited entry's node (e.g. unsubscribing a
     * different topic's earlier-registered handler) - full iterator-invalidation safety is not
     * worth the complexity for this simple a mechanism; the plan doc explicitly disclaims any
     * delivery guarantees. */
    for (BusSubscriber *e = g_subscribers; e; ) {
        BusSubscriber *next = e->next;
        if (strcmp(e->topic, narrowTopic) == 0) {
            call_closure(e->handler, args, 1);
            delivered++;
        }
        e = next;
    }
    hlx_log(HLX_LOG_DEBUG, "[hlx-boot] bus_publish: topic '%s' -> %d subscriber(s)", narrowTopic, delivered);
}

HLX_NATIVE_EXPORT(hlp_hlx_bus_publish, "PBD_v", bus_publish)
