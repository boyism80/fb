using System.Drawing;

namespace http.Util
{
    public static class Endian
    {
        public static int ToMachineEndian(this int value)
        {
            if (BitConverter.IsLittleEndian)
            {
                var x = BitConverter.GetBytes((uint)value);
                Array.Reverse(x);
                return BitConverter.ToInt32(x);
            }
            else
            {
                return value;
            }
        }

        public static T ToMachineEndian<T>(this T value) where T : Enum
        {
            return (T)(object)((int)(object)value).ToMachineEndian();
        }
    }
}
