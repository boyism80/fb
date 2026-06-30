using AdminTool.Options;
using Microsoft.Extensions.Options;

namespace AdminTool.Services
{
    public class SecurityService
    {
        private readonly string _elevationSecret;

        public SecurityService(IOptions<SecurityOptions> options)
        {
            _elevationSecret = options.Value?.ElevationSecret ?? string.Empty;
        }

        public bool IsValidElevationSecret(string secret)
        {
            if (string.IsNullOrEmpty(_elevationSecret))
                return false;

            return string.Equals(secret, _elevationSecret, StringComparison.Ordinal);
        }
    }
}
