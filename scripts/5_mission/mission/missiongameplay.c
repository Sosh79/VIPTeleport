modded class MissionGameplay
{
    protected ref VIPTeleportMenu m_VIPTeleportMenu;
    protected bool m_IsVIP = false;
    protected float m_KeyPressTimer = 0;
    protected const float KEY_PRESS_COOLDOWN = 0.5;
    protected bool m_MenuIsOpen = false;

    override void OnInit()
    {
        super.OnInit();
        Print("[VIPTeleport] Client initialized");
    }

    override void OnMissionFinish()
    {
        super.OnMissionFinish();
    }

    override void OnUpdate(float timeslice)
    {
        super.OnUpdate(timeslice);

        m_KeyPressTimer += timeslice;

        // Check for pending menu from VIPTeleportFunctions
        if (VIPTeleportFunctions.HasPendingMenu())
        {
            array<ref VIPTeleportLocation> locations;
            string menuTitle;
            VIPTeleportFunctions.GetPendingMenuData(locations, menuTitle);

            Print("[VIPTeleport] MissionGameplay detected pending menu:");
            Print("[VIPTeleport] - Locations count: " + locations.Count());
            Print("[VIPTeleport] - Menu title: " + menuTitle);

            ShowTeleportMenu(locations, menuTitle);
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
        }
    }

    override void OnMissionStart()
    {
        super.OnMissionStart();
    }

    void SetMenuOpen(bool isOpen)
    {
        m_MenuIsOpen = isOpen;
        Print("[VIPTeleport] Menu state changed to: " + isOpen);
    }

    void ToggleTeleportMenu()
    {
        Print("[VIPTeleport] ToggleTeleportMenu called");
        Print("[VIPTeleport] m_MenuIsOpen flag: " + m_MenuIsOpen);

        if (m_MenuIsOpen)
        {
            Print("[VIPTeleport] Closing menu");
            CloseTeleportMenu();
        }
        else
        {
            Print("[VIPTeleport] Opening menu - requesting data from server");
            // Request menu data from server via RPC
            PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
            if (player)
            {
                ScriptRPC rpc = new ScriptRPC();
                rpc.Send(player, RPC_VIP_TELEPORT_OPEN_MENU, true, null);
            }
        }
    }

    void CloseTeleportMenu()
    {
        Print("[VIPTeleport] CloseTeleportMenu called");

        // Restore camera control before closing
        GetGame().GetInput().ChangeGameFocus(-1);
        GetGame().GetUIManager().ShowUICursor(false);
        Print("[VIPTeleport] Camera control restored in CloseTeleportMenu");

        if (m_VIPTeleportMenu)
        {
            Print("[VIPTeleport] Closing menu via Close()");
            m_VIPTeleportMenu.Close();
            m_VIPTeleportMenu = null;
        }
        else
        {
            Print("[VIPTeleport] Menu is already null");
        }
        m_MenuIsOpen = false;
    }

    void ShowTeleportMenu(array<ref VIPTeleportLocation> locations, string menuTitle)
    {
        Print("[VIPTeleport] ShowTeleportMenu called with " + locations.Count() + " locations");

        if (!locations || locations.Count() == 0)
        {
            Print("[VIPTeleport] ERROR: No locations to show!");
            return;
        }

        m_IsVIP = true;

        // Close existing menu if open
        if (GetGame().GetUIManager().IsMenuOpen(MENU_VIPTELEPORT))
        {
            Print("[VIPTeleport] Closing existing menu first");
            GetGame().GetUIManager().HideScriptedMenu(m_VIPTeleportMenu);
        }

        // Always create fresh menu
        Print("[VIPTeleport] Creating new menu instance");
        m_VIPTeleportMenu = new VIPTeleportMenu();

        if (m_VIPTeleportMenu)
        {
            Print("[VIPTeleport] Menu instance created, loading locations");
            m_VIPTeleportMenu.LoadLocations(locations, menuTitle);
            Print("[VIPTeleport] Showing scripted menu");
            m_MenuIsOpen = true;
            GetGame().GetUIManager().ShowScriptedMenu(m_VIPTeleportMenu, null);
            Print("[VIPTeleport] Menu displayed successfully");
        }
        else
        {
            Print("[VIPTeleport] ERROR: Failed to create menu instance!");
        }
    }

    void ShowTeleportResult(bool success, string message)
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (player)
        {
            if (success)
            {
                player.MessageStatus(message);
            }
            else
            {
                player.MessageImportant(message);
            }
        }
    }
}
