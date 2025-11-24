using AdminTool.Options;
using Microsoft.Extensions.Options;

namespace AdminTool.Services
{
    /// <summary>
    /// Provides helper methods for validating elevation secrets that allow privileged operations.
    /// </summary>
    public class SecurityService
    {
        private readonly string _elevationSecret;

        /// <summary>
        /// Initializes a new instance of the <see cref="SecurityService"/> class.
        /// </summary>
        /// <param name="options">Security configuration options.</param>
        public SecurityService(IOptions<SecurityOptions> options)
        {
            _elevationSecret = options.Value?.ElevationSecret ?? string.Empty;
        }

        /// <summary>
        /// Determines whether the provided secret matches the configured elevation secret.
        /// </summary>
        /// <param name="secret">The secret provided by the user.</param>
        /// <returns><c>true</c> when the secret matches the configured secret; otherwise <c>false</c>.</returns>
        public bool IsValidElevationSecret(string? secret)
        {
            if (string.IsNullOrEmpty(_elevationSecret))
                return false;

            return string.Equals(secret, _elevationSecret, StringComparison.Ordinal);
        }
    }
}

