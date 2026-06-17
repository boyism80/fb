namespace Http.Model
{
    public class Bulletin : IModel
    {
        public uint Id { get; set; }
        public uint Section { get; set; }
        public uint User { get; set; }
        public string UserName { get; set; }
        public string Title { get; set; }
        public string Contents { get; set; }
        public DateTime CreatedDate { get; set; }
        public DateTime UpdatedDate { get; set; }
    }
}
