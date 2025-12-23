class CfgPatches
{
    class VIPTeleport
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
    };
};

class CfgMods
{
    class VIPTeleport
    {
        dir = "VIPTeleport";
        picture = "";
        action = "";
        hideName = 1;
        hidePicture = 1;
        name = "VIPTeleport";
        credits = "";
        author = "VIPTeleport";
        version = "1.0.0";
        type = "mod";
        dependencies[] = {"Game", "World", "Mission"};
        inputs = "VIPTeleport/data/inputs.xml";

        class defs
        {
            class gameLibScriptModule
            {
                value = "";
                files[] = {"VIPTeleport/scripts/common"};
            };
            class gameScriptModule
            {
                value = "";
                files[] = {"VIPTeleport/scripts/3_game"};
            };
            class worldScriptModule
            {
                value = "";
                files[] = {"VIPTeleport/scripts/4_world"};
            };
            class missionScriptModule
            {
                value = "";
                files[] = {"VIPTeleport/scripts/5_mission"};
            };
        };
    };
};
