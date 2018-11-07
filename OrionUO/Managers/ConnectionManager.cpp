﻿// MIT License
// Copyright (C) August 2016 Hotride

CConnectionManager g_ConnectionManager;

NETWORK_INIT_TYPE *g_NetworkInit = nullptr;
NETWORK_ACTION_TYPE *g_NetworkAction = nullptr;
NETWORK_POST_ACTION_TYPE *g_NetworkPostAction = nullptr;

CConnectionManager::CConnectionManager()
{
}

CConnectionManager::~CConnectionManager()
{
    DEBUG_TRACE_FUNCTION;
    if (m_LoginSocket.Connected)
        m_LoginSocket.Disconnect();

    if (m_GameSocket.Connected)
        m_GameSocket.Disconnect();
}

void CConnectionManager::SetUseProxy(bool val)
{
    DEBUG_TRACE_FUNCTION;
    m_UseProxy = val;
    m_LoginSocket.UseProxy = val;
    m_GameSocket.UseProxy = val;
}

void CConnectionManager::SetProxyAddress(const string &val)
{
    DEBUG_TRACE_FUNCTION;
    m_ProxyAddress = val;
    m_LoginSocket.ProxyAddress = val;
    m_GameSocket.ProxyAddress = val;
}

void CConnectionManager::SetProxyPort(int val)
{
    DEBUG_TRACE_FUNCTION;
    m_ProxyPort = val;
    m_LoginSocket.ProxyPort = val;
    m_GameSocket.ProxyPort = val;
}

void CConnectionManager::SetProxySocks5(bool val)
{
    DEBUG_TRACE_FUNCTION;
    m_ProxySocks5 = val;
    m_LoginSocket.ProxySocks5 = val;
    m_GameSocket.ProxySocks5 = val;
}

void CConnectionManager::SetProxyAccount(const string &val)
{
    DEBUG_TRACE_FUNCTION;
    m_ProxyAccount = val;
    m_LoginSocket.ProxyAccount = val;
    m_GameSocket.ProxyAccount = val;
}

void CConnectionManager::SetProxyPassword(const string &val)
{
    DEBUG_TRACE_FUNCTION;
    m_ProxyPassword = val;
    m_LoginSocket.ProxyPassword = val;
    m_GameSocket.ProxyPassword = val;
}

void CConnectionManager::Init()
{
    DEBUG_TRACE_FUNCTION;
    if (m_LoginSocket.Connected)
        return;

    m_IsLoginSocket = true;
    const auto localIp = socket_localaddress();
    m_Seed[0] = static_cast<unsigned char>((localIp >> 24) & 0xff);
    m_Seed[1] = static_cast<unsigned char>((localIp >> 16) & 0xff);
    m_Seed[2] = static_cast<unsigned char>((localIp >> 8) & 0xff);
    m_Seed[3] = static_cast<unsigned char>(localIp & 0xff);

    g_NetworkInit(true, m_Seed);
}

void CConnectionManager::Init(uint8_t *gameSeed)
{
    DEBUG_TRACE_FUNCTION;
    if (m_GameSocket.Connected)
        return;

    m_IsLoginSocket = false;
    g_NetworkInit(false, gameSeed);
}

void CConnectionManager::SendIP(CSocket &socket, uint8_t *ip)
{
    DEBUG_TRACE_FUNCTION;
    PLUGIN_EVENT(UOMSG_IP_SEND, ip, 4);
    socket.Send(ip, 4);
}

bool CConnectionManager::Connect(const string &address, int port, uint8_t *gameSeed)
{
    DEBUG_TRACE_FUNCTION;
    LOG("Connecting %s:%d\n", address.c_str(), port);
    if (m_IsLoginSocket)
    {
        if (m_LoginSocket.Connected)
            return true;

        bool result = m_LoginSocket.Connect(address, port);
        if (result)
        {
            LOG("connected\n");
            g_TotalSendSize = 4;
            g_LastPacketTime = g_Ticks;
            g_LastSendTime = g_LastPacketTime;
            if (g_PacketManager.GetClientVersion() < CV_6060)
            {
                SendIP(m_LoginSocket, m_Seed);
            }
            else
            {
                uint8_t buf = 0xEF;
                m_LoginSocket.Send(&buf, 1); //0xEF
                SendIP(m_LoginSocket, m_Seed);
                Wisp::CDataWritter stream;
                auto &str = g_Orion.ClientVersionText;
                if (str.length())
                {
                    char ver[20] = { 0 };
                    char *ptr = ver;
                    strncpy_s(ver, str.c_str(), str.length());
                    int idx = 0;

                    for (int i = 0; i < (int)str.length(); i++)
                    {
                        if (ver[i] == '.')
                        {
                            ver[i] = 0;
                            stream.WriteUInt32BE(atoi(ptr));
                            ptr = ver + (i + 1);
                            idx++;

                            if (idx > 3)
                                break;
                        }
                    }

                    stream.WriteUInt32BE(atoi(ptr));
                }

                g_TotalSendSize = 21;
                m_LoginSocket.Send(stream.Data()); // Client version, 16 bytes
            }
        }
        else
        {
            m_LoginSocket.Disconnect();
        }
        return result;
    }
    else
    {
        if (m_GameSocket.Connected)
            return true;

        g_TotalSendSize = 4;
        g_LastPacketTime = g_Ticks;
        g_LastSendTime = g_LastPacketTime;

        const bool result = m_GameSocket.Connect(address, port);
        if (result)
        {
            SendIP(m_GameSocket, gameSeed);
        }

        m_LoginSocket.Disconnect();
        return result;
    }

    return false;
}

void CConnectionManager::Disconnect()
{
    DEBUG_TRACE_FUNCTION;
    if (m_LoginSocket.Connected)
        m_LoginSocket.Disconnect();

    if (m_GameSocket.Connected)
        m_GameSocket.Disconnect();
}

void CConnectionManager::Recv()
{
    DEBUG_TRACE_FUNCTION;
    if (m_IsLoginSocket)
    {
        if (!m_LoginSocket.Connected)
            return;

        if (!m_LoginSocket.ReadyRead())
        {
            if (m_LoginSocket.DataReady == -1)
            {
                LOG("Failed to Recv()...Disconnecting...\n");
                g_Orion.InitScreen(GS_MAIN_CONNECT);
                g_ConnectionScreen.SetType(CST_CONLOST);
            }
            return;
        }

        g_PacketManager.Read(&m_LoginSocket);
    }
    else
    {
        if (!m_GameSocket.Connected)
            return;

        if (!m_GameSocket.ReadyRead())
        {
            if (m_GameSocket.DataReady == -1)
            {
                LOG("Failed to Recv()...Disconnecting...\n");
                if (g_GameState == GS_GAME ||
                    (g_GameState == GS_GAME_BLOCKED && g_GameBlockedScreen.Code))
                {
                    g_Orion.DisconnectGump();
                }
                else
                {
                    g_Orion.InitScreen(GS_MAIN_CONNECT);
                    g_ConnectionScreen.SetType(CST_CONLOST);
                }
            }

            return;
        }

        g_PacketManager.Read(&m_GameSocket);
    }
}

int CConnectionManager::Send(uint8_t *buf, int size)
{
    DEBUG_TRACE_FUNCTION;
    if (g_TheAbyss)
    {
        switch (buf[0])
        {
            case 0x34:
                buf[0] = 0x71;
                break;
            case 0x72:
                buf[0] = 0x6C;
                break;
            case 0x6C:
                buf[0] = 0x72;
                break;
            case 0x3B:
                buf[0] = 0x34;
                break;
            case 0x6F:
                buf[0] = 0x56;
                break;
            case 0x56:
                buf[0] = 0x6F;
                break;
            default:
                break;
        }
    }
    else if (g_Asmut)
    {
        if (buf[0] == 0x02)
            buf[0] = 0x04;
        else if (buf[0] == 0x07)
            buf[0] = 0x0A;
    }

    if (m_IsLoginSocket)
    {
        if (!m_LoginSocket.Connected)
            return 0;

        vector<uint8_t> cbuf(size);
        g_NetworkAction(true, &buf[0], &cbuf[0], size);
        return m_LoginSocket.Send(cbuf);
    }
    else
    {
        if (!m_GameSocket.Connected)
            return 0;

        vector<uint8_t> cbuf(size);
        g_NetworkAction(false, &buf[0], &cbuf[0], size);
        return m_GameSocket.Send(cbuf);
    }

    return 0;
}

int CConnectionManager::Send(const vector<uint8_t> &data)
{
    DEBUG_TRACE_FUNCTION;
    return Send((uint8_t *)&data[0], (int)data.size());
}
