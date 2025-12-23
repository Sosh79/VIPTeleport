class VIPTeleportAdminMenu extends UIScriptedMenu
{
    private TextWidget m_TitleText;
    private TextWidget m_StatusText;
    private ButtonWidget m_ReloadButton;
    private ButtonWidget m_CloseButton;

    void VIPTeleportAdminMenu()
    {
        Print("[VIPTeleport] Admin Menu created");
    }

    override int GetID()
    {
        return MENU_VIPTELEPORT_ADMIN;
    }

    override Widget Init()
    {
        layoutRoot = GetGame().GetWorkspace().CreateWidgets("VIPTeleport/GUI/layouts/VIPTeleportAdminMenu.layout");

        if (!layoutRoot)
        {
            Print("[VIPTeleport] Failed to load admin menu layout");
            return null;
        }

        m_TitleText = TextWidget.Cast(layoutRoot.FindAnyWidget("TitleText"));
        m_StatusText = TextWidget.Cast(layoutRoot.FindAnyWidget("StatusText"));
        m_ReloadButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("ReloadButton"));
        m_CloseButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("CloseButton"));

        // Freeze camera and show cursor
        GetGame().GetInput().ChangeGameFocus(1);
        GetGame().GetUIManager().ShowUICursor(true);

        UpdateStatus("Ready");
        Print("[VIPTeleport] Admin menu initialized successfully");

        return layoutRoot;
    }

    void UpdateStatus(string message)
    {
        if (m_StatusText)
        {
            m_StatusText.SetText("Status: " + message);
        }
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (w == m_ReloadButton)
        {
            Print("[VIPTeleport] Admin clicked reload button");
            UpdateStatus("Reloading configuration...");

            // Send reload request to server
            PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
            if (player)
            {
                player.RequestAdminReloadConfig();
            }

            return true;
        }

        if (w == m_CloseButton)
        {
            Print("[VIPTeleport] Admin: Close button clicked");

            // Get MissionGameplay to close the menu properly
            MissionGameplay mission = MissionGameplay.Cast(GetGame().GetMission());
            if (mission)
            {
                mission.CloseAdminMenu();
            }

            return true;
        }

        return false;
    }

    void ShowReloadResult(bool success, string message)
    {
        if (success)
        {
            UpdateStatus("SUCCESS: " + message);
        }
        else
        {
            UpdateStatus("ERROR: " + message);
        }
    }

    override void OnShow()
    {
        super.OnShow();
        Print("[VIPTeleport] Admin menu shown");
    }

    override void OnHide()
    {
        super.OnHide();
        Print("[VIPTeleport] Admin menu hidden - restoring camera control");

        // Restore camera control
        GetGame().GetInput().ChangeGameFocus(-1);
        GetGame().GetUIManager().ShowUICursor(false);
    }
}
