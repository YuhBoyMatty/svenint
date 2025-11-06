function OnFirstClientdataReceived(receive_time)
    printl( "OnFirstClientdataReceived(" .. receive_time .. ")" );
    
    -- if ( SpeedrunStart ~= nil ) then SpeedrunStart() end
    
    -- ClientCmd( "record autorecord_" .. CVar.GetString( "name" ) .. "_" .. MapName );

    -- ClientCmd( "use weapon_gauss" );
    -- ClientCmd( "use weapon_medkit" );
    -- ClientCmd( "use weapon_medkit; wait; kill" );
end

function OnBeginLoading()
    printl( "OnBeginLoading()" );
end

function OnEndLoading()
    printl( "OnEndLoading()" );
    -- ClientCmd( "stop; record autorecord_" .. CVar.GetString( "name" ) .. "_" .. MapName );
end

function OnDisconnect()
    printl( "OnDisconnect()" );
end

function OnRestart()
    printl( "OnRestart()" );

    -- if ( IncludeMapScript( MapName ) ) then
        -- printl( "Executed map script \"" .. MapName .. "\"" );
    -- end
end

-- local last_qees_position;
-- local qees_player_index = 1;

-- function OnPlayerUnstuck( pPlayerEdict )
--     printl( "Player \"" .. tostring( pPlayerEdict.vars.netname ) .. "\" unstucked at " .. tostring( pPlayerEdict.vars.origin ) );

--     if ( pPlayerEdict.vars.netname == "Sw1ft" ) then
--         pPlayerEdict.vars.origin = Vector( 1440.428833, -981.990295, -979.512817 );
--         pPlayerEdict.vars.origin.z = last_qees_position.z + 4.65;
--     end
-- end

-- function OnGameFrame(state, frametime, postruncmd)
--     if ( not postruncmd ) then
--         local player = GetPEntityFromEntityIndex( qees_player_index );

--         if ( player ~= nil ) then
--             last_qees_position = player.vars.origin;
--         end
--     end
-- end

function OnServerSignal( value )
    printl( "OnServerSignal( " .. value .. " )" );
end

function OnClientSignal( pPlayerEdict, value )
    printl( "OnClientSignal( " .. pPlayerEdict.vars.netname .. ", " .. value .. " )" );
end

function OnClientPutInServer( pPlayerEdict )
    printl( "OnClientPutInServer( " .. pPlayerEdict.vars.netname .. " )" );

    -- SendCommandToClient( pPlayerEdict, "sc_st_legit_mode" );
end

function OnPlayerSpawn( pSpawnSpotEdict, pPlayerEdict )
    printl( "OnPlayerSpawn()" );
    
    printl( "Player \"" .. tostring( pPlayerEdict.vars.netname ) .. "\" spawned at " .. tostring( pPlayerEdict.vars.origin ) .. ", viewangles: " .. tostring(pPlayerEdict.vars.angles) .. "" );
end

function IncludeMapScript( mapname )
    local playername = CVar.GetString( "name" );

	--[[
    if ( mapname == "hl_c01_a2" ) then
        IncludeScript( "c01_a2" );
        return true;
    elseif ( mapname == "hl_c02_a2" ) then
        if ( playername == "Sw1ft" ) then
            -- IncludeScript( "hl_c02_a2_sw1ft" );
        end

        -- IncludeScript( "hl_c02_a2" );

        return true;
    elseif ( mapname == "hl_c03" ) then
        -- IncludeScript( "c03" );
        return true;
    elseif ( mapname == "hl_c05_a3" ) then
        -- IncludeScript( "c05_a3" );
        return true;
    elseif ( mapname == "hl_c07_a2" ) then
        if ( playername == "Sw1ft" ) then
            IncludeScript( "c07_a2" );
        elseif ( playername == "QEES51" ) then
            IncludeScript( "c07_a2_qees" );
        elseif ( playername == "iMicro" ) then
            IncludeScript( "c07_a2_micro" );
        end

        return true;
    elseif ( mapname == "hl_c08_a2" ) then
        IncludeScript( "c08_a2_sw1ft" );
        -- IncludeScript( "c08_a2_newV2" );
        return true;
    elseif ( mapname == "hl_c10" ) then
        -- IncludeScript( "c10_new" );
        return true;
    elseif ( mapname == "hl_c13_a1" ) then
        IncludeScript( "c13_a1_new" );
        return true;
    elseif ( mapname == "hl_c13_a4" ) then
        IncludeScript( "c13_a4_p1" );
        return true;
    elseif ( mapname == "hl_c15" ) then
        IncludeScript( "c15" );
        return true;
    end
	--]]

    return false;
end

-- if ( IncludeMapScript( MapName ) ) then
    -- printl( "Executed map script \"" .. MapName .. "\"" );
-- end

printl( "Executed \"main.lua\"" );