typedef void *(*HlpFn)(const char **sign);

static void *ResolveRealHlp(const char *name, void **cache)
{
    if (!*cache) *cache = (void *)GetProcAddress(g_realLibhl, name);
    return *cache;
}

__declspec(dllexport) void * WINAPI hlp_alloc_array(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_alloc_array");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_alloc_bytes(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_alloc_bytes");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_alloc_carray(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_alloc_carray");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_alloc_enum_dyn(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_alloc_enum_dyn");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_alloc_obj(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_alloc_obj");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_array_blit(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_array_blit");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_array_bytes(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_array_bytes");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_array_type(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_array_type");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_atomic_add32(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_atomic_add32");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_atomic_and32(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_atomic_and32");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_atomic_compare_exchange32(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_atomic_compare_exchange32");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_atomic_compare_exchange_ptr(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_atomic_compare_exchange_ptr");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_atomic_exchange32(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_atomic_exchange32");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_atomic_exchange_ptr(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_atomic_exchange_ptr");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_atomic_load32(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_atomic_load32");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_atomic_load_ptr(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_atomic_load_ptr");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_atomic_or32(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_atomic_or32");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_atomic_store32(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_atomic_store32");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_atomic_store_ptr(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_atomic_store_ptr");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_atomic_sub32(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_atomic_sub32");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_atomic_xor32(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_atomic_xor32");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_blocking(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_blocking");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_breakpoint(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_breakpoint");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_bsort_f64(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_bsort_f64");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_bsort_i32(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_bsort_i32");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_bsort_i64(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_bsort_i64");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_bytes_address(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_bytes_address");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_bytes_address64(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_bytes_address64");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_bytes_blit(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_bytes_blit");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_bytes_compare(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_bytes_compare");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_bytes_compare16(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_bytes_compare16");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_bytes_fill(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_bytes_fill");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_bytes_find(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_bytes_find");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_bytes_from_address(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_bytes_from_address");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_bytes_from_address64(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_bytes_from_address64");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_bytes_get_memsize(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_bytes_get_memsize");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_bytes_offset(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_bytes_offset");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_bytes_rfind(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_bytes_rfind");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_bytes_subtract(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_bytes_subtract");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_call_method(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_call_method");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_call_stack_raw(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_call_stack_raw");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_carray_blit(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_carray_blit");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_condition_acquire(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_condition_acquire");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_condition_alloc(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_condition_alloc");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_condition_broadcast(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_condition_broadcast");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_condition_release(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_condition_release");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_condition_signal(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_condition_signal");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_condition_timed_wait(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_condition_timed_wait");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_condition_try_acquire(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_condition_try_acquire");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_condition_wait(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_condition_wait");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_date_from_string(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_date_from_string");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_date_from_time(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_date_from_time");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_date_get_inf(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_date_get_inf");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_date_get_time(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_date_get_time");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_date_get_utc_inf(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_date_get_utc_inf");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_date_new(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_date_new");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_date_now(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_date_now");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_date_to_string(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_date_to_string");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_debug_breakpoint(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_debug_breakpoint");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_debug_call(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_debug_call");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_debug_flush(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_debug_flush");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_debug_read(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_debug_read");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_debug_read_register(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_debug_read_register");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_debug_resume(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_debug_resume");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_debug_start(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_debug_start");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_debug_stop(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_debug_stop");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_debug_wait(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_debug_wait");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_debug_write(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_debug_write");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_debug_write_register(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_debug_write_register");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_deque_add(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_deque_add");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_deque_alloc(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_deque_alloc");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_deque_pop(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_deque_pop");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_deque_push(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_deque_push");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_dyn_compare(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_dyn_compare");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_dyn_op(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_dyn_op");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_enum_parameters(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_enum_parameters");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_exception_stack(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_exception_stack");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_exception_stack_raw(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_exception_stack_raw");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_field_name(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_field_name");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_file_close(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_file_close");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_file_contents(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_file_contents");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_file_eof(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_file_eof");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_file_error_code(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_file_error_code");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_file_flush(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_file_flush");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_file_is_locked(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_file_is_locked");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_file_open(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_file_open");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_file_read(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_file_read");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_file_read_char(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_file_read_char");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_file_seek(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_file_seek");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_file_seek2(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_file_seek2");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_file_stderr(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_file_stderr");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_file_stdin(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_file_stdin");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_file_stdout(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_file_stdout");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_file_tell(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_file_tell");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_file_tell2(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_file_tell2");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_file_write(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_file_write");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_file_write_char(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_file_write_char");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_ftos(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_ftos");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_fun_compare(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_fun_compare");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_gc_dump_memory(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_gc_dump_memory");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_gc_enable(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_gc_enable");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_gc_get_flags(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_gc_get_flags");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_gc_get_live_objects(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_gc_get_live_objects");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_gc_major(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_gc_major");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_gc_profile(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_gc_profile");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_gc_safepoint(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_gc_safepoint");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_gc_set_flags(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_gc_set_flags");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_gc_stats(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_gc_stats");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_get_closure_value(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_get_closure_value");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_get_virtual_value(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_get_virtual_value");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hash(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hash");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hballoc(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hballoc");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hbclear(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hbclear");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hbexists(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hbexists");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hbget(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hbget");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hbkeys(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hbkeys");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hbremove(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hbremove");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hbset(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hbset");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hbsize(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hbsize");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hbvalues(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hbvalues");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hi64alloc(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hi64alloc");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hi64clear(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hi64clear");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hi64exists(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hi64exists");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hi64get(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hi64get");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hi64keys(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hi64keys");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hi64remove(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hi64remove");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hi64set(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hi64set");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hi64size(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hi64size");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hi64values(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hi64values");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hialloc(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hialloc");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hiclear(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hiclear");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hiexists(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hiexists");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_higet(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_higet");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hikeys(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hikeys");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hiremove(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hiremove");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hiset(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hiset");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hisize(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hisize");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hivalues(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hivalues");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hoalloc(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hoalloc");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hoclear(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hoclear");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hoexists(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hoexists");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hoget(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hoget");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hokeys(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hokeys");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_horemove(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_horemove");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hoset(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hoset");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hosize(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hosize");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_host_local(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_host_local");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_host_resolve(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_host_resolve");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_host_reverse(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_host_reverse");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_host_to_string(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_host_to_string");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_hovalues(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_hovalues");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_is_prim_loaded(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_is_prim_loaded");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_itos(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_itos");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_lock_create(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_lock_create");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_lock_release(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_lock_release");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_lock_wait(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_lock_wait");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_make_closure(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_make_closure");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_make_var_args(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_make_var_args");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_math_abs(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_math_abs");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_math_acos(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_math_acos");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_math_asin(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_math_asin");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_math_atan(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_math_atan");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_math_atan2(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_math_atan2");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_math_ceil(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_math_ceil");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_math_cos(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_math_cos");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_math_exp(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_math_exp");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_math_fceil(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_math_fceil");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_math_ffloor(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_math_ffloor");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_math_floor(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_math_floor");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_math_fround(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_math_fround");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_math_isfinite(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_math_isfinite");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_math_isnan(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_math_isnan");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_math_log(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_math_log");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_math_pow(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_math_pow");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_math_round(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_math_round");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_math_sin(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_math_sin");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_math_sqrt(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_math_sqrt");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_math_tan(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_math_tan");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_mem_compact(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_mem_compact");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_mutex_acquire(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_mutex_acquire");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_mutex_alloc(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_mutex_alloc");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_mutex_free(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_mutex_free");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_mutex_release(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_mutex_release");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_mutex_try_acquire(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_mutex_try_acquire");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_nan(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_nan");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_no_closure(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_no_closure");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_obj_copy(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_obj_copy");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_obj_delete_field(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_obj_delete_field");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_obj_fields(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_obj_fields");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_obj_get_field(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_obj_get_field");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_obj_has_field(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_obj_has_field");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_obj_set_field(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_obj_set_field");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_parse_float(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_parse_float");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_parse_int(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_parse_int");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_process_close(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_process_close");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_process_exit(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_process_exit");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_process_kill(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_process_kill");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_process_pid(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_process_pid");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_process_run(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_process_run");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_process_stderr_read(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_process_stderr_read");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_process_stdin_close(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_process_stdin_close");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_process_stdin_write(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_process_stdin_write");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_process_stdout_read(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_process_stdout_read");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_ptr_compare(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_ptr_compare");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_regexp_match(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_regexp_match");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_regexp_matched_num(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_regexp_matched_num");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_regexp_matched_pos(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_regexp_matched_pos");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_regexp_new_options(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_regexp_new_options");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_register_guid_name(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_register_guid_name");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_resolve_symbol(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_resolve_symbol");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_rnd_alloc(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_rnd_alloc");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_rnd_float(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_rnd_float");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_rnd_init_system(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_rnd_init_system");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_rnd_int(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_rnd_int");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_rnd_set_seed(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_rnd_set_seed");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_semaphore_acquire(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_semaphore_acquire");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_semaphore_alloc(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_semaphore_alloc");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_semaphore_free(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_semaphore_free");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_semaphore_release(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_semaphore_release");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_semaphore_try_acquire(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_semaphore_try_acquire");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_set_error_handler(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_set_error_handler");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_set_thread_flags(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_set_thread_flags");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_socket_accept(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_socket_accept");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_socket_bind(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_socket_bind");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_socket_close(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_socket_close");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_socket_connect(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_socket_connect");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_socket_fd_size(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_socket_fd_size");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_socket_host(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_socket_host");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_socket_init(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_socket_init");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_socket_listen(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_socket_listen");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_socket_new(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_socket_new");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_socket_peer(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_socket_peer");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_socket_recv(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_socket_recv");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_socket_recv_char(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_socket_recv_char");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_socket_recv_from(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_socket_recv_from");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_socket_select(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_socket_select");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_socket_send(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_socket_send");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_socket_send_char(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_socket_send_char");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_socket_send_to(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_socket_send_to");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_socket_set_blocking(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_socket_set_blocking");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_socket_set_broadcast(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_socket_set_broadcast");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_socket_set_fast_send(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_socket_set_fast_send");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_socket_set_timeout(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_socket_set_timeout");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_socket_shutdown(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_socket_shutdown");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_string_compare(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_string_compare");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_args(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_args");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_check_reload(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_check_reload");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_command(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_command");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_cpu_time(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_cpu_time");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_create_dir(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_create_dir");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_delete(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_delete");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_env(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_env");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_exe_path(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_exe_path");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_exists(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_exists");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_exit(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_exit");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_full_path(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_full_path");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_get_char(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_get_char");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_get_cwd(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_get_cwd");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_get_env(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_get_env");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_getpid(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_getpid");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_has_debugger(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_has_debugger");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_hl_file(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_hl_file");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_is64(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_is64");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_is_dir(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_is_dir");
    return real ? ((HlpFn)real)(sign) : NULL;
}

/* Not a mechanical passthrough - chains to HookedSysLoadPlugin (log.c) so Push/PopModuleName
 * bracket each module's synchronous main(). */
__declspec(dllexport) void * WINAPI hlp_sys_load_plugin(const char **sign)
{
    static void *realWrapper = NULL;
    void *real = ResolveRealHlp("hlp_sys_load_plugin", &realWrapper);
    void *realFn = real ? ((HlpFn)real)(sign) : NULL;
    g_realSysLoadPlugin = (HlSysLoadPluginFn)realFn;
    return (void *)&HookedSysLoadPlugin;
}

__declspec(dllexport) void * WINAPI hlp_sys_locale(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_locale");
    return real ? ((HlpFn)real)(sign) : NULL;
}

/* Not a mechanical passthrough - resolves the real hlp_sys_print to chain to, then
 * returns a fresh per-module trampoline (BuildSysPrintTrampoline in log.c) each call. */
__declspec(dllexport) void * WINAPI hlp_sys_print(const char **sign)
{
    static void *realWrapper = NULL;
    void *real = ResolveRealHlp("hlp_sys_print", &realWrapper);
    void *realFn = real ? ((HlpFn)real)(sign) : NULL;
    g_realSysPrint = (HlSysPrintFn)realFn;
    return BuildSysPrintTrampoline();
}

__declspec(dllexport) void * WINAPI hlp_sys_process_memory(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_process_memory");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_profile_event(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_profile_event");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_put_env(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_put_env");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_read_dir(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_read_dir");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_remove_dir(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_remove_dir");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_rename(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_rename");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_resolve_type(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_resolve_type");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_set_cwd(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_set_cwd");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_set_flags(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_set_flags");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_set_time_locale(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_set_time_locale");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_sleep(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_sleep");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_special(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_special");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_stat(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_stat");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_string(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_string");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_thread_cpu_time(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_thread_cpu_time");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_time(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_time");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_timestamp_ms(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_timestamp_ms");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_utf8_path(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_utf8_path");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_sys_vtune_init(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_sys_vtune_init");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_thread_create(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_thread_create");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_thread_current(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_thread_current");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_thread_get_name(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_thread_get_name");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_thread_set_name(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_thread_set_name");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_tls_alloc(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_tls_alloc");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_tls_get(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_tls_get");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_tls_set(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_tls_set");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_track_count(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_track_count");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_track_entry(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_track_entry");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_track_get_bits(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_track_get_bits");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_track_init(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_track_init");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_track_lock(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_track_lock");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_track_reset(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_track_reset");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_track_set_bits(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_track_set_bits");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_track_set_depth(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_track_set_depth");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_type_args_count(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_type_args_count");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_type_data_size(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_type_data_size");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_type_enum_eq(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_type_enum_eq");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_type_enum_fields(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_type_enum_fields");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_type_enum_values(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_type_enum_values");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_type_get_global(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_type_get_global");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_type_instance_fields(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_type_instance_fields");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_type_name(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_type_name");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_type_safe_cast(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_type_safe_cast");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_type_set_global(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_type_set_global");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_type_str(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_type_str");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_type_super(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_type_super");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_ucs2_lower(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_ucs2_lower");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_ucs2_upper(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_ucs2_upper");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_ucs2length(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_ucs2length");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_url_decode(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_url_decode");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_url_encode(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_url_encode");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_utf16_to_utf8(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_utf16_to_utf8");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_utf8_to_utf16(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_utf8_to_utf16");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_value_address(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_value_address");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_value_cast(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_value_cast");
    return real ? ((HlpFn)real)(sign) : NULL;
}

__declspec(dllexport) void * WINAPI hlp_value_to_string(const char **sign)
{
    static void *real = NULL;
    if (!real) real = (void *)GetProcAddress(g_realLibhl, "hlp_value_to_string");
    return real ? ((HlpFn)real)(sign) : NULL;
}
