namespace Internal.Service
{
    public class BulletinWriteRequest
    {
        public uint Section { get; set; }
        public uint User { get; set; }
        public string Title { get; set; } = string.Empty;
        public string Contents { get; set; } = string.Empty;
        public TaskCompletionSource<bool> CompletionSource { get; set; } = new TaskCompletionSource<bool>();
    }
}

