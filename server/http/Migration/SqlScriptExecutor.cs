using System.Security.Cryptography;
using System.Text;
using System.Text.RegularExpressions;

namespace Http.Migration
{
    public static class MigrationChecksum
    {
        public static string Compute(string sql)
        {
            var normalized = sql.Replace("\r\n", "\n").Trim();
            var bytes = SHA256.HashData(Encoding.UTF8.GetBytes(normalized));
            return Convert.ToHexString(bytes).ToLowerInvariant();
        }
    }

    public static partial class SqlScriptExecutor
    {
        private const string TransactionOffDirective = "-- @transaction off";

        public static bool UsesTransaction(string sql)
        {
            var firstLine = sql
                .Replace("\r\n", "\n")
                .Split('\n', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries)
                .FirstOrDefault();

            return !string.Equals(firstLine, TransactionOffDirective, StringComparison.OrdinalIgnoreCase);
        }

        public static IReadOnlyList<string> SplitStatements(string sql)
        {
            var statements = new List<string>();
            var builder = new StringBuilder();
            var delimiter = ";";

            var normalized = sql.Replace("\r\n", "\n").Replace("\r", "\n");
            var length = normalized.Length;
            var atLineStart = true;

            for (var i = 0; i < length; i++)
            {
                var current = normalized[i];

                // DELIMITER directive must be detected at the beginning of a line.
                if (atLineStart && IsBlank(builder)
                    && TryReadDelimiterDirective(normalized, i, out var newDelimiter, out var nextIndex))
                {
                    delimiter = newDelimiter;
                    builder.Clear();
                    i = nextIndex;
                    atLineStart = true;
                    continue;
                }

                // Line comments: -- (followed by whitespace/EOL) or #
                if (IsLineCommentStart(normalized, i))
                {
                    var newlineIndex = normalized.IndexOf('\n', i);
                    if (newlineIndex < 0)
                    {
                        break;
                    }

                    builder.Append(normalized, i, newlineIndex - i);
                    i = newlineIndex - 1;
                    atLineStart = false;
                    continue;
                }

                // Block comments: /* ... */
                if (current == '/' && i + 1 < length && normalized[i + 1] == '*')
                {
                    var end = normalized.IndexOf("*/", i + 2, StringComparison.Ordinal);
                    if (end < 0)
                    {
                        break;
                    }

                    builder.Append(normalized, i, end + 2 - i);
                    i = end + 1;
                    atLineStart = false;
                    continue;
                }

                // Quoted strings and identifiers: ', ", `
                if (current == '\'' || current == '"' || current == '`')
                {
                    var quoteEnd = ReadQuoted(normalized, i, current);
                    builder.Append(normalized, i, quoteEnd - i + 1);
                    i = quoteEnd;
                    atLineStart = false;
                    continue;
                }

                // Delimiter match outside of quotes/comments.
                if (MatchesAt(normalized, i, delimiter))
                {
                    var statement = builder.ToString().Trim();
                    if (statement.Length > 0)
                    {
                        statements.Add(statement);
                    }

                    builder.Clear();
                    i += delimiter.Length - 1;
                    atLineStart = false;
                    continue;
                }

                builder.Append(current);
                atLineStart = current == '\n';
            }

            var tail = builder.ToString().Trim();
            if (tail.Length > 0)
            {
                statements.Add(tail);
            }

            return statements;
        }

        private static bool IsBlank(StringBuilder builder)
        {
            for (var i = 0; i < builder.Length; i++)
            {
                if (!char.IsWhiteSpace(builder[i]))
                {
                    return false;
                }
            }

            return true;
        }

        private static bool TryReadDelimiterDirective(string sql, int index, out string delimiter, out int nextIndex)
        {
            delimiter = ";";
            nextIndex = index;

            const string keyword = "DELIMITER";
            if (index + keyword.Length >= sql.Length)
            {
                return false;
            }

            if (string.Compare(sql, index, keyword, 0, keyword.Length, StringComparison.OrdinalIgnoreCase) != 0)
            {
                return false;
            }

            var cursor = index + keyword.Length;
            if (cursor >= sql.Length || (sql[cursor] != ' ' && sql[cursor] != '\t'))
            {
                return false;
            }

            while (cursor < sql.Length && (sql[cursor] == ' ' || sql[cursor] == '\t'))
            {
                cursor++;
            }

            var start = cursor;
            while (cursor < sql.Length && sql[cursor] != '\n' && sql[cursor] != ' ' && sql[cursor] != '\t')
            {
                cursor++;
            }

            if (cursor == start)
            {
                return false;
            }

            delimiter = sql[start..cursor];

            var lineEnd = sql.IndexOf('\n', cursor);
            nextIndex = lineEnd < 0 ? sql.Length - 1 : lineEnd;
            return true;
        }

        private static bool IsLineCommentStart(string sql, int index)
        {
            var current = sql[index];
            if (current == '#')
            {
                return true;
            }

            if (current == '-' && index + 1 < sql.Length && sql[index + 1] == '-')
            {
                var afterIndex = index + 2;
                if (afterIndex >= sql.Length)
                {
                    return true;
                }

                var after = sql[afterIndex];
                return after == ' ' || after == '\t' || after == '\n';
            }

            return false;
        }

        private static int ReadQuoted(string sql, int index, char quote)
        {
            for (var i = index + 1; i < sql.Length; i++)
            {
                var current = sql[i];
                if (current == '\\' && quote != '`')
                {
                    i++;
                    continue;
                }

                if (current == quote)
                {
                    if (i + 1 < sql.Length && sql[i + 1] == quote)
                    {
                        i++;
                        continue;
                    }

                    return i;
                }
            }

            return sql.Length - 1;
        }

        private static bool MatchesAt(string sql, int index, string token)
        {
            if (index + token.Length > sql.Length)
            {
                return false;
            }

            return string.CompareOrdinal(sql, index, token, 0, token.Length) == 0;
        }

        public static string ParseVersion(string fileName)
        {
            var match = VersionPattern().Match(fileName);
            if (!match.Success)
            {
                throw new InvalidOperationException($"Migration file name must start with yyyyMMddHHmmss: {fileName}");
            }

            return match.Groups[1].Value;
        }

        public static string ParseName(string fileName)
        {
            var version = ParseVersion(fileName);
            var stem = Path.GetFileNameWithoutExtension(fileName);
            if (stem.Length <= version.Length + 1)
            {
                return stem;
            }

            return stem[(version.Length + 1)..];
        }

        [GeneratedRegex("^(\\d{14})_")]
        private static partial Regex VersionPattern();
    }
}
