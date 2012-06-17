-- Event system framework.

function CreateEvent(root, name)
  print(root, name)
  local eventtable = InsertItem(root, "Event." .. name, {})

  return function (...)
    for i = 1, #eventtable do
      assert(type(eventtable[i] == "function"))
      eventtable[i](...)
    end
  end
end
