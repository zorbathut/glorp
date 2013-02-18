assert(loadfile("glorp/init_stdlib_coro.lua"))()

-- must be last
assert(loadfile("glorp/init_stdlib_environment.lua"))()
