namespace Http.Model
{
    public class MailWriteResult
    {
        public required Mail Mail { get; set; }
        public ushort Unread { get; set; }
    }
}
