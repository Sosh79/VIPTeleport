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
        }
    }

    void OnRPCMenuRequest(PlayerIdentity sender)
    {
        if (!sender)
            return;

        // Check if player is VIP
        string steamId = sender.GetPlainId();
        if (!VIPTeleportConfig.IsPlayerAllowed(steamId))
        {
            Print("[VIPTeleport] Non-VIP player attempted to open menu: " + steamId);
            return;
        }

        // Send menu data to client
        array<ref VIPTeleportLocation> locations = VIPTeleportConfig.GetLocations();
        string menuTitle = VIPTeleportConfig.GetMenuTitle();

        Print("[VIPTeleport] Preparing to send menu data:");
        Print("[VIPTeleport] - Locations count: " + locations.Count());
        Print("[VIPTeleport] - Menu title: " + menuTitle);

        ScriptRPC rpc = new ScriptRPC();

        // Send count first
        int locationCount = locations.Count();
        rpc.Write(locationCount);

        // Send menu info
        rpc.Write(menuTitle);

        // Send each location separately
        for (int i = 0; i < locationCount; i++)
        {
            VIPTeleportLocation loc = locations.Get(i);
            rpc.Write(loc.Name);
            rpc.Write(loc.Position);
            rpc.Write(loc.Description);
        }

        rpc.Send(this, RPC_VIP_TELEPORT_OPEN_MENU, true, sender);

        Print("[VIPTeleport] Sent menu to VIP player: " + sender.GetName());
    }

    void OnRPCMenuReceive(ParamsReadContext ctx)
    {
        Print("[VIPTeleport] Client receiving menu data...");

        int locationCount;
        string menuTitle;

        // Read count first
        if (!ctx.Read(locationCount))
        {
            Print("[VIPTeleport] ERROR: Failed to read locationCount from RPC!");
            return;
        }

        Print("[VIPTeleport] Expecting " + locationCount + " locations");

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

            VIPTeleportLocation loc = new VIPTeleportLocation(name, pos, desc);
            locations.Insert(loc);
            Print("[VIPTeleport] Received location: " + name);
        }

        Print("[VIPTeleport] Client received menu data:");
        Print("[VIPTeleport] - Locations count: " + locations.Count());
        Print("[VIPTeleport] - Menu title: " + menuTitle);

        // Show menu using helper class
        VIPTeleportFunctions.ShowMenu(locations, menuTitle);
    }

    void OnRPCTeleportRequest(PlayerIdentity sender, ParamsReadContext ctx)
    {
        if (!sender)
            return;

        int locationIndex;
        if (!ctx.Read(locationIndex))
            return;

        // Verify player is VIP
        string steamId = sender.GetPlainId();
        if (!VIPTeleportConfig.IsPlayerAllowed(steamId))
        {
            Print("[VIPTeleport] Unauthorized teleport attempt from: " + steamId);
            SendTeleportResponse(sender, false, "You are not authorized to use VIP Teleport");
            return;
        }

        // Get location
        array<ref VIPTeleportLocation> locations = VIPTeleportConfig.GetLocations();
        if (locationIndex < 0 || locationIndex >= locations.Count())
        {
            Print("[VIPTeleport] Invalid location index: " + locationIndex);
            SendTeleportResponse(sender, false, "Invalid teleport location");
            return;
        }

        VIPTeleportLocation location = locations.Get(locationIndex);

        // Check if player is in vehicle
        if (IsInVehicle())
        {
            SendTeleportResponse(sender, false, "Cannot teleport while in a vehicle");
            return;
        }

        // Teleport player
        vector targetPos = location.Position;

        // If Y coordinate is 0, get terrain height
        if (targetPos[1] == 0)
        {
            targetPos[1] = GetGame().SurfaceRoadY(targetPos[0], targetPos[2]);
        }

        SetPosition(targetPos);

        Print("[VIPTeleport] Player " + sender.GetName() + " teleported to " + location.Name);
        SendTeleportResponse(sender, true, "Teleported to " + location.Name);
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