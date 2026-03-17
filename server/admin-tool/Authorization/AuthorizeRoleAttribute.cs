using Fb.Model.EnumValue;
using Microsoft.AspNetCore.Authorization;

namespace AdminTool.Authorization
{
    public sealed class AuthorizeRoleAttribute : AuthorizeAttribute
    {
        public AuthorizeRoleAttribute(Role minimumRole)
        {
            Policy = $"{MinimumRolePolicyProvider.PolicyPrefix}{minimumRole}";
        }
    }
}
