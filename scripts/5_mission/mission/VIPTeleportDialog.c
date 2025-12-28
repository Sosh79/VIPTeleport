class VIPTeleportDialog extends UIScriptedMenu
{
    protected MultilineTextWidget m_Content;
    protected ButtonWidget m_OkButton;
    protected string m_Message;

    void VIPTeleportDialog()
    {
    }

    override Widget Init()
    {
        layoutRoot = GetGame().GetWorkspace().CreateWidgets("VIPTeleport/GUI/layouts/VIPTeleportDialog.layout");

        m_Content = MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("DialogContent"));
        m_OkButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("OkButton"));

        if (m_Message != "")
        {
            m_Content.SetText(m_Message);
        }

        return layoutRoot;
    }

    override void OnShow()
    {
        super.OnShow();
        GetGame().GetInput().ChangeGameFocus(1);
        GetGame().GetUIManager().ShowUICursor(true);
    }

    override void OnHide()
    {
        super.OnHide();
        GetGame().GetInput().ChangeGameFocus(-1);
        GetGame().GetUIManager().ShowUICursor(false);
    }

    void SetMessage(string message)
    {
        m_Message = message;
        if (m_Content)
        {
            m_Content.SetText(m_Message);
        }
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        super.OnClick(w, x, y, button);

        if (w == m_OkButton)
        {
            Close();
            return true;
        }

        return false;
    }

    // Allow closing with Enter or ESC
    override bool OnKeyDown(Widget w, int x, int y, int key)
    {
        super.OnKeyDown(w, x, y, key);

        if (key == KeyCode.KC_ESCAPE || key == KeyCode.KC_RETURN || key == KeyCode.KC_NUMPADENTER)
        {
            Close();
            return true;
        }

        return false;
    }
}
