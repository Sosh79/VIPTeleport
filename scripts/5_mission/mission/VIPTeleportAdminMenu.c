class VIPTeleportAdminMenu extends UIScriptedMenu
{
    private TextWidget m_TitleText;
    private TextWidget m_StatusText;
    private ButtonWidget m_ReloadButton;
    private ButtonWidget m_CloseButton;
    
    // New controls
    private CheckBoxWidget m_EnableCooldownCheckbox;
    private EditBoxWidget m_CooldownSecondsEdit;
    private EditBoxWidget m_MaxTeleportsEdit;
    private ButtonWidget m_ApplySettingsButton;

    void VIPTeleportAdminMenu()
    {
        // Admin menu constructor
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
            return null; // Failed to load
        }

        m_TitleText = TextWidget.Cast(layoutRoot.FindAnyWidget("TitleText"));
        m_StatusText = TextWidget.Cast(layoutRoot.FindAnyWidget("StatusText"));
        m_ReloadButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("ReloadButton"));
        m_CloseButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("CloseButton"));
        
        // New controls
        m_EnableCooldownCheckbox = CheckBoxWidget.Cast(layoutRoot.FindAnyWidget("EnableCooldownCheckbox"));
        m_CooldownSecondsEdit = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("CooldownSecondsEdit"));
        m_MaxTeleportsEdit = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("MaxTeleportsEdit"));
        m_ApplySettingsButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("ApplySettingsButton"));

        // Freeze camera and show cursor
        GetGame().GetInput().ChangeGameFocus(1);
        GetGame().GetUIManager().ShowUICursor(true);

        // Fill current values
        LoadCurrentConfig();
        
        UpdateStatus("Ready");

        return layoutRoot;
    }

    void LoadCurrentConfig()
    {

        if (m_EnableCooldownCheckbox)
            m_EnableCooldownCheckbox.SetChecked(VIPTeleportConfig.m_EnableCooldown);
        
        if (m_CooldownSecondsEdit)
            m_CooldownSecondsEdit.SetText(VIPTeleportConfig.m_CooldownSeconds.ToString());
            
        if (m_MaxTeleportsEdit)
            m_MaxTeleportsEdit.SetText(VIPTeleportConfig.m_MaxTeleportsPerHour.ToString());
    }

    void SanitizeNumericInput(EditBoxWidget w)
    {
        if (!w) return;
        
        string original = w.GetText();
        string sanitized = "";
        
        for (int i = 0; i < original.Length(); i++)
        {
            string char = original.Substring(i, 1);
            if (char == "0" || char == "1" || char == "2" || char == "3" || char == "4" || char == "5" || char == "6" || char == "7" || char == "8" || char == "9")
            {
                sanitized += char;
            }
        }
        
        if (sanitized != original)
        {
            w.SetText(sanitized);
        }
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
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());

        if (w == m_ReloadButton)
        {
            UpdateStatus("Reloading from disk...");
            if (player)
            {
                player.RequestAdminReloadConfig();
            }
            return true;
        }

        if (w == m_ApplySettingsButton)
        {
            bool enabled = m_EnableCooldownCheckbox.IsChecked();
            int seconds = m_CooldownSecondsEdit.GetText().ToInt();
            int maxTele = m_MaxTeleportsEdit.GetText().ToInt();
            
            UpdateStatus("Applying changes...");
            
            if (player)
            {
                player.UpdateAdminConfig(enabled, seconds, maxTele);
            }
            return true;
        }

        if (w == m_CloseButton)
        {
            MissionGameplay mission = MissionGameplay.Cast(GetGame().GetMission());
            if (mission)
            {
                mission.CloseAdminMenu();
            }
            return true;
        }

        return false;
    }

    override bool OnChange(Widget w, int x, int y, bool finished)
    {
        super.OnChange(w, x, y, finished);
        
        if (w == m_CooldownSecondsEdit || w == m_MaxTeleportsEdit)
        {
            SanitizeNumericInput(EditBoxWidget.Cast(w));
            return true;
        }
        
        return false;
    }

    void ShowReloadResult(bool success, string message)
    {
        if (success)
        {
            UpdateStatus("SUCCESS: " + message);
            // Re-load fields in case they changed on disk
            LoadCurrentConfig();
        }
        else
        {
            UpdateStatus("ERROR: " + message);
        }
    }

    override void OnHide()
    {
        super.OnHide();
        GetGame().GetInput().ChangeGameFocus(-1);
        GetGame().GetUIManager().ShowUICursor(false);
    }
}
