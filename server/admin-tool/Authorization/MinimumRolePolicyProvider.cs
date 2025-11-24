using Fb.Model.EnumValue;
using Microsoft.AspNetCore.Authorization;
using Microsoft.Extensions.Options;

namespace AdminTool.Authorization
{
    /// <summary>
    /// Authorization policy provider that builds minimum-role policies on demand.
    /// </summary>
    public sealed class MinimumRolePolicyProvider : IAuthorizationPolicyProvider
    {
        /// <summary>
        /// Prefix applied to minimum-role policy names.
        /// </summary>
        public const string PolicyPrefix = "RoleMinimum:";

        private readonly DefaultAuthorizationPolicyProvider _fallbackProvider;

        /// <summary>
        /// Initializes a new instance of the <see cref="MinimumRolePolicyProvider"/> class.
        /// </summary>
        /// <param name="options">Authorization options.</param>
        public MinimumRolePolicyProvider(IOptions<AuthorizationOptions> options)
        {
            _fallbackProvider = new DefaultAuthorizationPolicyProvider(options);
        }

        /// <inheritdoc/>
        public Task<AuthorizationPolicy?> GetPolicyAsync(string policyName)
        {
            if (policyName.StartsWith(PolicyPrefix, StringComparison.OrdinalIgnoreCase))
            {
                var roleName = policyName.Substring(PolicyPrefix.Length);
                if (Enum.TryParse(roleName, true, out Role minimumRole))
                {
                    var policy = new AuthorizationPolicyBuilder()
                        .AddRequirements(new MinimumRoleRequirement(minimumRole))
                        .Build();
                    return Task.FromResult<AuthorizationPolicy?>(policy);
                }
            }

            return _fallbackProvider.GetPolicyAsync(policyName);
        }

        /// <inheritdoc/>
        public Task<AuthorizationPolicy> GetDefaultPolicyAsync() => _fallbackProvider.GetDefaultPolicyAsync();

        /// <inheritdoc/>
        public Task<AuthorizationPolicy?> GetFallbackPolicyAsync() => _fallbackProvider.GetFallbackPolicyAsync();
    }
}

