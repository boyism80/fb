namespace Internal.Service
{
    public class BulletinDeleteRequest
    {
        public uint Section { get; set; }
        public uint Id { get; set; }
        public uint User { get; set; }
        public TaskCompletionSource<int> CompletionSource { get; set; } = new TaskCompletionSource<int>();
    }
}

