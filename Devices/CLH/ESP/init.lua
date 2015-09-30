-- Copyright (c) 2015 Sebastian Hodapp
-- https://github.com/sebastianhodapp/ESPbootloader

if pcall(function () dofile("config.lc") end) then
    wifi.setmode(wifi.STATION)
    wifi.sta.config(ssid,password)
    wifi.sta.connect()

    ssid=nil
    password=nil

    local tries = 10

    tmr.alarm(1, 1000, 1, function() 
        if wifi.sta.getip() == nil then
            print("IP unavailable, waiting.")
            tries = tries - 1
            if (tries == 0) then 
                tmr.stop(1);
                dofile("run_config.lc")
            end    
        else 
            tmr.stop(1)
            print("Connected, IP is "..wifi.sta.getip())
            dofile("run_program.lua")
        end 
    end)
    
else
    print("Enter configuration mode")
    dofile("run_config.lc")
end
          

