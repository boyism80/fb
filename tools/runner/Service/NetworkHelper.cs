using System.Net.Sockets;

namespace Runner.Service
{
    public static class NetworkHelper
    {
        public static bool TcpPing(string host, ushort port, int timeoutMs = 2000)
        {
            if (string.IsNullOrWhiteSpace(host))
                return false;

            using var client = new TcpClient();
            try
            {
                var connect = client.ConnectAsync(host, port);
                if (!connect.Wait(timeoutMs))
                    return false;

                return client.Connected;
            }
            catch
            {
                return false;
            }
        }
    }
}
