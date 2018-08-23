# Test that you are not allowed to use types on the right sides of assignments.
# They are only allowed for target variables.
x = 4
y::int = x::int
