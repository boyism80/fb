namespace Db
{
    public class ChangePasswordException : Exception
    {
        public required uint Error { get; set; }
    }
}