local engine1 = Engine.new("Example Engine in Lua", Engine.getFiringOrderFromString("1-3-4-2"), vecFloat({180.0,180.0,180.0,180.0}), 5.4)
print(engine1.firingIntervalFactors)

function tick()
    return 1;
end