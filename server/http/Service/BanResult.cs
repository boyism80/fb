using Fb.Model.EnumValue;

namespace Http.Service
{
    public class BanResult
    {
        public bool Success { get; set; }

        public ErrorCode Error { get; set; }

        public uint? UserId { get; set; }

        public DateTime? ExpireDate { get; set; }
    }

    public class UnbanResult
    {
        public bool Success { get; set; }

        public ErrorCode Error { get; set; }

        public uint? UserId { get; set; }
    }

    public class BanCheckResult
    {
        public bool IsBanned { get; set; }

        public string Reason { get; set; }

        public DateTime? ExpireDate { get; set; }
    }
}
