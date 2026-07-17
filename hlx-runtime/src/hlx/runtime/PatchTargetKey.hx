package hlx.runtime;

// Independent copy of hlx.PatchTargetKey - safe because the abstract fully erases to a plain String, so cross-module key equality is string-based.
abstract PatchTargetKey(String) {
    public inline function new(typeName:String, methodName:String) {
        this = typeName + "." + methodName;
    }

    public inline function toString():String {
        return this;
    }
}
