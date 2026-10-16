using System.Security.Cryptography;
using System.Text;

namespace Http.Service
{
    public static class PasswordHash
    {
        public static string Compute(string value)
        {
            return Hex(SHA256.HashData(Encoding.UTF8.GetBytes(value)));
        }

        public static bool Matches(string stored, string password, out string updated)
        {
            var utf8 = Compute(password);
            if (stored == utf8)
            {
                updated = null;
                return true;
            }

            var ascii = Hex(SHA256.HashData(Encoding.ASCII.GetBytes(password)));
            if (stored == ascii)
            {
                updated = utf8;
                return true;
            }

            updated = null;
            return false;
        }

        private static string Hex(byte[] hash)
        {
            return Convert.ToHexString(hash).ToLowerInvariant();
        }
    }
}
