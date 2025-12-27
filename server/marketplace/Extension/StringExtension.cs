namespace Marketplace.Extension;

/// <summary>
/// Provides extension methods for string formatting operations.
/// </summary>
public static class StringExtension
{
    /// <summary>
    /// Converts C++ format string (using {}) to C# format string (using {0}, {1}, etc.).
    /// </summary>
    /// <param name="format">The C++ format string with {} placeholders.</param>
    /// <returns>The C# format string with {0}, {1}, etc. placeholders.</returns>
    public static string ToCSharpFormat(this string format)
    {
        if (string.IsNullOrEmpty(format))
            return format;

        var result = new System.Text.StringBuilder(format.Length);
        var index = 0;
        var i = 0;

        while (i < format.Length)
        {
            if (format[i] == '{' && i + 1 < format.Length && format[i + 1] == '}')
            {
                // Found {} - replace with {index}
                result.Append('{');
                result.Append(index);
                result.Append('}');
                index++;
                i += 2; // Skip both { and }
            }
            else
            {
                result.Append(format[i]);
                i++;
            }
        }

        return result.ToString();
    }
}
