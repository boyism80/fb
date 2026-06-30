using Microsoft.AspNetCore.Components;
using Microsoft.AspNetCore.WebUtilities;

namespace AdminTool.Services
{
    public static class UserSearchNavigation
    {
        public static async Task NavigateFromSearchAsync(
            NavigationManager navigation,
            UserService userService,
            uint? world,
            string searchTerm)
        {
            if (string.IsNullOrWhiteSpace(searchTerm))
                return;

            if (world.HasValue && world.Value > 0)
            {
                var userId = await userService.TryResolveUserIdAsync(world.Value, searchTerm);
                if (userId.HasValue)
                {
                    navigation.NavigateTo($"/users/{userId.Value}");
                    return;
                }
            }

            var query = new Dictionary<string, string> { ["search"] = searchTerm.Trim() };
            if (world.HasValue && world.Value > 0)
                query["world"] = world.Value.ToString();

            navigation.NavigateTo(QueryHelpers.AddQueryString("/users", query));
        }
    }
}
