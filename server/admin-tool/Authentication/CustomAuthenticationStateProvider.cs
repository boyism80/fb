using Microsoft.AspNetCore.Components.Authorization;
using Microsoft.AspNetCore.Components.Server.ProtectedBrowserStorage;
using System.Security.Claims;

namespace AdminTool.Authentication
{
    public class CustomAuthenticationStateProvider : AuthenticationStateProvider
    {
        private readonly ProtectedSessionStorage _sessionStorage;
        private const string AuthKey = "AdminAuth";

        public CustomAuthenticationStateProvider(ProtectedSessionStorage sessionStorage)
        {
            _sessionStorage = sessionStorage;
        }

        public const string LoginWorldClaimType = "LoginWorld";

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
                if (authData.LoginWorld.HasValue)
                {
                    claims.Add(new Claim(LoginWorldClaimType, authData.LoginWorld.Value.ToString()));
                }

                var identity = new ClaimsIdentity(claims, "admin");
                var user = new ClaimsPrincipal(identity);

                return new AuthenticationState(user);
            }
            catch
            {
                return new AuthenticationState(new ClaimsPrincipal(new ClaimsIdentity()));
            }
        }

        public async Task MarkUserAsAuthenticated(AuthData authData)
        {
            await _sessionStorage.SetAsync(AuthKey, authData);
            NotifyAuthenticationStateChanged(GetAuthenticationStateAsync());
        }

        public async Task SetAuthenticationStateAsync(string userName, Fb.Model.EnumValue.Role role)
        {
            var authData = new AuthData
            {
                UserName = userName,
                Role = role.ToString()
            };
            await MarkUserAsAuthenticated(authData);
        }

        public async Task MarkUserAsLoggedOut()
        {
            await _sessionStorage.DeleteAsync(AuthKey);
            NotifyAuthenticationStateChanged(GetAuthenticationStateAsync());
        }
    }

    public class AuthData
    {
        public uint UserId { get; set; }

        public string UserName { get; set; } = string.Empty;

        public string Role { get; set; } = string.Empty;

        public uint? LoginWorld { get; set; }
    }
}
