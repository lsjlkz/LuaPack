# LuaPack
This is a lua library, the purpose is to pack lua’s table into a byte stream, which is convenient for data transmission.




test：

    local p = require("lua.pack")
    
    local o = p.new()
    
    local a = {}
    
    a[3] = 4
    
    a[4] = "3333333"
    
    a[5] = {}
    
    p.pack(o, a)
    
    local b = p.unpack(o)
    
    print(b[4])
