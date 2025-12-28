modded class PlayerBase
{
    override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
    {
        super.OnRPC(sender, rpc_type, ctx);

        switch (rpc_type)
        {
        case RPC_VIP_TELEPORT_OPEN_MENU:
        {
            if (GetGame().IsServer())
            {
                OnRPCMenuRequest(sender);
            }
            else if (GetGame().IsClient())
            {
                OnRPCMenuReceive(ctx);
            }
            break;
        }
        case RPC_VIP_TELEPORT_REQUEST:
        {
            if (GetGame().IsServer())
            {
                OnRPCTeleportRequest(sender, ctx);
            }
            break;
        }
        case RPC_VIP_TELEPORT_RESPONSE:
        {
            if (GetGame().IsClient())
            {
                OnRPCTeleportResponse(ctx);
            }
            break;
        }
        case RPC_VIP_ADMIN_OPEN_MENU:
        {
            if (GetGame().IsServer())
            {
                OnRPCAdminMenuRequest(sender);
            }
            else if (GetGame().IsClient())
            {
                OnRPCAdminMenuReceive(ctx);
            }
            break;
        }
        case RPC_VIP_ADMIN_RELOAD_CONFIG:
        {
            if (GetGame().IsServer())
            {
                OnRPCAdminReloadRequest(sender);
            }
            break;
        }
        case RPC_VIP_ADMIN_RESPONSE:
        {
            if (GetGame().IsClient())
            {
                OnRPCAdminResponse(ctx);
            }
            break;
        }
        }
    }

    // CLIENT: Request admin menu
    void RequestAdminMenu()
    {
        if (GetGame().IsClient())
        {
            ScriptRPC rpc = new ScriptRPC();
            rpc.Send(this, RPC_VIP_ADMIN_OPEN_MENU, true, null);
            Print("[VIPTeleport] CLIENT: Requested admin menu");
        }
    }

    // CLIENT: Request config reload
    void RequestAdminReloadConfig()
    {
        if (GetGame().IsClient())
        {
            ScriptRPC rpc = new ScriptRPC();
            rpc.Send(this, RPC_VIP_ADMIN_RELOAD_CONFIG, true, null);
            Print("[VIPTeleport] CLIENT: Requested config reload");
        }
    }

    // SERVER: Handle admin menu request
    void OnRPCAdminMenuRequest(PlayerIdentity sender)
    {
        if (!sender)
            return;

        string steamId = sender.GetPlainId();
        if (!VIPTeleportConfig.IsAdmin(steamId))
        {
            Print("[VIPTeleport] Non-admin attempted to open admin menu: " + steamId);
            return;
        }

        // Send confirmation to open admin menu
        ScriptRPC rpc = new ScriptRPC();
        rpc.Write(true);
        rpc.Write("Admin menu opened");
        rpc.Send(this, RPC_VIP_ADMIN_OPEN_MENU, true, sender);

        Print("[VIPTeleport] Admin menu access granted to: " + sender.GetName());
    }

    // CLIENT: Receive admin menu open confirmation
    void OnRPCAdminMenuReceive(ParamsReadContext ctx)
    {
        bool success;
        string message;

        if (!ctx.Read(success))
            return;
        if (!ctx.Read(message))
            return;

        if (success)
        {
            Print("[VIPTeleport] CLIENT: Opening admin menu");
            VIPTeleportFunctions.RequestAdminMenuOpen();
        }
    }

    // SERVER: Handle config reload request
    void OnRPCAdminReloadRequest(PlayerIdentity sender)
    {
        if (!sender)
            return;

        string steamId = sender.GetPlainId();
        if (!VIPTeleportConfig.IsAdmin(steamId))
        {
            Print("[VIPTeleport] Non-admin attempted to reload config: " + steamId);
            SendAdminResponse(sender, false, "Unauthorized");
            return;
        }

        Print("[VIPTeleport] Admin reload config requested by: " + sender.GetName());
        
        // Reload configuration from JSON
        VIPTeleportConfig.LoadConfig();
        
        // Note: Client-side cache will be refreshed automatically when players
        // request menu data after config reload
        
        int menuCount = VIPTeleportConfig.m_Menus.Count();
        int adminCount = VIPTeleportConfig.m_AdminSteamIDs.Count();

        string successMsg = "Reloaded! Menus: " + menuCount + " | Admins: " + adminCount;
        SendAdminResponse(sender, true, successMsg);

#ifdef SERVER
        VIPTeleportLogger.Log("[Admin] Config reloaded by '" + sender.GetName() + "' - Menus: " + menuCount + ", Admins: " + adminCount);
#endif
    }

    // SERVER: Send admin response
    void SendAdminResponse(PlayerIdentity identity, bool success, string message)
    {
        if (!identity)
            return;

        ScriptRPC rpc = new ScriptRPC();
        rpc.Write(success);
        rpc.Write(message);
        rpc.Send(this, RPC_VIP_ADMIN_RESPONSE, true, identity);
    }

    // CLIENT: Receive admin response
    void OnRPCAdminResponse(ParamsReadContext ctx)
    {
        bool success;
        string message;

        if (!ctx.Read(success))
            return;
        if (!ctx.Read(message))
            return;

        Print("[VIPTeleport] CLIENT: Received admin response - " + message);
        VIPTeleportFunctions.ShowAdminReloadResult(success, message);
    }

    void OnRPCMenuRequest(PlayerIdentity sender)
    {
        if (!sender)
            return;

        // Check if player is VIP and get their menu
        string steamId = sender.GetPlainId();
        VIPTeleportMenuConfig playerMenu = VIPTeleportConfig.GetMenuForPlayer(steamId);

        if (!playerMenu)
        {
            Print("[VIPTeleport] Non-VIP player attempted to open menu: " + steamId);
            return;
        }

        // Send menu data to client
        array<ref VIPTeleportLocation> locations = playerMenu.TeleportLocations;
        string menuTitle = playerMenu.MenuTitle;

        // Sending menu data to client

        ScriptRPC rpc = new ScriptRPC();

        // Send count first
        int locationCount = locations.Count();
        rpc.Write(locationCount);

        // Send menu info
        rpc.Write(menuTitle);

        // Get player position for distance calculation
        vector playerPos = this.GetPosition(); 
        // Send each location separately with distance
        for (int i = 0; i < locationCount; i++)
        {
            VIPTeleportLocation loc = locations.Get(i);
            
            // Calculate distance from player to this location
            float distance = vector.Distance(playerPos, loc.Position);
            
            rpc.Write(loc.Name);
            rpc.Write(loc.Position);
            rpc.Write(loc.Description);
            rpc.Write(distance); // Send distance in meters
        }

        rpc.Send(this, RPC_VIP_TELEPORT_OPEN_MENU, true, sender);

        // Menu sent successfully
#ifdef SERVER
        VIPTeleportLogger.Log("[Menu] Player " + sender.GetName() + " opened VIPTeleport menu.");
#endif
    }

    void OnRPCMenuReceive(ParamsReadContext ctx)
    {
        // Receiving menu data from server

        int locationCount;
        string menuTitle;

        // Read count first
        if (!ctx.Read(locationCount))
        {
            Print("[VIPTeleport] ERROR: Failed to read locationCount from RPC!");
            return;
        }

        // Reading location data

        // Read menu info
        if (!ctx.Read(menuTitle))
        {
            Print("[VIPTeleport] ERROR: Failed to read menuTitle from RPC!");
            return;
        }

        // Read each location separately
        array<ref VIPTeleportLocation> locations = new array<ref VIPTeleportLocation>;

        for (int i = 0; i < locationCount; i++)
        {
            string name;
            vector pos;
            string desc;

            if (!ctx.Read(name))
            {
                Print("[VIPTeleport] ERROR: Failed to read location name at index " + i);
                return;
            }
            if (!ctx.Read(pos))
            {
                Print("[VIPTeleport] ERROR: Failed to read location position at index " + i);
                return;
            }
            if (!ctx.Read(desc))
            {
                Print("[VIPTeleport] ERROR: Failed to read location description at index " + i);
                return;
            }

            float distance;
            if (!ctx.Read(distance))
            {
                Print("[VIPTeleport] ERROR: Failed to read location distance at index " + i);
                return;
            }

            VIPTeleportLocation loc = new VIPTeleportLocation(name, pos, desc, true, distance);
            locations.Insert(loc);
            // Location received
        }

        // Menu data loaded successfully

        // Show menu using helper class
        VIPTeleportFunctions.ShowMenu(locations, menuTitle);
    }

    void OnRPCTeleportRequest(PlayerIdentity sender, ParamsReadContext ctx)
    {
        if (!sender)
            return;

        string steamId = sender.GetPlainId();
        
        Print("[VIPTeleport] === TELEPORT REQUEST START ===");
        Print("[VIPTeleport] Player: " + sender.GetName() + " (" + steamId + ")");
        
        // ADMIN BYPASS: Admins skip cooldown and restricted zone checks
        bool isAdmin = VIPTeleportConfig.IsAdmin(steamId);
        vector currentPos = this.GetPosition();
        
        // SECURITY: Check Restricted "Teleport FROM" Zones
        if (!isAdmin)
        {
            foreach (VIPTeleportRestrictedZone zone : VIPTeleportConfig.m_RestrictedZones)
            {
                if (vector.Distance(currentPos, zone.Center) <= zone.Radius)
                {
                    SendTeleportResponse(sender, false, zone.Message);
#ifdef SERVER
                    VIPTeleportLogger.Log("[Restricted] Blocked teleport for '" + sender.GetName() + "' (" + steamId + ") from " + zone.Name);
#endif
                    return;
                }
            }
        }

        // SECURITY: Check cooldown first to prevent spam/abuse (skip for admins)
        if (!isAdmin)
        {
            string cooldownMessage;
            if (!VIPTeleportCooldownManager.CanTeleport(steamId, cooldownMessage))
            {
                SendTeleportResponse(sender, false, cooldownMessage);
#ifdef SERVER
                VIPTeleportLogger.Log("[Cooldown] Blocked teleport for '" + sender.GetName() + "' (" + steamId + ") - " + cooldownMessage);
#endif
                return;
            }
        }

        // SECURITY: Read location name and position instead of index
        string locationName;
        vector requestedPosition;
        
        if (!ctx.Read(locationName))
        {
            Print("[VIPTeleport] ERROR: Failed to read location name from RPC");
            return;
        }
        
        if (!ctx.Read(requestedPosition))
        {
            Print("[VIPTeleport] ERROR: Failed to read requested position from RPC");
            return;
        }

        // Verify player is VIP and get their menu
        VIPTeleportMenuConfig playerMenu = VIPTeleportConfig.GetMenuForPlayer(steamId);

        if (!playerMenu)
        {
            Print("[VIPTeleport] Unauthorized teleport attempt from: " + steamId);
            SendTeleportResponse(sender, false, "You are not authorized to use VIP Teleport");
            return;
        }

        // SECURITY: Validate that the requested location exists in player's allowed menu
        VIPTeleportLocation validLocation;
        bool locationFound = false;
        
        array<ref VIPTeleportLocation> locations = playerMenu.TeleportLocations;
        foreach (VIPTeleportLocation loc : locations)
        {
            // Match by name and verify position is close (within 10m margin for floating point differences)
            if (loc.Name == locationName && vector.Distance(loc.Position, requestedPosition) < 10)
            {
                validLocation = loc;
                locationFound = true;
                break;
            }
        }
        
        if (!locationFound)
        {
            Print("[VIPTeleport] SECURITY: Invalid location requested by " + steamId + " - Location: " + locationName);
            SendTeleportResponse(sender, false, "Invalid teleport location");
#ifdef SERVER
            VIPTeleportLogger.Log("[SECURITY] Player '" + sender.GetName() + "' (" + steamId + ") attempted INVALID teleport to '" + locationName + "'");
#endif
            return;
        }

        // Use server-side location data, not client data (NEVER trust client!)
        VIPTeleportLocation location = validLocation;

        // Teleport player
        vector targetPos = location.Position;

        // If Y coordinate is 0, get terrain height
        if (targetPos[1] == 0)
        {
            targetPos[1] = GetGame().SurfaceRoadY(targetPos[0], targetPos[2]);
        }

        
        // Check if player is in vehicle
        Transport vehicle = null;
        HumanCommandVehicle vehCommand = GetCommand_Vehicle();
        if (vehCommand)
        {
            vehicle = vehCommand.GetTransport();
        }

        if (vehicle)
        {
            // Check if this specific location allows vehicle teleport
            if (location.AllowVehicleTeleport)
            {
                // Teleport vehicle with all passengers
                vehicle.SetPosition(targetPos);
                // Vehicle teleport successful
                SendTeleportResponse(sender, true, "Teleported to " + location.Name + " with vehicle");
#ifdef SERVER
                VIPTeleportLogger.Log("[Teleport] Player '" + sender.GetName() + "' (" + sender.GetPlainId() + ") teleported WITH VEHICLE from " + currentPos.ToString() + " to '" + location.Name + "' at " + targetPos.ToString());
#endif
                // Record successful teleport in cooldown system (skip for admins)
                if (!isAdmin)
                    VIPTeleportCooldownManager.RecordTeleport(steamId, sender.GetName());
            }
            else
            {
                // Vehicle teleport not allowed for this location
                SendTeleportResponse(sender, false, "Vehicle teleport is not allowed for this location");
                Print("[VIPTeleport] Player " + sender.GetName() + " tried to teleport with vehicle to " + location.Name + " but it's disabled for this location");
#ifdef SERVER
                VIPTeleportLogger.Log("[Teleport] DENIED - Player '" + sender.GetName() + "' (" + sender.GetPlainId() + ") tried to teleport with vehicle to '" + location.Name + "' but vehicle teleport is disabled");
#endif
            }
        }
        else
        {
            // Teleport player only
            SetPosition(targetPos);
            // Teleport successful
            SendTeleportResponse(sender, true, "Teleported to " + location.Name);
#ifdef SERVER
            VIPTeleportLogger.Log("[Teleport] Player '" + sender.GetName() + "' (" + sender.GetPlainId() + ") teleported from " + currentPos.ToString() + " to '" + location.Name + "' at " + targetPos.ToString());
#endif
            // Record successful teleport in cooldown system (skip for admins)
            if (!isAdmin)
                VIPTeleportCooldownManager.RecordTeleport(steamId, sender.GetName());
        }
    }

    void SendTeleportResponse(PlayerIdentity identity, bool success, string message)
    {
        if (!identity)
            return;

        ScriptRPC rpc = new ScriptRPC();
        rpc.Write(success);
        rpc.Write(message);
        rpc.Send(this, RPC_VIP_TELEPORT_RESPONSE, true, identity);
    }

    void OnRPCTeleportResponse(ParamsReadContext ctx)
    {
        bool success;
        string message;

        if (!ctx.Read(success))
            return;
        if (!ctx.Read(message))
            return;

        // Show result using helper class
        VIPTeleportFunctions.ShowResult(success, message);
    }
}