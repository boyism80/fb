using Dapper;
using Newtonsoft.Json;
using System.Data;

namespace Http.Extension
{
    /// <summary>
    /// Provides extension methods for MySQL database operations and value formatting.
    /// Contains utility methods for escaping values for safe SQL query construction.
    /// </summary>
    public static class MySql
    {
        /// <summary>
        /// Escapes an object value for safe inclusion in MySQL SQL queries.
        /// Handles null values, strings, booleans, DateTime objects, and other types appropriately.
        /// Escapes special characters in strings including single quotes, backslashes, and control characters.
        /// </summary>
        /// <typeparam name="T">The type of the object to escape.</typeparam>
        /// <param name="obj">The object value to escape for SQL usage.</param>
        /// <returns>A properly escaped string representation suitable for MySQL queries.</returns>
        public static string Escape<T>(this T obj)
        {
            if (obj == null)
                return "NULL";

            return obj switch
            {
                string s => EscapeStringValue(s),
                bool b => b ? "1" : "0",
                DateTime dt => $"'{dt:yyyy-MM-dd HH:mm:ss.ffffff}'",
                _ => obj.ToString(),
            };
        }

        /// <summary>
        /// Escapes a string value for MySQL query usage.
        /// Escapes special characters in strings for safe SQL query construction.
        /// </summary>
        /// <param name="value">The string value to escape.</param>
        /// <returns>An escaped string representation suitable for MySQL queries.</returns>
        private static string EscapeStringValue(string value)
        {
            if (value == null)
                return "NULL";

            return $"'{EscapeString(value)}'";
        }

        /// <summary>
        /// Escapes special characters in a string for safe MySQL query usage.
        /// Replaces single quotes with doubled quotes, escapes backslashes, and handles control characters.
        /// </summary>
        /// <param name="value">The string value to escape.</param>
        /// <returns>The escaped string safe for MySQL queries.</returns>
        private static string EscapeString(string value)
        {
            if (string.IsNullOrEmpty(value))
                return value;

            return value
                .Replace("\\", "\\\\")  // Escape backslashes first
                .Replace("'", "''")     // Escape single quotes
                .Replace("\0", "\\0")   // Escape NULL character
                .Replace("\n", "\\n")   // Escape newline
                .Replace("\r", "\\r")   // Escape carriage return
                .Replace("\x1a", "\\x1a"); // Escape Ctrl+Z
        }
    }

    /// <summary>
    /// Provides JSON serialization and deserialization support for Dapper ORM.
    /// Handles automatic conversion between .NET objects and JSON strings in database operations.
    /// </summary>
    public class JsonTypeHandler : SqlMapper.ITypeHandler
    {
        /// <summary>
        /// Sets the parameter value by serializing the object to JSON string.
        /// </summary>
        /// <param name="parameter">The database parameter to set the value for.</param>
        /// <param name="value">The object value to serialize and set as parameter value.</param>
        public void SetValue(IDbDataParameter parameter, object value)
        {
            parameter.Value = JsonConvert.SerializeObject(value);
        }

        /// <summary>
        /// Parses a JSON string from the database back to the specified .NET type.
        /// </summary>
        /// <param name="destinationType">The target .NET type to deserialize to.</param>
        /// <param name="value">The JSON string value from the database.</param>
        /// <returns>The deserialized object of the specified destination type.</returns>
        public object Parse(Type destinationType, object value)
        {
            return JsonConvert.DeserializeObject(value as string, destinationType);
        }
    }
}
