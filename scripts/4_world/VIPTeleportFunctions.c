// VIP Teleport Helper Functions
class VIPTeleportFunctions
{
    // Static variables to store menu data for client
    static ref array<ref VIPTeleportLocation> m_PendingMenuLocations;
    static string m_PendingMenuTitle;
    static bool m_HasPendingMenu = false;

    // Admin menu pending flag
    static bool m_HasPendingAdminMenu = false;

    // Custom dialog pending flags
    static bool m_HasPendingDialog = false;
    static string m_PendingDialogMessage;

    // Custom notification pending flags
    static bool m_HasPendingNotification = false;
    static string m_PendingNotifTitle;
    static string m_PendingNotifMessage;
    static string m_PendingNotifIcon;
    static float m_PendingNotifDuration;

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

    // Admin menu functions
    static void RequestAdminMenuOpen()
    {
        m_HasPendingAdminMenu = true;
    }

    static bool HasPendingAdminMenu()
    {
        return m_HasPendingAdminMenu;
    }

    static void ClearPendingAdminMenu()
    {
        m_HasPendingAdminMenu = false;
    }

    // Pending admin reload result
    static bool m_HasPendingAdminReloadResult = false;
    static bool m_PendingAdminReloadSuccess;
    static string m_PendingAdminReloadMessage;

    static void ShowAdminReloadResult(bool success, string message)
    {
        m_HasPendingAdminReloadResult = true;
        m_PendingAdminReloadSuccess = success;
        m_PendingAdminReloadMessage = message;
    }

    static bool HasPendingAdminReloadResult()
    {
        return m_HasPendingAdminReloadResult;
    }

    static void GetPendingAdminReloadResult(out bool success, out string message)
    {
        success = m_PendingAdminReloadSuccess;
        message = m_PendingAdminReloadMessage;
        m_HasPendingAdminReloadResult = false;
    }

    // Custom dialog helper functions
    static void ShowCustomDialog(string message)
    {
        m_PendingDialogMessage = message;
        m_HasPendingDialog = true;
    }

    static bool HasPendingDialog()
    {
        return m_HasPendingDialog;
    }

    static string GetPendingDialogMessage()
    {
        m_HasPendingDialog = false;
        return m_PendingDialogMessage;
    }

    // Custom notification helper functions
    static void AddCustomNotification(string title, string message, string icon, float duration = 5.0)
    {
        m_PendingNotifTitle = title;
        m_PendingNotifMessage = message;
        m_PendingNotifIcon = icon;
        m_PendingNotifDuration = duration;
        m_HasPendingNotification = true;
    }

    static bool HasPendingNotification()
    {
        return m_HasPendingNotification;
    }

    static void GetPendingNotification(out string title, out string message, out string icon, out float duration)
    {
        title = m_PendingNotifTitle;
        message = m_PendingNotifMessage;
        icon = m_PendingNotifIcon;
        duration = m_PendingNotifDuration;
        m_HasPendingNotification = false;
    }

    // Show teleport result message to player
    static void ShowResult(bool success, string message, int severity = 0)
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (player)
        {
            // Make sure camera control is restored
            GetGame().GetInput().ChangeGameFocus(-1);
            GetGame().GetUIManager().ShowUICursor(false);

            if (severity == 2)
            {
                // Severity 2 = Critical (Red Popup) - Request custom dialog (via mission layer pickup)
                ShowCustomDialog(message);
            }
            else if (severity == 1)
            {
                // Severity 1 = Warning - Use Custom System (via mission layer pickup)
                AddCustomNotification("VIP TELEPORT WARNING", message, "VIPTeleport/GUI/icons/notif_icon_warning.edds");
            }
            else
            {
                // Severity 0 = Success - Use Custom System (via mission layer pickup)
                if (success)
                    AddCustomNotification("VIP TELEPORT SUCCESS", message, "VIPTeleport/GUI/icons/notif_icon_success.edds");
                else
                    AddCustomNotification("VIP TELEPORT INFO", message, "VIPTeleport/GUI/icons/notif_icon_info.edds");
            }
        }
    }
}
