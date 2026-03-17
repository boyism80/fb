using Fb.Model;

namespace Marketplace.Extension;

public static class ItemTableExtension
{
    private static readonly Lazy<SortedDictionary<string, uint>> _sortedItems = new Lazy<SortedDictionary<string, uint>>(
        () =>
        {
            var dict = new SortedDictionary<string, uint>();
            foreach (var item in Table.Item.Values)
            {
                if (item != null && !string.IsNullOrEmpty(item.Name))
                {
                    var nameLower = item.Name.ToLower();
                    dict[nameLower] = item.Id;
                }
            }
            return dict;
        },
        System.Threading.LazyThreadSafetyMode.ExecutionAndPublication);

    public static List<uint> NameToItemModelIds(this Table.ItemTable itemTable, string itemNamePrefix)
    {
        var sortedItems = _sortedItems.Value;

        if (string.IsNullOrEmpty(itemNamePrefix))
        {
            // Empty prefix = return all item model IDs
            return sortedItems.Values.ToList();
        }

        var result = new List<uint>();
        var prefixLower = itemNamePrefix.ToLower();

        // Use binary search for efficient prefix matching (similar to C++ lower_bound)
        var keys = sortedItems.Keys.ToList();
        var startIndex = keys.BinarySearch(prefixLower);

        // If exact match not found, BinarySearch returns bitwise complement of insertion point
        if (startIndex < 0)
        {
            startIndex = ~startIndex;
        }

        // Find all items that start with prefix
        for (int i = startIndex; i < keys.Count; i++)
        {
            var key = keys[i];
            if (key.Length < prefixLower.Length)
                continue;

            if (key.Substring(0, prefixLower.Length) != prefixLower)
                break;

            result.Add(sortedItems[key]);
        }

        return result;
    }
}
