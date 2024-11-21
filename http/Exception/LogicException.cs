using Fb.Model.EnumValue;

namespace Http
{
    public class LogicException : Exception
    {
        public ErrorCode Error { get; private set; }

        public LogicException(ErrorCode error)
        {
            Error = error;
        }
    }
}