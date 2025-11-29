using Microsoft.AspNetCore.Components.Authorization;
using Microsoft.AspNetCore.Components.Server.ProtectedBrowserStorage;
using System.Security.Claims;

namespace AdminTool.Authentication
{
    /// <summary>
    /// Provides custom authentication state management for the admin tool.
    /// Manages user authentication state using protected browser storage.
    /// </summary>
    public class CustomAuthenticationStateProvider : AuthenticationStateProvider
    {
        private readonly ProtectedSessionStorage _sessionStorage;
        private const string AuthKey = "AdminAuth";

        /// <summary>
        /// Initializes a new instance of the <see cref="CustomAuthenticationStateProvider"/> class.
        /// </summary>
        /// <param name="sessionStorage">The protected session storage for storing authentication data.</param>
        public CustomAuthenticationStateProvider(ProtectedSessionStorage sessionStorage)
        {
            _sessionStorage = sessionStorage;
        }

        /// <summary>
        /// Gets the current authentication state asynchronously.
        /// </summary>
        /// <returns>The current authentication state.</returns>
        public override async Task<AuthenticationState> GetAuthenticationStateAsync()
        {
            try
            {
                var result = await _sessionStorage.GetAsync<AuthData>(AuthKey);
                if (!result.Success || result.Value == null)
                {
                    return new AuthenticationState(new ClaimsPrincipal(new ClaimsIdentity()));
                }

                var authData = result.Value;
                var claims = new List<Claim>
                {
                    new Claim(ClaimTypes.Name, authData.UserName),
                    new Claim(ClaimTypes.NameIdentifier, authData.UserId.ToString()),
                    new Claim(ClaimTypes.Role, authData.Role.ToString())
                };

                var identity = new ClaimsIdentity(claims, "admin");
                var user = new ClaimsPrincipal(identity);

                return new AuthenticationState(user);
            }
            catch
            {
                return new AuthenticationState(new ClaimsPrincipal(new ClaimsIdentity()));
            }
        }

        /// <summary>
        /// Marks the user as authenticated and stores authentication data.
        /// </summary>
        /// <param name="authData">The authentication data to store.</param>
        public async Task MarkUserAsAuthenticated(AuthData authData)
        {
            await _sessionStorage.SetAsync(AuthKey, authData);
            NotifyAuthenticationStateChanged(GetAuthenticationStateAsync());
        }

        /// <summary>
        /// Sets the authentication state with user name and role.
        /// </summary>
        /// <param name="userName">The user name.</param>
        /// <param name="role">The user role.</param>
        public async Task SetAuthenticationStateAsync(string userName, Fb.Model.EnumValue.Role role)
        {
            var authData = new AuthData
            {
                UserName = userName,
                Role = role.ToString()
            };
            await MarkUserAsAuthenticated(authData);
        }

        /// <summary>
        /// Marks the user as logged out and clears authentication data.
        /// </summary>
        public async Task MarkUserAsLoggedOut()
        {
            await _sessionStorage.DeleteAsync(AuthKey);
            NotifyAuthenticationStateChanged(GetAuthenticationStateAsync());
        }
    }

    /// <summary>
    /// Represents authentication data stored in session storage.
    /// </summary>
    public class AuthData
    {
        /// <summary>
        /// Gets or sets the user ID.
        /// </summary>
        public uint UserId { get; set; }

        /// <summary>
        /// Gets or sets the user name.
        /// </summary>
        public string UserName { get; set; } = string.Empty;

        /// <summary>
        /// Gets or sets the user role.
        /// </summary>
        public string Role { get; set; } = string.Empty;
    }
}

