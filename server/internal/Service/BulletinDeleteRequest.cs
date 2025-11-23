namespace Internal.Service
{
    /// <summary>
    /// Represents a bulletin article delete request queued for batch processing.
    /// Contains article identification data and a completion source for notifying when the delete operation completes.
    /// </summary>
    public class BulletinDeleteRequest
    {
        /// <summary>
        /// Gets or sets the bulletin section identifier.
        /// </summary>
        /// <value>The section ID where the article belongs.</value>
        public uint Section { get; set; }

        /// <summary>
        /// Gets or sets the article identifier to delete.
        /// </summary>
        /// <value>The unique article ID to be deleted.</value>
        public uint Id { get; set; }

        /// <summary>
        /// Gets or sets the user identifier who is deleting the article.
        /// </summary>
        /// <value>The user ID of the article owner requesting deletion.</value>
        public uint User { get; set; }

        /// <summary>
        /// Gets or sets the task completion source for notifying when the delete operation completes.
        /// </summary>
        /// <value>The TaskCompletionSource that will be set with the deletion result code when the operation completes.</value>
        public TaskCompletionSource<int> CompletionSource { get; set; } = new TaskCompletionSource<int>();
    }
}

