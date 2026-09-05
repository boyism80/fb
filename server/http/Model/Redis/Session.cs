namespace Http.Model.Redis
{
    public class Session
    {
        public uint Uid { get; set; }
        public uint Host { get; set; }
        public string Role { get; set; } = "home";
    }
}
