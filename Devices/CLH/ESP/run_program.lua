-- Start your normal program routines here 

print("running user code")

uart.setup( 0, 9600, 8, 0, 1, 0 )

con_std = nil

  -- a simple telnet server
    s=net.createServer(net.TCP,300) 
    s:listen(2323,function(c)
        con_std = c
        c:on("disconnection",function(c) con_std = nil end)
        c:on("receive",function(conn,payload) uart.write(0, "__|", payload) end) 
    end)

function CM(data)
    if con_std then
        con_std:send(data)
    end
end
