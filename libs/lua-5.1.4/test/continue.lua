-- test the continue statement
-- vim:sw=4:sts=4

-- simple
sum = 0
for i = 1, 10 do
    if i == 5 then continue end
    sum = sum + i
end
assert(sum == 50)

-- multiple continues
sum = 0
for i = 1, 10 do
    if i == 2 then continue end
    if i == 5 then continue end
    if i == 10 then continue end
    sum = sum + i
end
assert(sum == 38)


-- continues and breaks mixed
sum = 0
for i = 1, 10 do
    if i == 1 then continue end
    if i == 5 then continue end
    if i == 8 then break end
    sum = sum + i
end
assert(sum == 22)


-- continue in a repeat statement
i = 0
sum = 0
repeat
    i = i + 1
    if i == 5 then continue end
    if i == 8 then continue end
    if i == 10 then continue end
    sum = sum + i
until i == 10
assert(sum == 55-5-8-10)


-- continue in a while statement
i = 0
sum = 0
while i < 10 do
    i = i + 1
    if i == 5 then continue end
    if i == 10 then continue end
    sum = sum + i
end
assert(sum == 55-5-10)


-- nested loops
sum = 0
for i = 1, 10 do
    if i == 2 then continue end
    if i == 7 then continue end
    for j = 1, 4 do
	if j == 2 then continue end
	if j == 3 then continue end
	sum = sum + i + j
    end
end
assert(sum == 132)

