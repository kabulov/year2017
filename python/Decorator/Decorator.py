import time
import functools

def profiler(func):
    function_name = 'before_recursion'
    start_time = 0
    counter = 0

    @functools.wraps(func)
    def call_this_always(*args, **kwargs):
        if function_name == 'before_recursion':
            return before_recursion(*args, **kwargs)
        nonlocal counter
        counter += 1
        call_this_always.calls = counter
        result = func(*args, **kwargs)
        nonlocal start_time
        call_this_always.last_time_taken = time.time() - start_time
        return result

    def before_recursion(*args, **kwargs):
        nonlocal function_name
        function_name = 'recursion'
        nonlocal start_time
        start_time = time.time()
        nonlocal counter
        counter = 0
        return_value = call_this_always(*args, **kwargs)
        function_name = 'before_recursion'
        return return_value

    return call_this_always


@profiler
def akker(m, n):
    if m == 0:
        return n + 1
    elif n == 0:
        return akker(m - 1, 1)
    else:
        return akker(m - 1, akker(m, n - 1))

akker(3,4)
print(akker.calls)
print(akker.last_time_taken)
