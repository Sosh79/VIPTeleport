// VIP Teleport location definition
class VIPTeleportLocation
{
    string Name;
    vector Position;
    string Description;
    bool AllowVehicleTeleport; // Inherited from parent menu
    float Distance = 0; // Distance from player in meters

    void VIPTeleportLocation(string name, vector pos, string desc, bool allowVehicle = true, float distance = 0)
    {
        Name = name;
        Position = pos;
        Description = desc;
        AllowVehicleTeleport = allowVehicle;
        Distance = distance;
    }
}

// Restricted "Teleport FROM" zone definition
class VIPTeleportRestrictedZone
{
    string Name;
    vector Center;
    float Radius;
    string Message;

    void VIPTeleportRestrictedZone(string name, vector center, float radius, string message)
    {
        Name = name;
        Center = center;
        Radius = radius;
        Message = message;
    }
}

// Single menu configuration
class VIPTeleportMenuConfig
{
    string MenuTitle;
    bool IsPublic;             // true = available to all players, false = only for specific Steam IDs
    bool Enabled;              // true = menu is active, false = menu is disabled
    bool AllowVehicleTeleport; // true = allow teleport with vehicle, false = player only
    ref array<string> AllowedSteamIDs;
    ref array<ref VIPTeleportLocation> TeleportLocations;

    void VIPTeleportMenuConfig()
    {
        IsPublic = false;
        Enabled = true;
        AllowVehicleTeleport = true;
        AllowedSteamIDs = new array<string>;
        TeleportLocations = new array<ref VIPTeleportLocation>;
    }
}

// VIP Teleport configuration manager
class VIPTeleportConfig
{
    static const string MOD_AUTHOR = "SOSH79";
    static const string CONFIG_FOLDER = "$profile:" + MOD_AUTHOR + "/VIPTeleport/";
    static const string CONFIG_NAME = "VIPTeleportLocations.json";
    static const string ADMIN_CONFIG_NAME = "VIPTeleportAdmins.json";
    static const string RESTRICTED_CONFIG_NAME = "VIPTeleportRestrictedZones.json";
    static string CONFIG_PATH = CONFIG_FOLDER + CONFIG_NAME;
    static string ADMIN_CONFIG_PATH = CONFIG_FOLDER + ADMIN_CONFIG_NAME;
    static string RESTRICTED_CONFIG_PATH = CONFIG_FOLDER + RESTRICTED_CONFIG_NAME;
    static const string CURRENT_VERSION = "2.0.5";

    static string m_ConfigVersion = "";
    static ref array<ref VIPTeleportMenuConfig> m_Menus = new array<ref VIPTeleportMenuConfig>;
    static ref array<string> m_AdminSteamIDs = new array<string>;
    static ref array<ref VIPTeleportRestrictedZone> m_RestrictedZones = new array<ref VIPTeleportRestrictedZone>;
    
    // Cooldown settings (loaded from JSON)
    static int m_CooldownSeconds = 30;              // Default: 30 seconds
    static int m_MaxTeleportsPerHour = 20;          // Default: 20 per hour
    static bool m_EnableCooldown = true;            // Default: enabled

    static void LoadConfig()
    {
        Print("[VIPTeleport] Loading configuration...");
// Initialize logging on server and load configs
#ifdef SERVER
        VIPTeleportLogger.InitLogFile();
        VIPTeleportLogger.Log("[Init] VIPTeleport LoadConfig called");
#endif
        LoadAdminConfig();
        LoadMenuConfig();
        LoadRestrictedZones();
    }

    static void LoadAdminConfig()
    {
        Print("[VIPTeleport] Loading admin configuration...");

        // Create directory if needed
        if (!FileExist("$profile:" + MOD_AUTHOR))
        {
            MakeDirectory("$profile:" + MOD_AUTHOR);
        }

        if (!FileExist(CONFIG_FOLDER))
        {
            MakeDirectory(CONFIG_FOLDER);
        }

        // Check if admin config exists
        if (FileExist(ADMIN_CONFIG_PATH))
        {
            VIPTeleportAdminConfigData adminData = new VIPTeleportAdminConfigData();
            JsonFileLoader<VIPTeleportAdminConfigData>.JsonLoadFile(ADMIN_CONFIG_PATH, adminData);

            if (adminData && adminData.AdminSteamIDs)
            {
                m_AdminSteamIDs.Clear();
                foreach (string steamId : adminData.AdminSteamIDs)
                {
                    m_AdminSteamIDs.Insert(steamId);
                }
                Print("[VIPTeleport] Loaded " + m_AdminSteamIDs.Count() + " admin Steam IDs");
            }
        }
        else
        {
            Print("[VIPTeleport] Admin config not found, creating default");
            CreateDefaultAdminConfig();
        }
    }

    static void CreateDefaultAdminConfig()
    {
        VIPTeleportAdminConfigData adminData = new VIPTeleportAdminConfigData();
        adminData.AdminSteamIDs = new array<string>;
        adminData.AdminSteamIDs.Insert("76561198000000000"); // Replace with your admin Steam ID

        JsonFileLoader<VIPTeleportAdminConfigData>.JsonSaveFile(ADMIN_CONFIG_PATH, adminData);

        m_AdminSteamIDs.Clear();
        m_AdminSteamIDs.Insert("76561198000000000");

        Print("[VIPTeleport] Created default admin config");
    }

    static bool IsAdmin(string steamId)
    {
        foreach (string adminId : m_AdminSteamIDs)
        {
            if (adminId == steamId)
            {
                return true;
            }
        }
        return false;
    }

    static void LoadMenuConfig()
    {
        Print("[VIPTeleport] Loading configuration...");

        if (FileExist(CONFIG_PATH))
        {
            
            // Load existing config
            VIPTeleportConfigData configData = new VIPTeleportConfigData();
            JsonFileLoader<VIPTeleportConfigData>.JsonLoadFile(CONFIG_PATH, configData);

            if (configData)
            {
                // Check version
                if (configData.CONFIG_VERSION != CURRENT_VERSION)
                {
                    Print("[VIPTeleport] Config version mismatch! Current: " + configData.CONFIG_VERSION + ", Expected: " + CURRENT_VERSION);
                    Print("[VIPTeleport] Creating backup and regenerating config...");

                    // Backup old config
                    JsonFileLoader<VIPTeleportConfigData>.JsonSaveFile(CONFIG_PATH + "_old", configData);

                    // Create new config with current version
                    CreateDefaultConfig();
                    return;
                }

                // Version matches, load data
                m_ConfigVersion = configData.CONFIG_VERSION;
                m_Menus.Clear();
                
                // Load cooldown settings (convert int to bool if needed)
                m_CooldownSeconds = configData.CooldownSeconds;
                m_MaxTeleportsPerHour = configData.MaxTeleportsPerHour;
                m_EnableCooldown = (configData.EnableCooldown != 0);

                foreach (VIPTeleportMenuData menuData : configData.Menus)
                {
                    VIPTeleportMenuConfig menu = new VIPTeleportMenuConfig();
                    menu.MenuTitle = menuData.MenuTitle;
                    menu.IsPublic = menuData.IsPublic;
                    menu.Enabled = menuData.Enabled;
                    menu.AllowVehicleTeleport = menuData.AllowVehicleTeleport;

                    foreach (string steamId : menuData.AllowedSteamIDs)
                    {
                        menu.AllowedSteamIDs.Insert(steamId);
                    }

                    foreach (VIPTeleportLocationData locData : menuData.TeleportLocations)
                    {
                        vector pos = Vector(locData.Position[0], locData.Position[1], locData.Position[2]);
                        VIPTeleportLocation loc = new VIPTeleportLocation(locData.Name, pos, locData.Description, menuData.AllowVehicleTeleport);
                        menu.TeleportLocations.Insert(loc);
                    }

                    m_Menus.Insert(menu);
                }

                Print("[VIPTeleport] Configuration loaded successfully with " + m_Menus.Count() + " menus (Version: " + m_ConfigVersion + ")");
                return;
            }
        }

        // Config doesn't exist, create default
        Print("[VIPTeleport] Config file not found, creating default configuration");
        CreateDefaultConfig();
    }

    static void CreateDefaultConfig()
    {
        VIPTeleportConfigData defaultConfig = new VIPTeleportConfigData();
        defaultConfig.CONFIG_VERSION = CURRENT_VERSION;
        
        // Set default cooldown settings
        defaultConfig.CooldownSeconds = 45;
        defaultConfig.MaxTeleportsPerHour = 15;
        defaultConfig.EnableCooldown = 1;
        
        defaultConfig.Menus = new array<ref VIPTeleportMenuData>;

        // Menu 0: Public Menu (Available to ALL players)
        VIPTeleportMenuData publicMenu = new VIPTeleportMenuData();
        publicMenu.MenuTitle = "Public Teleport";
        publicMenu.IsPublic = true; // Available to everyone
        publicMenu.Enabled = false;  // Enabled by default (set to false to disable)        publicMenu.AllowVehicleTeleport = false;  // Public menu: no vehicle teleport        publicMenu.AllowedSteamIDs = new array<string>;
        publicMenu.TeleportLocations = new array<ref VIPTeleportLocationData>;

        VIPTeleportLocationData pubLoc1 = new VIPTeleportLocationData();
        pubLoc1.Name = "Berezino";
        pubLoc1.Position = {12743, 0, 9544};
        pubLoc1.Description = "City on the north-east coast";
        publicMenu.TeleportLocations.Insert(pubLoc1);

        VIPTeleportLocationData pubLoc2 = new VIPTeleportLocationData();
        pubLoc2.Name = "Cherno";
        pubLoc2.Position = {6688, 0, 2588};
        pubLoc2.Description = "Large city on the south coast";
        publicMenu.TeleportLocations.Insert(pubLoc2);

        defaultConfig.Menus.Insert(publicMenu);

        // Menu 1: Admin Menu
        VIPTeleportMenuData adminMenu = new VIPTeleportMenuData();
        adminMenu.MenuTitle = "Admin Teleport";
        adminMenu.IsPublic = false;
        adminMenu.Enabled = true;
        adminMenu.AllowVehicleTeleport = true; // Admins can teleport with vehicles
        adminMenu.AllowedSteamIDs = new array<string>;
        adminMenu.AllowedSteamIDs.Insert("76561198000000000"); // Replace with admin Steam ID
        adminMenu.TeleportLocations = new array<ref VIPTeleportLocationData>;

        VIPTeleportLocationData loc1 = new VIPTeleportLocationData();
        loc1.Name = "Balota Airfield";
        loc1.Position = {4525, 0, 2443};
        loc1.Description = "Military airfield in the south-west";
        adminMenu.TeleportLocations.Insert(loc1);

        VIPTeleportLocationData loc2 = new VIPTeleportLocationData();
        loc2.Name = "NWAF - Northwest Airfield";
        loc2.Position = {4814, 0, 10316};
        loc2.Description = "Large military airfield in the north";
        adminMenu.TeleportLocations.Insert(loc2);

        VIPTeleportLocationData loc3 = new VIPTeleportLocationData();
        loc3.Name = "Electro";
        loc3.Position = {10430, 0, 2140};
        loc3.Description = "City on the east coast";
        adminMenu.TeleportLocations.Insert(loc3);

        defaultConfig.Menus.Insert(adminMenu);

        // Menu 2: VIP Menu
        VIPTeleportMenuData vipMenu = new VIPTeleportMenuData();
        vipMenu.MenuTitle = "VIP Teleport";
        vipMenu.AllowedSteamIDs = new array<string>;
        vipMenu.AllowedSteamIDs.Insert("76561198111111111"); // Replace with VIP Steam ID
        vipMenu.TeleportLocations = new array<ref VIPTeleportLocationData>;

        VIPTeleportLocationData loc4 = new VIPTeleportLocationData();
        loc4.Name = "Berezino";
        loc4.Position = {12743, 0, 9544};
        loc4.Description = "City on the north-east coast";
        vipMenu.TeleportLocations.Insert(loc4);

        VIPTeleportLocationData loc5 = new VIPTeleportLocationData();
        loc5.Name = "Cherno";
        loc5.Position = {6688, 0, 2588};
        loc5.Description = "Large city on the south coast";
        vipMenu.TeleportLocations.Insert(loc5);

        defaultConfig.Menus.Insert(vipMenu);

        // Menu 3: Moderator Menu
        VIPTeleportMenuData modMenu = new VIPTeleportMenuData();
        modMenu.MenuTitle = "Moderator Teleport";
        modMenu.AllowedSteamIDs = new array<string>;
        modMenu.AllowedSteamIDs.Insert("76561198222222222"); // Replace with Moderator Steam ID
        modMenu.TeleportLocations = new array<ref VIPTeleportLocationData>;

        VIPTeleportLocationData loc6 = new VIPTeleportLocationData();
        loc6.Name = "Stary Sobor";
        loc6.Position = {6149, 0, 7750};
        loc6.Description = "Military tents in the center";
        modMenu.TeleportLocations.Insert(loc6);

        defaultConfig.Menus.Insert(modMenu);

        JsonFileLoader<VIPTeleportConfigData>.JsonSaveFile(CONFIG_PATH, defaultConfig);

        // Load into memory
        m_ConfigVersion = defaultConfig.CONFIG_VERSION;
        m_Menus.Clear();

        foreach (VIPTeleportMenuData menuData : defaultConfig.Menus)
        {
            VIPTeleportMenuConfig menu = new VIPTeleportMenuConfig();
            menu.MenuTitle = menuData.MenuTitle;
            menu.IsPublic = menuData.IsPublic;
            menu.Enabled = menuData.Enabled;
            menu.AllowVehicleTeleport = menuData.AllowVehicleTeleport;
            menu.AllowedSteamIDs = menuData.AllowedSteamIDs;

            foreach (VIPTeleportLocationData locData : menuData.TeleportLocations)
            {
                vector pos = Vector(locData.Position[0], locData.Position[1], locData.Position[2]);
                VIPTeleportLocation loc = new VIPTeleportLocation(locData.Name, pos, locData.Description, menuData.AllowVehicleTeleport);
                menu.TeleportLocations.Insert(loc);
            }

            m_Menus.Insert(menu);
        }

        Print("[VIPTeleport] Default configuration created with " + m_Menus.Count() + " menus");
    }

    // Get menu for specific player by Steam ID - merges all menus player has access to
    static VIPTeleportMenuConfig GetMenuForPlayer(string steamId)
    {
        array<ref VIPTeleportMenuConfig> playerMenus = new array<ref VIPTeleportMenuConfig>;

        // Find all menus this player has access to
        foreach (VIPTeleportMenuConfig menu : m_Menus)
        {
            // Skip disabled menus
            if (!menu.Enabled)
                continue;

            // Check if menu is public (available to everyone)
            if (menu.IsPublic)
            {
                playerMenus.Insert(menu);
                continue;
            }

            // Check if player has specific access to this menu
            foreach (string allowedId : menu.AllowedSteamIDs)
            {
                if (allowedId == steamId)
                {
                    playerMenus.Insert(menu);
                    break;
                }
            }
        }

        // If player has no access, return null
        if (playerMenus.Count() == 0)
        {
            return null;
        }

        // If player has access to only one menu, return it
        if (playerMenus.Count() == 1)
        {
            return playerMenus.Get(0);
        }

        // If player has access to multiple menus, merge them
        VIPTeleportMenuConfig mergedMenu = new VIPTeleportMenuConfig();

        // Build merged title
        string mergedTitle = "";

        for (int i = 0; i < playerMenus.Count(); i++)
        {
            if (i > 0)
                mergedTitle += " + ";
            mergedTitle += playerMenus.Get(i).MenuTitle;
        }
        mergedMenu.MenuTitle = mergedTitle;

        // Merge all locations from all menus - each location keeps its own AllowVehicleTeleport setting
        mergedMenu.TeleportLocations = new array<ref VIPTeleportLocation>;
        foreach (VIPTeleportMenuConfig playerMenu : playerMenus)
        {
            foreach (VIPTeleportLocation loc : playerMenu.TeleportLocations)
            {
                // Location already has AllowVehicleTeleport from its parent menu
                mergedMenu.TeleportLocations.Insert(loc);
            }
        }

        mergedMenu.AllowedSteamIDs = new array<string>;
        mergedMenu.AllowedSteamIDs.Insert(steamId);

        Print("[VIPTeleport] Merged " + playerMenus.Count() + " menus for player " + steamId + " with total " + mergedMenu.TeleportLocations.Count() + " locations");

        return mergedMenu;
    }

    // Check if player has access to any menu
    static bool IsPlayerAllowed(string steamId)
    {
        return GetMenuForPlayer(steamId) != null;
    }

    // Get locations for specific player (deprecated, use GetMenuForPlayer)
    static array<ref VIPTeleportLocation> GetLocations()
    {
        if (m_Menus.Count() > 0)
        {
            return m_Menus.Get(0).TeleportLocations;
        }
        return new array<ref VIPTeleportLocation>;
    }

    // Get menu title for specific player (deprecated, use GetMenuForPlayer)
    static string GetMenuTitle()
    {
        if (m_Menus.Count() > 0)
        {
            return m_Menus.Get(0).MenuTitle;
        }
        return "VIP Teleport";
    }

    static void LoadRestrictedZones()
    {
        Print("[VIPTeleport] Loading restricted zones...");

        if (FileExist(RESTRICTED_CONFIG_PATH))
        {
            VIPTeleportRestrictedConfigData restData = new VIPTeleportRestrictedConfigData();
            JsonFileLoader<VIPTeleportRestrictedConfigData>.JsonLoadFile(RESTRICTED_CONFIG_PATH, restData);

            if (restData && restData.RestrictedZones)
            {
                m_RestrictedZones.Clear();
                foreach (VIPTeleportRestrictedZoneData zoneData : restData.RestrictedZones)
                {
                    vector center = Vector(zoneData.Center[0], zoneData.Center[1], zoneData.Center[2]);
                    m_RestrictedZones.Insert(new VIPTeleportRestrictedZone(zoneData.Name, center, zoneData.Radius, zoneData.Message));
                }
                Print("[VIPTeleport] Loaded " + m_RestrictedZones.Count() + " restricted zones");
            }
        }
        else
        {
            Print("[VIPTeleport] Restricted zones config not found, creating default");
            CreateDefaultRestrictedZones();
        }
    }

    static void CreateDefaultRestrictedZones()
    {
        VIPTeleportRestrictedConfigData restData = new VIPTeleportRestrictedConfigData();
        restData.RestrictedZones = new array<ref VIPTeleportRestrictedZoneData>;

        VIPTeleportRestrictedZoneData zone1 = new VIPTeleportRestrictedZoneData();
        zone1.Name = "Prison Island";
        zone1.Center = {2633, 0, 1318}; // Approximate prison island center
        zone1.Radius = 300;
        zone1.Message = "You cannot teleport from Prison Island! Escape first.";
        restData.RestrictedZones.Insert(zone1);

        JsonFileLoader<VIPTeleportRestrictedConfigData>.JsonSaveFile(RESTRICTED_CONFIG_PATH, restData);

        // Load into memory
        m_RestrictedZones.Clear();
        vector center = Vector(zone1.Center[0], zone1.Center[1], zone1.Center[2]);
        m_RestrictedZones.Insert(new VIPTeleportRestrictedZone(zone1.Name, center, zone1.Radius, zone1.Message));

        Print("[VIPTeleport] Created default restricted zones config");
    }

    static void SaveConfig()
    {
        Print("[VIPTeleport] Saving configuration...");

        VIPTeleportConfigData configData = new VIPTeleportConfigData();
        configData.CONFIG_VERSION = CURRENT_VERSION;
        configData.CooldownSeconds = m_CooldownSeconds;
        configData.MaxTeleportsPerHour = m_MaxTeleportsPerHour;
        configData.EnableCooldown = 0;
        if (m_EnableCooldown) configData.EnableCooldown = 1;
        
        configData.Menus = new array<ref VIPTeleportMenuData>;

        foreach (VIPTeleportMenuConfig menu : m_Menus)
        {
            VIPTeleportMenuData menuData = new VIPTeleportMenuData();
            menuData.MenuTitle = menu.MenuTitle;
            menuData.IsPublic = menu.IsPublic;
            menuData.Enabled = menu.Enabled;
            menuData.AllowVehicleTeleport = menu.AllowVehicleTeleport;
            menuData.AllowedSteamIDs = menu.AllowedSteamIDs;
            menuData.TeleportLocations = new array<ref VIPTeleportLocationData>;

            foreach (VIPTeleportLocation loc : menu.TeleportLocations)
            {
                VIPTeleportLocationData locData = new VIPTeleportLocationData();
                locData.Name = loc.Name;
                locData.Description = loc.Description;
                locData.Position = new array<float>;
                locData.Position.Insert(loc.Position[0]);
                locData.Position.Insert(loc.Position[1]);
                locData.Position.Insert(loc.Position[2]);
                menuData.TeleportLocations.Insert(locData);
            }

            configData.Menus.Insert(menuData);
        }

        JsonFileLoader<VIPTeleportConfigData>.JsonSaveFile(CONFIG_PATH, configData);
        Print("[VIPTeleport] Configuration saved successfully");
    }
}

// Helper classes for JSON serialization
class VIPTeleportLocationData
{
    string Name;
    ref array<float> Position;
    string Description;

    void VIPTeleportLocationData()
    {
        Position = new array<float>;
    }
}

class VIPTeleportMenuData
{
    string MenuTitle;
    bool IsPublic;             // true = available to all players, false = only for specific Steam IDs
    bool Enabled;              // true = menu is active, false = menu is disabled
    bool AllowVehicleTeleport; // true = allow teleport with vehicle, false = player only
    ref array<string> AllowedSteamIDs;
    ref array<ref VIPTeleportLocationData> TeleportLocations;

    void VIPTeleportMenuData()
    {
        IsPublic = false;
        Enabled = true;
        AllowVehicleTeleport = true;
        AllowedSteamIDs = new array<string>;
        TeleportLocations = new array<ref VIPTeleportLocationData>;
    }
}

class VIPTeleportConfigData
{
    string CONFIG_VERSION;
    
    // Cooldown settings (will appear right after CONFIG_VERSION in JSON)
    int CooldownSeconds = 30;
    int MaxTeleportsPerHour = 20;
    int EnableCooldown = 1; // Use int for better JSON compatibility
    
    // Menus array (will appear after cooldown settings)
    ref array<ref VIPTeleportMenuData> Menus;

    void VIPTeleportConfigData()
    {
        Menus = new array<ref VIPTeleportMenuData>;
    }
}

// Restricted zones configuration data classes
class VIPTeleportRestrictedZoneData
{
    string Name;
    ref array<float> Center;
    float Radius;
    string Message;

    void VIPTeleportRestrictedZoneData()
    {
        Center = new array<float>;
    }
}

class VIPTeleportRestrictedConfigData
{
    ref array<ref VIPTeleportRestrictedZoneData> RestrictedZones;

    void VIPTeleportRestrictedConfigData()
    {
        RestrictedZones = new array<ref VIPTeleportRestrictedZoneData>;
    }
}

// Admin configuration data class
class VIPTeleportAdminConfigData
{
    ref array<string> AdminSteamIDs;

    void VIPTeleportAdminConfigData()
    {
        AdminSteamIDs = new array<string>;
    }
}
