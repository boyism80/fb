using Fb.Model.EnumValue;
using Microsoft.AspNetCore.Authorization;
using System.Security.Claims;

namespace AdminTool.Authorization
{
    public sealed class MinimumRoleRequirement : IAuthorizationRequirement
    {
        public MinimumRoleRequirement(Role minimumRole)
        {
            MinimumRole = minimumRole;
        }

        public Role MinimumRole { get; }
    }

    public sealed class MinimumRoleAuthorizationHandler : AuthorizationHandler<MinimumRoleRequirement>
    {
        protected override Task HandleRequirementAsync(AuthorizationHandlerContext context, MinimumRoleRequirement requirement)
        {
            var roleClaim = context.User.FindFirst(ClaimTypes.Role);
            if (roleClaim == null)
                return Task.CompletedTask;

            if (!TryParseRole(roleClaim.Value, out var currentRole))
                return Task.CompletedTask;

            if (currentRole >= requirement.MinimumRole)
                context.Succeed(requirement);

            return Task.CompletedTask;
        }

        private static bool TryParseRole(string value, out Role role)
        {
            if (Enum.TryParse(value, true, out role))
                return true;

            if (value.Equals("Operator", StringComparison.OrdinalIgnoreCase))
            {
                role = Role.Moderator;
                return true;
            }

            role = Role.User;
            return false;
        }
    }
}
