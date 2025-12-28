class VIPTeleportNotificationManager
{
    private static ref array<ref VIPTeleportNotification> m_Notifications = new array<ref VIPTeleportNotification>();
    private static const float NOTIFICATION_SPACING = 5;
    private static const float START_Y = 100;

    static void AddNotification(string title, string message, string iconPath, float duration = 5.0)
    {
        // We insert at start so newest is always at index 0 (top)
        VIPTeleportNotification notification = new VIPTeleportNotification(title, message, iconPath, duration, START_Y);
        m_Notifications.InsertAt(notification, 0);
        UpdatePositions();
    }

    static void UpdatePositions()
    {
        float currentY = START_Y;
        for (int i = 0; i < m_Notifications.Count(); i++)
        {
            VIPTeleportNotification notif = m_Notifications.Get(i);
            if (notif)
            {
                notif.SetPosition(currentY);
                currentY += notif.GetHeight() + NOTIFICATION_SPACING;
            }
        }
    }

    static void RemoveNotification(VIPTeleportNotification notification)
    {
        int index = m_Notifications.Find(notification);
        if (index != -1)
        {
            m_Notifications.Remove(index);
            UpdatePositions();
        }
    }

    static void OnUpdate(float timeslice)
    {
        for (int i = m_Notifications.Count() - 1; i >= 0; i--)
        {
            VIPTeleportNotification notif = m_Notifications.Get(i);
            if (notif)
            {
                notif.Update(timeslice);
            }
        }
    }
}

class VIPTeleportNotification
{
    private Widget m_Root;
    private TextWidget m_Title;
    private MultilineTextWidget m_Message;
    private ImageWidget m_Icon;
    private float m_Timer;
    private float m_Duration;
    private float m_TargetY;

    void VIPTeleportNotification(string title, string message, string iconPath, float duration, float startY)
    {
        m_Root = GetGame().GetWorkspace().CreateWidgets("VIPTeleport/GUI/layouts/VIPTeleportNotification.layout");
        m_Title = TextWidget.Cast(m_Root.FindAnyWidget("Title"));
        m_Message = MultilineTextWidget.Cast(m_Root.FindAnyWidget("Message"));
        m_Icon = ImageWidget.Cast(m_Root.FindAnyWidget("Icon"));

        m_Title.SetText(title);
        m_Message.SetText(message);
        m_Icon.LoadImageFile(0, iconPath);
        
        m_Duration = duration;
        m_Timer = 0;

        // Start off-screen right at the correct Y level
        // This ensures it slides in horizontally only
        m_Root.SetPos(500, startY); 
        m_TargetY = startY;
    }

    void SetPosition(float y)
    {
        m_TargetY = y;
    }

    float GetHeight()
    {
        float w, h;
        m_Root.GetSize(w, h);
        return h;
    }

    void Update(float timeslice)
    {
        m_Timer += timeslice;
        if (m_Timer >= m_Duration)
        {
            Destroy();
            return;
        }

        float x, y;
        m_Root.GetPos(x, y);
        
        // Pure Slide from right (to 10 pixels from edge)
        float targetX = 10;
        float newX = Math.Lerp(x, targetX, timeslice * 8.0); // Smooth horizontal entrance
        
        // Smoothly vertical stacking lerp (if older ones move down)
        float newY = Math.Lerp(y, m_TargetY, timeslice * 8.0);
        
        m_Root.SetPos(newX, newY);

        // Fade out near end
        if (m_Timer > m_Duration * 0.8)
        {
            float remaining = m_Duration - m_Timer;
            float fadeTime = m_Duration * 0.2;
            float alpha = remaining / fadeTime;
            m_Root.SetAlpha(alpha);
        }
    }

    void Destroy()
    {
        if (m_Root)
            m_Root.Unlink();
        VIPTeleportNotificationManager.RemoveNotification(this);
    }
}
