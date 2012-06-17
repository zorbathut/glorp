-- Event system framework.

function CreateEvent(root, name)
  local db = {}
  local locks = 0
  local buffered = {}
  
  local function Attach(context, functor, order)
    assert(context)
    assert(functor)
    assert(type(functor) == "function")
    if not context or not functor then return end
    if not order then order = 0 end
    
    if locks > 0 then table.insert(buffered, function () Attach(context, functor, order) end) return end
    
    table.insert(db, {c = context, f = functor, o = order})
    table.sort(db, function (a, b) return a.o < b.o end)  -- this is sort of slow
  end
  
  local function Detach(context, functor, order)
    assert(context)
    assert(functor)
    assert(type(functor) == "function")
    if not context or not functor then return end
    if locks > 0 then table.insert(buffered, function () Detach(context, functor, order) end) return end
    
    for k, v in ipairs(db) do
      if v.f == functor and v.c == context and (not order or v.o == order) then
        table.remove(db, k)
      end
    end
  end
  
  local function Flush()
    if #buffered > 0 then
      for k, v in ipairs(buffered) do
        v()
      end
      while #buffered > 0 do
        table.remove(buffered)
      end
    end
  end
  
  local function ClearContext(context)
    assert(context)
    if not context then return end
    if locks > 0 then table.insert(buffered, function () ClearContext(context) end) return end
    
    local new = {}
    for t = 1, #db do
      if db[t].c ~= context then
        table.insert(new, db[t])
      end
    end
    
    db = new
  end
  
  local function CreateContextHandle(_, context)
    return {Attach = function (_, functor, order) Attach(context, functor, order) end, Detach = function (_, functor, order) Detach(context, functor, order) end, CreateContextHandle = CreateContextHandle, ClearContext = function (_) ClearContext(context) end}
  end
  
  InsertItem(root, "Event." .. name, CreateContextHandle(nil, root))

  return function (...)
    locks = locks + 1
    for i = 1, #db do
      assert(type(db[i].f) == "function")
      db[i].f(...)
    end
    locks = locks - 1
    if locks == 0 then
      Flush()
    end
  end
end
