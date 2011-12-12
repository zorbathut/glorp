
local input, output, b2 = ...

assert(loadfile("glorp/util.lua"))()
assert(loadfile("glorp/stage_persistence.lua"))()

assert(loadfile("scaffold.lua"))()
assert(loadfile("terra.lua"))()
assert(loadfile("worldparams.lua"))()

local terra = terra_init(mappieces, entities)

terra.load(assert(io.snatch(input), lev))

terra.bake(tonumber(b2))

io.dump(output, terra.save())
