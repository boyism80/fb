namespace Db
{
    public class GroupException : Exception
    {
        public uint Error { get; private set; }

        public GroupException(uint error)
        {
            Error = error;
        }
    }
}