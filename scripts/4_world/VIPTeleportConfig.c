// VIP Teleport location definition
class VIPTeleportLocation
{
    string Name;
    vector Position;
    string Description;

    void VIPTeleportLocation(string name, vector pos, string desc)
    {
        Name = name;
        Position = pos;
        Description = desc;
    }
}

// VIP Teleport configuration manager
class VIPTeleportConfig
{
    static const string MOD_AUTHOR = "SOSH79";
    static const string CONFIG_FOLDER = "$profile:" + MOD_AUTHOR + "/VIPTeleport/";
    static const string CONFIG_NAME = "VIPTeleportLocations.json";
    static string CONFIG_PATH = CONFIG_FOLDER + CONFIG_NAME;
    static const string CURRENT_VERSION = "1.0.0";

    static string m_ConfigVersion = "";
    static string m_MenuTitle = "VIP Teleport Menu";
    static ref array<string> m_AllowedSteamIDs = new array<string>;
    static ref array<ref VIPTeleportLocation> m_TeleportLocations = new array<ref VIPTeleportLocation>;

    static void LoadConfig()
    {
        Print("[VIPTeleport] Loading configuration...");

        // Create directory structure if it doesn't exist
        if (!FileExist("$profile:" + MOD_AUTHOR))
        {
            MakeDirectory("$profile:" + MOD_AUTHOR);
            Print("[VIPTeleport] Created directory: " + MOD_AUTHOR);
        }

        if (!FileExist(CONFIG_FOLDER))
        {
            MakeDirectory(CONFIG_FOLDER);
            Print("[VIPTeleport] Created directory: VIPTeleport");
        }

        // Check if config file exists
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
                m_MenuTitle = configData.MenuTitle;
                m_AllowedSteamIDs.Clear();

                foreach (string steamId : configData.AllowedSteamIDs)
                {
                    m_AllowedSteamIDs.Insert(steamId);
                }

                m_TeleportLocations.Clear();
                foreach (VIPTeleportLocationData locData : configData.TeleportLocations)
                {
                    vector pos = Vector(locData.Position[0], locData.Position[1], locData.Position[2]);
                    VIPTeleportLocation loc = new VIPTeleportLocation(locData.Name, pos, locData.Description);
                    m_TeleportLocations.Insert(loc);
                }

                Print("[VIPTeleport] Configuration loaded successfully with " + m_TeleportLocations.Count() + " locations (Version: " + m_ConfigVersion + ")");
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
        defaultConfig.MenuTitle = "VIP Teleport Menu";
        defaultConfig.AllowedSteamIDs = new array<string>;
        defaultConfig.AllowedSteamIDs.Insert("76561198000000000");
        defaultConfig.AllowedSteamIDs.Insert("76561198111111111");

        defaultConfig.TeleportLocations = new array<ref VIPTeleportLocationData>;

        VIPTeleportLocationData loc1 = new VIPTeleportLocationData();
        loc1.Name = "Balota Airfield";
        loc1.Position = {4525, 0, 2443};
        loc1.Description = "Military airfield in the south-west";
        defaultConfig.TeleportLocations.Insert(loc1);

        VIPTeleportLocationData loc2 = new VIPTeleportLocationData();
        loc2.Name = "NWAF - Northwest Airfield";
        loc2.Position = {4814, 0, 10316};
        loc2.Description = "Large military airfield in the north";
        defaultConfig.TeleportLocations.Insert(loc2);

        JsonFileLoader<VIPTeleportConfigData>.JsonSaveFile(CONFIG_PATH, defaultConfig);

        m_ConfigVersion = defaultConfig.CONFIG_VERSION;
        m_MenuTitle = defaultConfig.MenuTitle;
        m_AllowedSteamIDs = defaultConfig.AllowedSteamIDs;

        m_TeleportLocations.Clear();
        foreach (VIPTeleportLocationData locData : defaultConfig.TeleportLocations)
        {
            vector pos = Vector(locData.Position[0], locData.Position[1], locData.Position[2]);
            VIPTeleportLocation loc = new VIPTeleportLocation(locData.Name, pos, locData.Description);
            m_TeleportLocations.Insert(loc);
        }

        Print("[VIPTeleport] Default configuration created");
    }

    static bool IsPlayerAllowed(string steamId)
    {
        foreach (string allowedId : m_AllowedSteamIDs)
        {
            if (allowedId == steamId)
            {
                return true;
            }
        }
        return false;
    }

    static array<ref VIPTeleportLocation> GetLocations()
    {
        return m_TeleportLocations;
    }

    static string GetMenuTitle()
    {
        return m_MenuTitle;
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

class VIPTeleportConfigData
{
    string CONFIG_VERSION;
    string MenuTitle;
    ref array<string> AllowedSteamIDs;
    ref array<ref VIPTeleportLocationData> TeleportLocations;

    void VIPTeleportConfigData()
    {
        AllowedSteamIDs = new array<string>;
        TeleportLocations = new array<ref VIPTeleportLocationData>;
    }
}
