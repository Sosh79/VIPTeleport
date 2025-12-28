// VIP Teleport Cooldown System
// Security system to prevent spam and abuse

class VIPTeleportPlayerData
{
    int lastTeleportTick = 0;      // Changed to tick-based (milliseconds)
    int teleportCount = 0;
    int hourStartTick = 0;          // Changed to tick-based
    
    void VIPTeleportPlayerData()
    {
        lastTeleportTick = 0;
        teleportCount = 0;
        hourStartTick = GetGame().GetTickTime();
    }
    
    void ResetHourlyCount()
    {
        teleportCount = 0;
        hourStartTick = GetGame().GetTickTime();
    }
}

class VIPTeleportCooldownManager
{
    static ref map<string, ref VIPTeleportPlayerData> m_PlayerData = new map<string, ref VIPTeleportPlayerData>;
    
    // Note: Cooldown settings are now loaded from JSON config file
    // See VIPTeleportConfig.m_CooldownSeconds, VIPTeleportConfig.m_MaxTeleportsPerHour, VIPTeleportConfig.m_EnableCooldown
    
    // Check if player can teleport
    static bool CanTeleport(string steamId, out string reasonMessage)
    {
        // Check if cooldown is enabled in config
        if (!VIPTeleportConfig.m_EnableCooldown)
        {
            return true;
        }
        
        reasonMessage = "";
        int currentTick = GetGame().GetTickTime();
        
        // Create player data if not exists
        if (!m_PlayerData.Contains(steamId))
        {
            m_PlayerData.Insert(steamId, new VIPTeleportPlayerData());
            return true;
        }
        
        VIPTeleportPlayerData data = m_PlayerData.Get(steamId);
        
        // Reset hourly counter if one hour has passed
        int timeSinceHourStart = currentTick - data.hourStartTick;
        int hourDuration = 3600; // 1 hour = 3600 seconds
        if (timeSinceHourStart >= hourDuration)
        {
            data.ResetHourlyCount();
        }
        
        // Check time-based cooldown (in ticks, where 1 tick ≈ 1 second)
        int ticksSinceLastTeleport = currentTick - data.lastTeleportTick;
        
        // Use cooldown from config
        int cooldownTicks = VIPTeleportConfig.m_CooldownSeconds;
        if (ticksSinceLastTeleport < cooldownTicks)
        {
            int remainingTicks = cooldownTicks - ticksSinceLastTeleport;
            reasonMessage = "Please wait " + remainingTicks.ToString() + " seconds before teleporting again";
            return false;
        }
        
        // Check hourly limit from config
        int maxTeleportsPerHour = VIPTeleportConfig.m_MaxTeleportsPerHour;
        if (data.teleportCount >= maxTeleportsPerHour)
        {
            reasonMessage = "You have reached the hourly teleport limit (" + maxTeleportsPerHour.ToString() + " per hour)";
            return false;
        }
        
        return true;
    }
    
    // Record successful teleport
    static void RecordTeleport(string steamId, string playerName = "")
    {
        if (!m_PlayerData.Contains(steamId))
        {
            m_PlayerData.Insert(steamId, new VIPTeleportPlayerData());
        }
        
        VIPTeleportPlayerData data = m_PlayerData.Get(steamId);
        data.lastTeleportTick = GetGame().GetTickTime();
        data.teleportCount++;
        
#ifdef SERVER
        int maxTeleports = VIPTeleportConfig.m_MaxTeleportsPerHour;
        int remaining = maxTeleports - data.teleportCount;
        VIPTeleportLogger.Log("[Cooldown] Player '" + playerName + "' (" + steamId + ") used teleport - Count: " + data.teleportCount + "/" + maxTeleports + " (Remaining: " + remaining + ")");
#endif
    }
    
    // Admin function: bypass or reset cooldown
    static void ResetCooldown(string steamId)
    {
        if (m_PlayerData.Contains(steamId))
        {
            m_PlayerData.Remove(steamId);
#ifdef SERVER
            VIPTeleportLogger.Log("[Admin] Cooldown reset for player: " + steamId);
#endif
        }
    }
    
    // Get cooldown info for player (admin use)
    static string GetPlayerCooldownInfo(string steamId)
    {
        if (!m_PlayerData.Contains(steamId))
        {
            return "No cooldown data for this player";
        }
        
        VIPTeleportPlayerData data = m_PlayerData.Get(steamId);
        int currentTick = GetGame().GetTickTime();
        int ticksSinceLast = currentTick - data.lastTeleportTick;
        
        string info = "Cooldown Info:\n";
        info += "- Last teleport: " + ticksSinceLast.ToString() + " seconds ago\n";
        info += "- Teleports this hour: " + data.teleportCount.ToString() + "/" + VIPTeleportConfig.m_MaxTeleportsPerHour.ToString();
        
        return info;
    }
}
