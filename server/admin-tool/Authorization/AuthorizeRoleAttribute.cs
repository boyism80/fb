using Fb.Model.EnumValue;
using Microsoft.AspNetCore.Authorization;

namespace AdminTool.Authorization
{
    /// <summary>
    /// Authorize attribute that evaluates whether the current user has at least the provided role.
    /// </summary>
    public sealed class AuthorizeRoleAttribute : AuthorizeAttribute
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="AuthorizeRoleAttribute"/> class.
        /// </summary>
        /// <param name="minimumRole">The minimum role required to access the resource.</param>
        public AuthorizeRoleAttribute(Role minimumRole)
        {
            Policy = $"{MinimumRolePolicyProvider.PolicyPrefix}{minimumRole}";
        }
    }
}

