using Dapper;
using Newtonsoft.Json;
using System.Data;

namespace Http.Extension
{
    public static class MySql
    {
        public static string Escape<T>(this T obj)
        {
            if (obj == null)
                return "NULL";

            // Handle enum types by converting to their underlying integer value
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
                // Fallback to int conversion
                return Convert.ToInt32(enumValue).ToString();
            }

            return obj switch
            {
                string s => s == null ? "NULL" : $"'{s.Replace("'", "''").Replace("\\", "\\\\")}'",
                bool b => b ? "1" : "0",
                DateTime dt => $"'{dt:yyyy-MM-dd HH:mm:ss.ffffff}'",
                _ => obj.ToString(),
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