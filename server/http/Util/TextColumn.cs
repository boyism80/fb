using System.Text;

namespace Http.Util
{
    public static class TextColumn
    {
        private static readonly Lazy<Encoding> EucKr = new(() =>
        {
            Encoding.RegisterProvider(CodePagesEncodingProvider.Instance);
            return Encoding.GetEncoding(51949, EncoderFallback.ExceptionFallback, DecoderFallback.ExceptionFallback);
        });

        public static bool FitsUtf8mb3(string value, int maxLength)
        {
            if (value == null)
            {
                return true;
            }

            return value.Length <= maxLength && value.Any(char.IsSurrogate) == false;
        }

        public static bool FitsEucKr(string value, int maxLength)
        {
            if (value == null)
            {
                return true;
            }

            if (value.Length > maxLength)
            {
                return false;
            }

            try
            {
                EucKr.Value.GetByteCount(value);
                return true;
            }
            catch (EncoderFallbackException)
            {
                return false;
            }
        }
    }
}
