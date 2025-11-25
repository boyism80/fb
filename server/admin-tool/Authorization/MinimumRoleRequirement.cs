using Fb.Model.EnumValue;
using Microsoft.AspNetCore.Authorization;
using System.Security.Claims;

namespace AdminTool.Authorization
{
    /// <summary>
    /// Authorization requirement that enforces a minimum role.
    /// </summary>
    public sealed class MinimumRoleRequirement : IAuthorizationRequirement
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="MinimumRoleRequirement"/> class.
        /// </summary>
        /// <param name="minimumRole">Minimum role allowed to access the resource.</param>
        public MinimumRoleRequirement(Role minimumRole)
        {
            MinimumRole = minimumRole;
        }

        /// <summary>
        /// Gets the minimum role required.
        /// </summary>
        public Role MinimumRole { get; }
    }

    /// <summary>
    /// Handles <see cref="MinimumRoleRequirement"/> evaluations.
    /// </summary>
    public sealed class MinimumRoleAuthorizationHandler : AuthorizationHandler<MinimumRoleRequirement>
    {
        /// <inheritdoc/>
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

