
-- We do our ownership semantics in Lua, since luabind has problems.

local function adoption(t, class, func)
  local creator = t[class]
  t[class] = function (...)
    local rv = creator(...)
    
    local rvt = rv[func]
    rv[func] = function (...)
      local created = rvt(...)
      
      created[rv] = true
      rv[created] = true
    
      return created
    end
    
    return rv
  end
end

adoption(b2, "World", "CreateBody")
adoption(b2, "Body", "CreateFixture")

