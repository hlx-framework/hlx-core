package hlx.runtime;

enum HlxPrefixResult<T> {
    Continue;
    Skip;
    SkipWith(result:T);
}
