class VIPTeleportMenu : UIScriptedMenu
{
    protected TextWidget m_Title;
    protected TextListboxWidget m_LocationList;
    protected MultilineTextWidget m_Description;
    protected int m_SelectedIndex = -1;
    protected ref array<ref VIPTeleportLocation> m_Locations;
    protected string m_PendingMenuTitle;
    protected bool m_DataLoaded = false;

    void VIPTeleportMenu()
    {
        Print("[VIPTeleport] Menu created");
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
            Print("[VIPTeleport] Failed to load menu layout");
            return null;
        }

        m_Title = TextWidget.Cast(layoutRoot.FindAnyWidget("Title"));
        m_LocationList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("LocationList"));
        m_Description = MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("Description"));

        Print("[VIPTeleport] Widget status:");
        Print("[VIPTeleport] - Title: " + (m_Title != null));
        Print("[VIPTeleport] - LocationList: " + (m_LocationList != null));
        Print("[VIPTeleport] - Description: " + (m_Description != null));

        // Make sure the root widget is visible
        if (layoutRoot)
        {
            layoutRoot.Show(true);
            Print("[VIPTeleport] Root widget visibility set to true");
        }

        // Freeze camera movement, allow mouse only
        GetGame().GetInput().ChangeGameFocus(1);
        GetGame().GetUIManager().ShowUICursor(true);

        Print("[VIPTeleport] Menu initialized successfully");

        return layoutRoot;
    }

    override void OnShow()
    {
        super.OnShow();
        Print("[VIPTeleport] OnShow called");

        // Camera control already set in Init(), don't call it again

        // Make sure layout is visible
        if (layoutRoot)
        {
            layoutRoot.Show(true);
            Print("[VIPTeleport] layoutRoot shown");
        }

        // Load data now that widgets are initialized
        if (!m_DataLoaded && m_Locations && m_Locations.Count() > 0)
        {
            Print("[VIPTeleport] OnShow - Loading locations now");
            LoadLocationsInternal();
            m_DataLoaded = true;
        }
        else
        {
            Print("[VIPTeleport] OnShow - Data already loaded or no locations");
        }
    }

    override void OnHide()
    {
        super.OnHide();
        Print("[VIPTeleport] OnHide called - Restoring camera control");

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

        Print("[VIPTeleport] Camera control restored");
    }

    void LoadLocations(array<ref VIPTeleportLocation> locations, string menuTitle)
    {
        Print("[VIPTeleport] LoadLocations - Storing data for later");

        if (!locations)
        {
            Print("[VIPTeleport] ERROR: locations array is null!");
            return;
        }

        Print("[VIPTeleport] Storing " + locations.Count() + " locations");
        m_Locations = locations;
        m_PendingMenuTitle = menuTitle;
        m_DataLoaded = false;
    }

    void LoadLocationsInternal()
    {
        Print("[VIPTeleport] ========== LoadLocationsInternal START ==========");

        if (!m_Locations)
        {
            Print("[VIPTeleport] ERROR: locations array is null!");
            return;
        }

        Print("[VIPTeleport] Locations count: " + m_Locations.Count());

        // Print all locations for debugging
        for (int j = 0; j < m_Locations.Count(); j++)
        {
            VIPTeleportLocation testLoc = m_Locations.Get(j);
            if (testLoc)
            {
                Print("[VIPTeleport] Location " + j + ": " + testLoc.Name);
            }
        }

        if (m_Title)
        {
            m_Title.SetText(m_PendingMenuTitle);
            Print("[VIPTeleport] Title set: " + m_PendingMenuTitle);
        }
        else
        {
            Print("[VIPTeleport] ERROR: Title widget is null!");
        }

        if (m_LocationList)
        {
            Print("[VIPTeleport] LocationList widget found");
            Print("[VIPTeleport] Clearing location list");
            m_LocationList.ClearItems();

            Print("[VIPTeleport] Items after clear: " + m_LocationList.GetNumItems());

            if (m_Locations && m_Locations.Count() > 0)
            {
                Print("[VIPTeleport] Starting to add " + m_Locations.Count() + " locations to list");
                for (int i = 0; i < m_Locations.Count(); i++)
                {
                    VIPTeleportLocation loc = m_Locations.Get(i);
                    if (loc)
                    {
                        Print("[VIPTeleport] [" + i + "] Adding location: " + loc.Name);
                        int rowIndex = m_LocationList.AddItem(loc.Name, NULL, 0);
                        Print("[VIPTeleport] [" + i + "] Added at row: " + rowIndex);
                    }
                    else
                    {
                        Print("[VIPTeleport] ERROR: Location at index " + i + " is null!");
                    }
                }
                int itemCount = m_LocationList.GetNumItems();
                Print("[VIPTeleport] LocationList final count: " + itemCount);

                if (itemCount > 0)
                {
                    Print("[VIPTeleport] SUCCESS: Locations added to list!");

                    // Make sure the listbox is visible
                    m_LocationList.Show(true);

                    // Try to select first item to test
                    m_LocationList.SelectRow(0);
                    Print("[VIPTeleport] Selected first row for testing");
                }
                else
                {
                    Print("[VIPTeleport] ERROR: No items in list after adding!");
                }
            }
            else
            {
                Print("[VIPTeleport] ERROR: No locations to add!");
            }
        }
        else
        {
            Print("[VIPTeleport] ERROR: LocationList widget is null!");
        }

        Print("[VIPTeleport] ========== LoadLocationsInternal END ==========");

        UpdateDescription();
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
            Print("[VIPTeleport] Double-click on location list");
            OnTeleportClicked();
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
            Print("[VIPTeleport] Close key pressed - closing menu");
            Close();
            return true;
        }

        return false;
    }

    void UpdateDescription()
    {
        if (!m_Description)
            return;

        if (m_SelectedIndex >= 0 && m_SelectedIndex < m_Locations.Count())
        {
            VIPTeleportLocation loc = m_Locations.Get(m_SelectedIndex);
            string desc = loc.Description + "\n\n";
            desc += "Position: " + loc.Position.ToString();
            m_Description.SetText(desc);
        }
        else
        {
            m_Description.SetText("Select a location to teleport");
        }
    }

    void OnTeleportClicked()
    {
        Print("[VIPTeleport] OnTeleportClicked - Selected index: " + m_SelectedIndex);

        if (m_SelectedIndex >= 0 && m_SelectedIndex < m_Locations.Count())
        {
            VIPTeleportLocation selectedLoc = m_Locations.Get(m_SelectedIndex);
            Print("[VIPTeleport] Teleporting to: " + selectedLoc.Name);

            // Send teleport request to server
            PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
            if (player)
            {
                Print("[VIPTeleport] Sending teleport RPC to server");
                ScriptRPC rpc = new ScriptRPC();
                rpc.Write(m_SelectedIndex);
                rpc.Send(player, RPC_VIP_TELEPORT_REQUEST, true, null);

                Print("[VIPTeleport] Closing menu after teleport request");
                Close();
            }
            else
            {
                Print("[VIPTeleport] ERROR: Player is null!");
            }
        }
        else
        {
            Print("[VIPTeleport] ERROR: No location selected or invalid index");
            GetGame().GetUIManager().ShowDialog("VIP Teleport", "Please select a location first", 0, DBT_OK, DBB_NONE, DMT_NONE, null);
        }
    }
}
