modded class MissionServer
{
    override void OnInit()
    {
        super.OnInit();

        // Load VIP Teleport configuration
        VIPTeleportConfig.LoadConfig();

        Print("[VIPTeleport] Server initialized");
    }
}
