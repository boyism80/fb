using Fb.Model.EnumValue;
using Http.Service;
using Microsoft.AspNetCore.Components.Authorization;
using Microsoft.AspNetCore.Components.Server.ProtectedBrowserStorage;
using System.Security.Claims;

namespace AdminTool.Authentication
{
    public class CustomAuthenticationStateProvider : AuthenticationStateProvider, IDisposable
    {
        private static readonly TimeSpan RevalidationInterval = TimeSpan.FromMinutes(1);
        private readonly ProtectedSessionStorage _sessionStorage;
        private readonly IServiceScopeFactory _scopeFactory;
        private readonly CancellationTokenSource _revalidationCancel = new();
        private const string AuthKey = "AdminAuth";
        private AuthData _authData;
        private string _role;

        public CustomAuthenticationStateProvider(ProtectedSessionStorage sessionStorage, IServiceScopeFactory scopeFactory)
        {
            _sessionStorage = sessionStorage;
            _scopeFactory = scopeFactory;
            _ = RevalidationLoop();
        }

        public const string LoginWorldClaimType = "LoginWorld";

        public override async Task<AuthenticationState> GetAuthenticationStateAsync()
        {
            try
            {
                var result = await _sessionStorage.GetAsync<AuthData>(AuthKey);
                _authData = result.Success ? result.Value : null;
            }
            catch
            {
                return Anonymous();
            }

            var state = await Validate(_authData);
            _role = state.User.FindFirst(ClaimTypes.Role)?.Value;
            return state;
        }

        public async Task MarkUserAsAuthenticated(AuthData authData)
        {
            await _sessionStorage.SetAsync(AuthKey, authData);
            NotifyAuthenticationStateChanged(GetAuthenticationStateAsync());
        }

        public async Task MarkUserAsLoggedOut()
        {
            await _sessionStorage.DeleteAsync(AuthKey);
            NotifyAuthenticationStateChanged(GetAuthenticationStateAsync());
        }

        public void Dispose()
        {
            _revalidationCancel.Cancel();
            _revalidationCancel.Dispose();
        }

        private async Task RevalidationLoop()
        {
            using var timer = new PeriodicTimer(RevalidationInterval);
            try
            {
                while (await timer.WaitForNextTickAsync(_revalidationCancel.Token))
                {
                    if (_authData == null)
                    {
                        continue;
                    }

                    var state = await Validate(_authData);
                    var role = state.User.FindFirst(ClaimTypes.Role)?.Value;
                    if (role != _role)
                    {
                        _role = role;
                        NotifyAuthenticationStateChanged(Task.FromResult(state));
                    }
                }
            }
            catch (OperationCanceledException)
            {
            }
        }

        private async Task<AuthenticationState> Validate(AuthData authData)
        {
            if (authData == null || !authData.LoginWorld.HasValue)
            {
                return Anonymous();
            }

            try
            {
                var world = authData.LoginWorld.Value;
                await using var scope = _scopeFactory.CreateAsyncScope();
                var dbContext = scope.ServiceProvider.GetRequiredService<DbContext>();
                var character = await dbContext.Character.Get(world, authData.UserId);
                if (character == null || character.Role < Role.Admin)
                {
                    return Anonymous();
                }

                var banService = scope.ServiceProvider.GetRequiredService<BanService>();
                if (await banService.IsBanned(world, character.Name) != null)
                {
                    return Anonymous();
                }

                var claims = new List<Claim>
                {
                    new Claim(ClaimTypes.Name, character.Name),
                    new Claim(ClaimTypes.NameIdentifier, authData.UserId.ToString()),
                    new Claim(ClaimTypes.Role, character.Role.ToString()),
                    new Claim(LoginWorldClaimType, world.ToString())
                };
                return new AuthenticationState(new ClaimsPrincipal(new ClaimsIdentity(claims, "admin")));
            }
            catch
            {
                return Anonymous();
            }
        }

        private static AuthenticationState Anonymous()
        {
            return new AuthenticationState(new ClaimsPrincipal(new ClaimsIdentity()));
        }
    }

    public class AuthData
    {
        public uint UserId { get; set; }

        public uint? LoginWorld { get; set; }
    }
}
