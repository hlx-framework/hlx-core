package hlx.runtime;

// Transient publish/subscribe - Phase 2 "Core Message Bus" of
// plans/CORE-SHARED-BUS-REGISTRY.md. Thin @:hlNative forwards into hlx-boot's "hlx_bus_*"
// natives (bus.c) - each mod compiles its own copy of this class, but the underlying
// subscriber list lives once per process, in hlx-boot, the same way Registry's store does.
//
// Simple transient pub/sub: no RPC, responses, queues, priorities, or delivery guarantees.
// Re-subscribing the same (topic, handler) pair - same underlying method + receiver, even if
// re-evaluating e.g. `this.onCommand` produced a fresh closure object each time - replaces
// the existing subscription rather than firing the handler twice per publish(); this is the
// same re-init safety net Registry.register already gives its named entries, adapted here to
// Bus's only available identity (topic + handler) since subscribe() has no separate key.
@:access(String)
class Bus {
    @:hlNative("std", "hlx_bus_publish")
    static function hlxPublish(topic:hl.Bytes, payload:Dynamic):Void {}

    @:hlNative("std", "hlx_bus_subscribe")
    static function hlxSubscribe(topic:hl.Bytes, handler:Dynamic):Void {}

    @:hlNative("std", "hlx_bus_unsubscribe")
    static function hlxUnsubscribe(topic:hl.Bytes, handler:Dynamic):Void {}

    public static inline function publish(topic:String, payload:Dynamic):Void {
        hlxPublish(topic.bytes, payload);
    }

    public static inline function subscribe(topic:String, handler:Dynamic->Void):Void {
        hlxSubscribe(topic.bytes, handler);
    }

    // Not required by every subscriber - only needed by one that wants to proactively stop
    // listening (e.g. on its own window/panel close) rather than rely on the
    // replace-on-resubscribe behavior described above.
    public static inline function unsubscribe(topic:String, handler:Dynamic->Void):Void {
        hlxUnsubscribe(topic.bytes, handler);
    }
}
