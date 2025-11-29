using Fb.Model.EnumValue;

namespace Http.Service
{
    /// <summary>
    /// Represents the result of a ban operation.
    /// </summary>
    public class BanResult
    {
        /// <summary>
        /// Gets or sets a value indicating whether the operation was successful.
        /// </summary>
        public bool Success { get; set; }

        /// <summary>
        /// Gets or sets the error code if the operation failed.
        /// </summary>
        public ErrorCode Error { get; set; }

        /// <summary>
        /// Gets or sets the user ID of the banned user.
        /// </summary>
        public uint? UserId { get; set; }

        /// <summary>
        /// Gets or sets the ban expiration date. Null for permanent bans.
        /// </summary>
        public DateTime? ExpireDate { get; set; }
    }

    /// <summary>
    /// Represents the result of an unban operation.
    /// </summary>
    public class UnbanResult
    {
        /// <summary>
        /// Gets or sets a value indicating whether the operation was successful.
        /// </summary>
        public bool Success { get; set; }

        /// <summary>
        /// Gets or sets the error code if the operation failed.
        /// </summary>
        public ErrorCode Error { get; set; }

        /// <summary>
        /// Gets or sets the user ID of the unbanned user.
        /// </summary>
        public uint? UserId { get; set; }
    }
}

