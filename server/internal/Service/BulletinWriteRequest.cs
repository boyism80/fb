namespace Internal.Service
{
    /// <summary>
    /// Represents a bulletin article write request queued for batch processing.
    /// Contains article data and a completion source for notifying when the write operation completes.
    /// </summary>
    public class BulletinWriteRequest
    {
        /// <summary>
        /// Gets or sets the bulletin section identifier.
        /// </summary>
        /// <value>The section ID where the article will be posted.</value>
        public uint Section { get; set; }

        /// <summary>
        /// Gets or sets the user identifier who is writing the article.
        /// </summary>
        /// <value>The user ID of the article author.</value>
        public uint User { get; set; }

        /// <summary>
        /// Gets or sets the article title.
        /// </summary>
        /// <value>The title of the article.</value>
        public string Title { get; set; } = string.Empty;

        /// <summary>
        /// Gets or sets the article contents.
        /// </summary>
        /// <value>The body content of the article.</value>
        public string Contents { get; set; } = string.Empty;

        /// <summary>
        /// Gets or sets the task completion source for notifying when the write operation completes.
        /// </summary>
        /// <value>The TaskCompletionSource that will be set when the article is written to the database.</value>
        public TaskCompletionSource<bool> CompletionSource { get; set; } = new TaskCompletionSource<bool>();
    }
}

