using Fb.Model.EnumValue;
using Microsoft.AspNetCore.Authorization;
using Microsoft.Extensions.Options;

namespace AdminTool.Authorization
{
    public sealed class MinimumRolePolicyProvider : IAuthorizationPolicyProvider
    {
        public const string PolicyPrefix = "RoleMinimum:";

        private readonly DefaultAuthorizationPolicyProvider _fallbackProvider;

        public MinimumRolePolicyProvider(IOptions<AuthorizationOptions> options)
        {
            _fallbackProvider = new DefaultAuthorizationPolicyProvider(options);
        }

        public Task<AuthorizationPolicy> GetPolicyAsync(string policyName)
        {
            if (policyName.StartsWith(PolicyPrefix, StringComparison.OrdinalIgnoreCase))
            {
                var roleName = policyName.Substring(PolicyPrefix.Length);
                if (Enum.TryParse(roleName, true, out Role minimumRole))
                {
                    var policy = new AuthorizationPolicyBuilder()
                        .AddRequirements(new MinimumRoleRequirement(minimumRole))
                        .Build();
                    return Task.FromResult<AuthorizationPolicy>(policy);
                }
            }

            return _fallbackProvider.GetPolicyAsync(policyName);
        }

        public Task<AuthorizationPolicy> GetDefaultPolicyAsync() => _fallbackProvider.GetDefaultPolicyAsync();

        public Task<AuthorizationPolicy> GetFallbackPolicyAsync() => _fallbackProvider.GetFallbackPolicyAsync();
    }
}
