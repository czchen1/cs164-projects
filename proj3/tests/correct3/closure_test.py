def incr(n):
    # """Return a vector of two functions, each of which increments
    # an argument k by N+1."""
    def p(k::int):
        return k + n
    x = p
    n = n + 1
    y = p
    return [x, y]
w = incr(3)
print w[0](5), w[1](5) # Should print 9 (= 5 + (3 + 1)) twice
