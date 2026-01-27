namespace Http.Service
{
    public class BulletinWriteRequest
    {
        public uint Section { get; set; } // Bulletin category (DO NOT CHANGE - this is bulletin board category)
        public uint World { get; set; } // Game world identifier (changed from GameSection string to GameWorld int)
        public uint User { get; set; }
        public string Title { get; set; } = string.Empty;
        public string Contents { get; set; } = string.Empty;
        public TaskCompletionSource<bool> CompletionSource { get; set; } = new TaskCompletionSource<bool>();
    }
}

