class VIPTeleportLogger
{
    private static FileHandle m_LogFile;
    private static string m_LogFilePath;
    static int m_LogLevel = 2; // 0 = off, 1 = normal, 2 = debug
    static string ROOT_FOLDER = "$profile:SOSH79/VIPTeleport/Logs/";
    // Daily rotation: one log file per day
    static string LOG_FILE = ROOT_FOLDER + "VIPTeleport_%1.log";

    static void InitLogFile()
    {
        if (m_LogLevel == 0)
            return;
        CheckDirectories();
        m_LogFilePath = LOG_FILE;
        m_LogFilePath.Replace("%1", GetDate());
        m_LogFile = OpenFile(m_LogFilePath, FileMode.APPEND);
        if (m_LogFile)
        {
            Print("[VIPTeleport] Log file initialized: " + m_LogFilePath);
            CloseFile(m_LogFile);
            m_LogFile = null;
        }
        else
        {
            Print("[VIPTeleport ERROR] Failed to initialize log file: " + m_LogFilePath);
        }
    }

    static void CheckDirectories()
    {
        if (!FileExist(ROOT_FOLDER))
        {
            bool rootCreated = MakeDirectory(ROOT_FOLDER);
            if (rootCreated)
            {
                Print("[VIPTeleport] Created log directory: " + ROOT_FOLDER);
            }
            else
            {
                Print("[VIPTeleport ERROR] Could not create log directory: " + ROOT_FOLDER);
            }
        }
    }

    static void Log(string message)
    {
        if (m_LogLevel == 0)
            return;
        CheckDirectories();
        m_LogFilePath = LOG_FILE;
        m_LogFilePath.Replace("%1", GetDate());
        m_LogFile = OpenFile(m_LogFilePath, FileMode.APPEND);
        if (m_LogFile)
        {
            FPrintln(m_LogFile, "[" + GetTime() + "] " + message);
            CloseFile(m_LogFile);
            m_LogFile = null;
        }
        else
        {
            Print("[VIPTeleport ERROR] Failed to open log file for writing: " + m_LogFilePath);
        }
    }

    static void LogDebug(string message)
    {
        if (m_LogLevel < 2)
            return;
        Log("[DEBUG] " + message);
    }

    static string GetDate()
    {
        int year, month, day;
        GetYearMonthDay(year, month, day);
        string monthStr = month.ToString();
        string dayStr = day.ToString();
        if (month < 10)
            monthStr = "0" + monthStr;
        if (day < 10)
            dayStr = "0" + dayStr;
        return year.ToString() + "-" + monthStr + "-" + dayStr;
    }

    static string GetTime()
    {
        int hour, minute, second;
        GetHourMinuteSecond(hour, minute, second);
        return hour.ToString() + ":" + minute.ToString() + ":" + second.ToString();
    }
}
