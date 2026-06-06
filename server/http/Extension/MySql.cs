using Dapper;
using Newtonsoft.Json;
using System.Data;

namespace Http.Extension
{
    public static class MySql
    {
        public static string Escape<T>(this T? obj) where T : struct
        {
            if (!obj.HasValue)
                return "NULL";

            return EscapeNonNull(obj.Value);
        }

        public static string Escape(this string? obj)
        {
            if (obj is null)
                return "NULL";

            return EscapeNonNull(obj);
        }

        public static string Escape<T>(this T obj) where T : notnull
        {
            if (obj is null)
                return "NULL";

            return EscapeNonNull(obj);
        }

        private static string EscapeNonNull<T>(T obj) where T : notnull
        {
            if (obj is System.Enum enumValue)
            {
                var underlyingType = System.Enum.GetUnderlyingType(enumValue.GetType());
                if (underlyingType == typeof(byte))
                    return ((byte)(object)enumValue).ToString();
                if (underlyingType == typeof(sbyte))
                    return ((sbyte)(object)enumValue).ToString();
                if (underlyingType == typeof(short))
                    return ((short)(object)enumValue).ToString();
                if (underlyingType == typeof(ushort))
                    return ((ushort)(object)enumValue).ToString();
                if (underlyingType == typeof(int))
                    return ((int)(object)enumValue).ToString();
                if (underlyingType == typeof(uint))
                    return ((uint)(object)enumValue).ToString();
                if (underlyingType == typeof(long))
                    return ((long)(object)enumValue).ToString();
                if (underlyingType == typeof(ulong))
                    return ((ulong)(object)enumValue).ToString();
                return Convert.ToInt32(enumValue).ToString();
            }

            return obj switch
            {
                string s => $"'{s.Replace("'", "''").Replace("\\", "\\\\")}'",
                bool b => b ? "1" : "0",
                byte v => v.ToString(),
                sbyte v => v.ToString(),
                short v => v.ToString(),
                ushort v => v.ToString(),
                int v => v.ToString(),
                uint v => v.ToString(),
                long v => v.ToString(),
                ulong v => v.ToString(),
                float v => v.ToString(),
                double v => v.ToString(),
                decimal v => v.ToString(),
                char c => $"'{c.ToString().Replace("'", "''").Replace("\\", "\\\\")}'",
                DateTime dt => $"'{dt:yyyy-MM-dd HH:mm:ss.ffffff}'",
                _ => EscapeNonNull(JsonConvert.SerializeObject(obj)),
            };
        }
    }

    public class JsonTypeHandler : SqlMapper.ITypeHandler
    {
        public void SetValue(IDbDataParameter parameter, object value)
        {
            parameter.Value = JsonConvert.SerializeObject(value);
        }

        public object Parse(Type destinationType, object value)
        {
            return JsonConvert.DeserializeObject(value as string, destinationType);
        }
    }
}