
local params, rv = ...

ursa.token.rule{"FLAC", nil, function () return "flac" end}

rv.extension = ".prog"  -- have to use something or it'll conflict
