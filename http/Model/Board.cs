namespace Http.Model
{
    /// <summary>
    /// Represents a board post model for the in-game bulletin board system.
    /// Contains all information about posts made by players on various board sections.
    /// </summary>
    public class Board : IModel
    {
        /// <summary>
        /// Gets or sets the unique identifier for this board post.
        /// </summary>
        /// <value>The unique post ID used for database operations and references.</value>
        public uint Id { get; set; }

        /// <summary>
        /// Gets or sets the board section identifier where this post belongs.
        /// </summary>
        /// <value>The section ID that categorizes this post (e.g., general, trade, guild, etc.).</value>
        public uint Section { get; set; }

        /// <summary>
        /// Gets or sets the unique identifier of the user who created this post.
        /// </summary>
        /// <value>The author's unique character ID.</value>
        public uint User { get; set; }

        /// <summary>
        /// Gets or sets the display name of the user who created this post.
        /// </summary>
        /// <value>The author's character name for display purposes.</value>
        public string UserName { get; set; }

        /// <summary>
        /// Gets or sets the title of the board post.
        /// </summary>
        /// <value>The post title displayed in the board list and post header.</value>
        public string Title { get; set; }

        /// <summary>
        /// Gets or sets the content body of the board post.
        /// </summary>
        /// <value>The full text content of the post message.</value>
        public string Contents { get; set; }

        /// <summary>
        /// Gets or sets the date and time when the post was created.
        /// </summary>
        /// <value>The creation timestamp of the board post.</value>
        public DateTime CreatedDate { get; set; }

        /// <summary>
        /// Gets or sets the date and time when the post was last updated.
        /// </summary>
        /// <value>The last modification timestamp of the board post.</value>
        public DateTime UpdatedDate { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether the post is marked as deleted.
        /// </summary>
        /// <value>True if the post is soft-deleted; otherwise, false.</value>
        public bool Deleted { get; set; }
    }
}
