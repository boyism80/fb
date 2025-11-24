namespace Http.Model
{
    /// <summary>
    /// Represents a mail message model for the in-game mail system.
    /// Contains all information about mail messages sent between players.
    /// </summary>
    public class Mail : IModel
    {
        /// <summary>
        /// Gets or sets the unique identifier for this mail message.
        /// </summary>
        /// <value>The unique mail ID used for database operations.</value>
        public uint Id { get; set; }

        /// <summary>
        /// Gets or sets the unique identifier of the recipient user.
        /// </summary>
        /// <value>The recipient character's unique ID.</value>
        public uint User { get; set; }

        /// <summary>
        /// Gets or sets the sender's user ID.
        /// </summary>
        /// <value>The sender's unique identifier stored in database.</value>
        public uint Sender { get; set; }

        /// <summary>
        /// Gets or sets the title/subject of the mail message.
        /// </summary>
        /// <value>The mail subject line displayed in the mail list.</value>
        public string Title { get; set; }

        /// <summary>
        /// Gets or sets the body content of the mail message.
        /// </summary>
        /// <value>The full text content of the mail message.</value>
        public string Contents { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether this mail has been read by the recipient.
        /// </summary>
        /// <value>True if the mail has been read; otherwise, false.</value>
        public bool Read { get; set; }

        /// <summary>
        /// Gets or sets the date and time when the mail was created.
        /// </summary>
        /// <value>The creation timestamp of the mail message.</value>
        public DateTime CreatedDate { get; set; }

        /// <summary>
        /// Gets or sets the date and time when the mail was last updated.
        /// </summary>
        /// <value>The last modification timestamp of the mail message.</value>
        public DateTime UpdatedDate { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether the mail is marked as deleted.
        /// </summary>
        /// <value>True if the mail is soft-deleted; otherwise, false.</value>
        public bool Deleted { get; set; }
    }
}
