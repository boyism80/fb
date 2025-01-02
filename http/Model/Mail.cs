namespace Http.Model
{
    public class Mail : IModel
    {
        public uint Id { get; set; }
        public uint User { get; set; }
        public uint Sender {get;set;}
        public string SenderName { get; set; }
        public string Title { get; set; }
        public string Contents { get; set; }
        public bool Read {get;set;}
        public DateTime CreatedDate { get; set; }
        public DateTime UpdatedDate { get; set; }
        public bool Deleted { get; set; }
    }
}
