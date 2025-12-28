modded class MissionGameplay
{
    protected ref VIPTeleportMenu m_VIPTeleportMenu;
    protected ref VIPTeleportAdminMenu m_VIPTeleportAdminMenu;
    protected bool m_IsVIP = false;
    protected float m_KeyPressTimer = 0;
    protected const float KEY_PRESS_COOLDOWN = 0.5;
    protected bool m_MenuIsOpen = false;
    protected bool m_AdminMenuIsOpen = false;

    override void OnInit()
    {
        super.OnInit();
    }

    override void OnMissionFinish()
    {
        super.OnMissionFinish();
    }

    override void OnUpdate(float timeslice)
    {
        super.OnUpdate(timeslice);

        m_KeyPressTimer += timeslice;

        // Update custom notifications
        VIPTeleportNotificationManager.OnUpdate(timeslice);

        // Check for pending menu from VIPTeleportFunctions
        if (VIPTeleportFunctions.HasPendingMenu())
        {
            array<ref VIPTeleportLocation> locations;
            string menuTitle;
            VIPTeleportFunctions.GetPendingMenuData(locations, menuTitle);

            // Handle pending menu
            ShowTeleportMenu(locations, menuTitle);
        }

        // Check for pending admin menu from VIPTeleportFunctions
        if (VIPTeleportFunctions.HasPendingAdminMenu())
        {
            // Handle pending admin menu
            VIPTeleportFunctions.ClearPendingAdminMenu();
            ShowAdminMenu();
        }

        // Check for pending admin reload result
        if (VIPTeleportFunctions.HasPendingAdminReloadResult())
        {
            bool reloadSuccess;
            string reloadMessage;
            VIPTeleportFunctions.GetPendingAdminReloadResult(reloadSuccess, reloadMessage);

            if (m_VIPTeleportAdminMenu)
            {
                m_VIPTeleportAdminMenu.ShowReloadResult(reloadSuccess, reloadMessage);
            }
        }

        // Check for pending custom dialog from VIPTeleportFunctions
        if (VIPTeleportFunctions.HasPendingDialog())
        {
            string dialogMessage = VIPTeleportFunctions.GetPendingDialogMessage();
            VIPTeleportDialog dialog = VIPTeleportDialog.Cast(GetGame().GetUIManager().EnterScriptedMenu(MENU_VIPTELEPORT_DIALOG, null));
            if (dialog)
            {
                dialog.SetMessage(dialogMessage);
            }
        }

        // Check for pending custom notification
        if (VIPTeleportFunctions.HasPendingNotification())
        {
            string nTitle, nMessage, nIcon;
            float nDuration;
            VIPTeleportFunctions.GetPendingNotification(nTitle, nMessage, nIcon, nDuration);
            VIPTeleportNotificationManager.AddNotification(nTitle, nMessage, nIcon, nDuration);
        }

        // Check for menu key press with toggle
        if (m_KeyPressTimer >= KEY_PRESS_COOLDOWN)
        {
            Input input = GetGame().GetInput();
            if (input.LocalPress("UAVIPTeleportMenu", false))
            {
                ToggleTeleportMenu();
                m_KeyPressTimer = 0;
            }

            // Check for admin menu key press
            if (input.LocalPress("UAVIPTeleportAdminMenu", false))
            {
                ToggleAdminMenu();
                m_KeyPressTimer = 0;
            }
        }
    }

    override void OnMissionStart()
    {
        super.OnMissionStart();
    }

    void SetMenuOpen(bool isOpen)
    {
        m_MenuIsOpen = isOpen;
    }

    void ToggleTeleportMenu()
    {
        if (m_MenuIsOpen)
        {
            CloseTeleportMenu();
        }
        else
        {
            // Request menu data from server
            PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
            if (player)
            {
                ScriptRPC rpc = new ScriptRPC();
                rpc.Send(player, RPC_VIP_TELEPORT_OPEN_MENU, true, null);
            }
        }
    }

    void ToggleAdminMenu()
    {
        if (m_AdminMenuIsOpen)
        {
            CloseAdminMenu();
        }
        else
        {
            // Request admin menu from server
            PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
            if (player)
            {
                player.RequestAdminMenu();
            }
        }
    }

    void CloseTeleportMenu()
    {
        // Restore camera control
        GetGame().GetInput().ChangeGameFocus(-1);
        GetGame().GetUIManager().ShowUICursor(false);

        if (m_VIPTeleportMenu)
        {
            m_VIPTeleportMenu.Close();
            m_VIPTeleportMenu = null;
        }
        m_MenuIsOpen = false;
    }

    void ShowTeleportMenu(array<ref VIPTeleportLocation> locations, string menuTitle)
    {
        if (!locations || locations.Count() == 0)
        {
            return; // No locations
        }

        m_IsVIP = true;

        // Close existing menu if open
        if (GetGame().GetUIManager().IsMenuOpen(MENU_VIPTELEPORT))
        {
            GetGame().GetUIManager().HideScriptedMenu(m_VIPTeleportMenu);
        }

        // Create fresh menu
        m_VIPTeleportMenu = new VIPTeleportMenu();

        if (m_VIPTeleportMenu)
        {
            m_VIPTeleportMenu.LoadLocations(locations, menuTitle);
            m_MenuIsOpen = true;
            GetGame().GetUIManager().ShowScriptedMenu(m_VIPTeleportMenu, null);
        }
    }

    void ShowAdminMenu()
    {
        // Close existing admin menu if open
        if (GetGame().GetUIManager().IsMenuOpen(MENU_VIPTELEPORT_ADMIN))
        {
            GetGame().GetUIManager().HideScriptedMenu(m_VIPTeleportAdminMenu);
        }

        // Create new admin menu
        m_VIPTeleportAdminMenu = new VIPTeleportAdminMenu();

        if (m_VIPTeleportAdminMenu)
        {
            m_AdminMenuIsOpen = true;
            GetGame().GetUIManager().ShowScriptedMenu(m_VIPTeleportAdminMenu, null);
        }
    }

    void CloseAdminMenu()
    {

        // Restore camera control before closing
        GetGame().GetInput().ChangeGameFocus(-1);
        GetGame().GetUIManager().ShowUICursor(false);

        if (m_VIPTeleportAdminMenu)
        {
            m_VIPTeleportAdminMenu.Close();
            m_VIPTeleportAdminMenu = null;
        }
        m_AdminMenuIsOpen = false;
    }

    void SetAdminMenuOpen(bool isOpen)
    {
        m_AdminMenuIsOpen = isOpen;
    }

    override UIScriptedMenu CreateScriptedMenu(int id)
    {
        UIScriptedMenu menu = super.CreateScriptedMenu(id);
        if (!menu)
        {
            switch (id)
            {
                case MENU_VIPTELEPORT_DIALOG:
                    menu = new VIPTeleportDialog();
                    break;
            }
            if (menu)
            {
                menu.SetID(id);
            }
        }
        return menu;
    }

}
