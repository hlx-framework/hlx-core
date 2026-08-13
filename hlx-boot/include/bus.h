#ifndef HLX_BUS_H
#define HLX_BUS_H

/* Transient publish/subscribe - Phase 2 "Core Message Bus" of
 * plans/CORE-SHARED-BUS-REGISTRY.md. Unlike the Registry (Phase 1, registry.c), a topic can
 * have MULTIPLE subscribers, so this is its own linked-list shape, not a reuse of
 * registry.c's RegistryEntry. No RPC, responses, queues, priorities, or delivery guarantees -
 * publish() just calls every current subscriber's closure with `payload` as its one argument,
 * in registration order, and discards any return value. */

void bus_publish(const unsigned short *topic, void *payload);
void bus_subscribe(const unsigned short *topic, void *handler);
void bus_unsubscribe(const unsigned short *topic, void *handler);

#endif /* HLX_BUS_H */
