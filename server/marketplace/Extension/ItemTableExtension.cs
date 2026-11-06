using Fb.Model;
using System.Runtime.CompilerServices;

namespace Marketplace.Extension;

public static class ItemTableExtension
{
    private static readonly ConditionalWeakTable<Table.ItemTable, NameIndex> _indexes = new();

    private sealed class NameIndex
    {
        public readonly List<string> Keys;
        public readonly List<uint> Ids;

        public NameIndex(Table.ItemTable itemTable)
        {
            var dict = new SortedDictionary<string, uint>(StringComparer.Ordinal);
            foreach (var item in itemTable.Values)
            {
                if (item != null && !string.IsNullOrEmpty(item.Name))
                {
                    var nameLower = item.Name.ToLower();
                    dict[nameLower] = item.Id;
                }
            }
            Keys = dict.Keys.ToList();
            Ids = dict.Values.ToList();
        }
    }

    public static List<uint> NameToItemModelIds(this Table.ItemTable itemTable, string itemNamePrefix)
    {
        var index = _indexes.GetValue(itemTable, table => new NameIndex(table));

        if (string.IsNullOrEmpty(itemNamePrefix))
        {
            // Empty prefix = return all item model IDs
            return index.Ids.ToList();
        }

        var result = new List<uint>();
        var prefixLower = itemNamePrefix.ToLower();

        // Use binary search for efficient prefix matching (similar to C++ lower_bound)
        var startIndex = index.Keys.BinarySearch(prefixLower, StringComparer.Ordinal);

        // If exact match not found, BinarySearch returns bitwise complement of insertion point
        if (startIndex < 0)
        {
            startIndex = ~startIndex;
        }

        // Find all items that start with prefix
        for (int i = startIndex; i < index.Keys.Count; i++)
        {
            var key = index.Keys[i];
            if (key.StartsWith(prefixLower, StringComparison.Ordinal) == false)
            {
                break;
            }

            result.Add(index.Ids[i]);
        }

        return result;
    }
}
