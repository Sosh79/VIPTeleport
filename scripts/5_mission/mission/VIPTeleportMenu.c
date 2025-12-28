class VIPTeleportMenu : UIScriptedMenu
{
    // Static cache for menu data (shared across all menu instances)
    static ref array<ref VIPTeleportLocation> s_CachedLocations = null;
    static string s_CachedMenuTitle = "";
    static bool s_CacheValid = false;
    
    protected TextWidget m_Title;
    protected TextListboxWidget m_LocationList;
    protected ButtonWidget m_TeleportButton;
    protected ButtonWidget m_CloseButton;
    protected int m_SelectedIndex = -1;
    protected ref array<ref VIPTeleportLocation> m_Locations;
    protected string m_PendingMenuTitle;
    protected bool m_DataLoaded = false;

    void VIPTeleportMenu()
    {
        // Menu constructor
    }

    override int GetID()
    {
        return MENU_VIPTELEPORT;
    }

    override Widget Init()
    {
        layoutRoot = GetGame().GetWorkspace().CreateWidgets("VIPTeleport/GUI/layouts/VIPTeleportMenu.layout");

        if (!layoutRoot)
        {
            // Failed to load menu layout
            return null;
        }

        m_Title = TextWidget.Cast(layoutRoot.FindAnyWidget("Title"));
        m_LocationList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("LocationList"));
        
        m_TeleportButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("TeleportButton"));
        m_CloseButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("CloseButton"));

        // Make sure the root widget is visible
        if (layoutRoot)
        {
            layoutRoot.Show(true);
        }

        // Freeze camera movement, allow mouse only
        GetGame().GetInput().ChangeGameFocus(1);
        GetGame().GetUIManager().ShowUICursor(true);

        return layoutRoot;
    }

    override void OnShow()
    {
        super.OnShow();

        // Make sure layout is visible
        if (layoutRoot)
        {
            layoutRoot.Show(true);
        }

        // Try to load from cache first
        if (s_CacheValid && s_CachedLocations && s_CachedLocations.Count() > 0)
        {
            // Load from cache
            m_Locations = s_CachedLocations;
            m_PendingMenuTitle = s_CachedMenuTitle;
            PopulateList();
            m_DataLoaded = true;
        }
        else
        {
            // Request from server
            RequestMenuDataFromServer();
        }
    }

    override void OnHide()
    {
        super.OnHide();
        // Restore camera control

        // Restore camera control
        GetGame().GetInput().ChangeGameFocus(-1);
        GetGame().GetUIManager().ShowUICursor(false);

        // Reset for next open
        m_DataLoaded = false;
        m_SelectedIndex = -1;

        // Notify MissionGameplay that menu is closed
        MissionGameplay mission = MissionGameplay.Cast(GetGame().GetMission());
        if (mission)
        {
            mission.SetMenuOpen(false);
        }
    }

    // Load menu data (can be from server RPC or from cache)
    void LoadLocations(array<ref VIPTeleportLocation> locations, string menuTitle)
    {
        // Load location data

        if (!locations)
        {
            return; // Invalid data
        }

        m_Locations = locations;
        m_PendingMenuTitle = menuTitle;
        
        // Update cache
        s_CachedLocations = locations;
        s_CachedMenuTitle = menuTitle;
        s_CacheValid = true;
        
        m_DataLoaded = false;
    }

    // Request fresh data from server
    void RequestMenuDataFromServer()
    {
        // Request menu from server
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (player)
        {
            ScriptRPC rpc = new ScriptRPC();
            rpc.Send(player, RPC_VIP_TELEPORT_OPEN_MENU, true, null);
        }
    }

    void LoadLocationsInternal()
    {
        if (!m_Locations)
        {
            return; // No locations
        }

        // Set title
        if (m_Title)
        {
            m_Title.SetText(m_PendingMenuTitle);
        }

        // Populate location list
        if (m_LocationList)
        {
            m_LocationList.ClearItems();

            if (m_Locations && m_Locations.Count() > 0)
            {
                for (int i = 0; i < m_Locations.Count(); i++)
                {
                    VIPTeleportLocation loc = m_Locations.Get(i);
                    if (loc)
                    {
                        m_LocationList.AddItem(loc.Name, NULL, 0);
                    }
                }
                
                // Show and select first item
                m_LocationList.Show(true);
                m_LocationList.SelectRow(0);
            }
        }

        UpdateDescription();
    }

    // Helper: Format distance for display
    string FormatDistance(float distanceMeters)
    {
        if (distanceMeters >= 1000)
        {
            // Convert to km and round to 1 decimal
            float distanceKm = distanceMeters / 1000.0;
            int roundedKm = Math.Round(distanceKm * 10);
            float displayKm = roundedKm / 10.0;
            return displayKm.ToString() + "km";
        }
        else
        {
            // Round to nearest meter
            int roundedM = Math.Round(distanceMeters);
            return roundedM.ToString() + "m";
        }
    }

    // Helper: Populate list from current m_Locations
    void PopulateList()
    {
        if (!m_LocationList || !m_Locations) return;
        
        m_LocationList.ClearItems();
        
        for (int i = 0; i < m_Locations.Count(); i++)
        {
            VIPTeleportLocation loc = m_Locations.Get(i);
            if (loc)
            {
                // Add distance to location name
                string displayName = loc.Name + " (" + FormatDistance(loc.Distance) + ")";
                m_LocationList.AddItem(displayName, NULL, 0);
            }
        }
        
        if (m_Title)
            m_Title.SetText(m_PendingMenuTitle);
            
        m_DataLoaded = true;
    }

    // Static method: Invalidate cache (call when config reloads)
    static void InvalidateCache()
    {
        s_CacheValid = false;
        s_CachedLocations = null;
        s_CachedMenuTitle = "";
    }

    override bool OnChange(Widget w, int x, int y, bool finished)
    {
        super.OnChange(w, x, y, finished);

        if (w == m_LocationList)
        {
            m_SelectedIndex = m_LocationList.GetSelectedRow();
            UpdateDescription();
            return true;
        }

        return false;
    }

    override bool OnItemSelected(Widget w, int x, int y, int row, int column, int oldRow, int oldColumn)
    {
        super.OnItemSelected(w, x, y, row, column, oldRow, oldColumn);

        if (w == m_LocationList)
        {
            m_SelectedIndex = row;
            UpdateDescription();
            return true;
        }

        return false;
    }

    override bool OnDoubleClick(Widget w, int x, int y, int button)
    {
        super.OnDoubleClick(w, x, y, button);

        if (w == m_LocationList && m_SelectedIndex >= 0)
        {
            OnTeleportClicked();
            return true;
        }

        return false;
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        super.OnClick(w, x, y, button);

        if (w == m_TeleportButton)
        {
            OnTeleportClicked();
            return true;
        }
        else if (w == m_CloseButton)
        {
            Close();
            return true;
        }

        return false;
    }

    // Handle ESC key to close menu
    override bool OnKeyDown(Widget w, int x, int y, int key)
    {
        super.OnKeyDown(w, x, y, key);

        if (key == KeyCode.KC_ESCAPE || key == KeyCode.KC_F5)
        {
            // Close menu
            Close();
            return true;
        }

        return false;
    }

    void UpdateDescription()
    {
        // Description display disabled
    }

    void OnTeleportClicked()
    {
        if (m_SelectedIndex >= 0 && m_SelectedIndex < m_Locations.Count())
        {
            VIPTeleportLocation selectedLoc = m_Locations.Get(m_SelectedIndex);

            // Send teleport request to server
            PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
            if (player)
            {
                ScriptRPC rpc = new ScriptRPC();
                
                // SECURITY: Send location name and position instead of index for validation
                rpc.Write(selectedLoc.Name);
                rpc.Write(selectedLoc.Position);
                
                rpc.Send(player, RPC_VIP_TELEPORT_REQUEST, true, null);

                Close();
            }
        }
        else
        {
            GetGame().GetUIManager().ShowDialog("VIP Teleport", "Please select a location first", 0, DBT_OK, DBB_NONE, DMT_NONE, null);
        }
    }
}
