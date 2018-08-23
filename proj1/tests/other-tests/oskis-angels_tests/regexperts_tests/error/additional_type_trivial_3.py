# Test a syntactically wrong example of using static typing.
def foo(x::int, y::int)::int:
    return x + y

fn_ptr::int->(int, int) = foo
