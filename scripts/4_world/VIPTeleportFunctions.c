// VIP Teleport Helper Functions
class VIPTeleportFunctions
{
    // Static variables to store menu data for client
    static ref array<ref VIPTeleportLocation> m_PendingMenuLocations;
    static string m_PendingMenuTitle;
    static bool m_HasPendingMenu = false;

    // Store menu data to be processed by MissionGameplay
    static void ShowMenu(array<ref VIPTeleportLocation> locations, string menuTitle)
    {
        m_PendingMenuLocations = locations;
        m_PendingMenuTitle = menuTitle;
        m_HasPendingMenu = true;
    }

    // Check if there's a pending menu request
    static bool HasPendingMenu()
    {
        return m_HasPendingMenu;
    }

    // Get and clear pending menu data
    static void GetPendingMenuData(out array<ref VIPTeleportLocation> locations, out string menuTitle)
    {
        locations = m_PendingMenuLocations;
        menuTitle = m_PendingMenuTitle;
        m_HasPendingMenu = false;
    }

    // Show teleport result message to player
    static void ShowResult(bool success, string message)
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (player)
        {
            // Make sure camera control is restored
            GetGame().GetInput().ChangeGameFocus(-1);
            GetGame().GetUIManager().ShowUICursor(false);

            if (success)
            {
                player.MessageStatus(message);
            }
            else
            {
                player.MessageImportant(message);
            }

            Print("[VIPTeleport] ShowResult - Camera control restored after teleport");
        }
    }
}
