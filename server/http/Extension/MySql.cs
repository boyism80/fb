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
        /// Converts UUID strings to BINARY(16) format using UNHEX, otherwise escapes as regular string.
        /// </summary>
        /// <param name="value">The string value to escape.</param>
        /// <returns>An escaped string representation suitable for MySQL queries.</returns>
        private static string EscapeStringValue(string value)
        {
            if (value == null)
                return "NULL";

            if (IsUuidString(value))
            {
                var escaped = EscapeString(value);
                return $"UNHEX(REPLACE('{escaped}', '-', ''))";
            }

            return $"'{EscapeString(value)}'";
        }

        /// <summary>
        /// Checks if a string is a valid UUID format.
        /// </summary>
        /// <param name="value">The string value to check.</param>
        /// <returns>True if the string matches UUID format; otherwise, false.</returns>
        private static bool IsUuidString(string value)
        {
            if (string.IsNullOrEmpty(value))
                return false;

            // UUID format: 550e8400-e29b-41d4-a716-446655440000 (36 characters with hyphens)
            return value.Length == 36 && Guid.TryParse(value, out _);
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

    /// <summary>
    /// Provides UUID string to BINARY(16) conversion support for Dapper ORM.
    /// Handles automatic conversion between UUID strings in application code and BINARY(16) in database.
    /// Only converts when the value is a byte array (from DB) or a valid UUID string (to DB).
    /// </summary>
    public class UuidStringTypeHandler : SqlMapper.ITypeHandler
    {
        /// <summary>
        /// Sets the parameter value by converting UUID string to BINARY(16) byte array.
        /// Only converts if the value is a valid UUID string format.
        /// </summary>
        /// <param name="parameter">The database parameter to set the value for.</param>
        /// <param name="value">The UUID string value to convert and set as parameter value.</param>
        public void SetValue(IDbDataParameter parameter, object value)
        {
            if (value == null || value == DBNull.Value)
            {
                parameter.Value = DBNull.Value;
                return;
            }

            var uuidString = value as string;
            if (string.IsNullOrEmpty(uuidString))
            {
                parameter.Value = DBNull.Value;
                return;
            }

            // Only convert if it's a valid UUID format
            if (Guid.TryParse(uuidString, out var guid))
            {
                parameter.Value = guid.ToByteArray();
                parameter.DbType = DbType.Binary;
                parameter.Size = 16;
            }
            else
            {
                // Not a UUID, pass through as string
                parameter.Value = uuidString;
            }
        }

        /// <summary>
        /// Parses a BINARY(16) byte array from the database back to UUID string.
        /// Only converts if the value is a byte array of length 16.
        /// </summary>
        /// <param name="destinationType">The target .NET type to deserialize to (should be string).</param>
        /// <param name="value">The value from the database (byte array for BINARY(16), string for other types).</param>
        /// <returns>The UUID string representation if byte array, otherwise the original value.</returns>
        public object Parse(Type destinationType, object value)
        {
            if (value == null || value == DBNull.Value)
                return null;

            // Only convert if it's a byte array (BINARY(16) from database)
            if (value is byte[] bytes && bytes.Length == 16)
            {
                return new Guid(bytes).ToString();
            }

            // Not a byte array, return as-is (might be string from other columns)
            return value;
        }
    }
}
