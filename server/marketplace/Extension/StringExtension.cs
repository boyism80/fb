namespace Marketplace.Extension;

public static class StringExtension
{
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