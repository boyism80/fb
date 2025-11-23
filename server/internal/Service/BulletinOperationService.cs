using System.Collections.Concurrent;

namespace Internal.Service
{
    /// <summary>
    /// Provides a service for managing bulletin article write and delete requests in memory queues.
    /// Organizes write and delete requests by section for efficient batch processing with database sharding support.
    /// </summary>
    public class BulletinOperationService
    {
        /// <summary>
        /// Dictionary of queues organized by section ID for write request batch processing.
        /// Each section maintains its own queue of write requests.
        /// </summary>
        private readonly ConcurrentDictionary<uint, ConcurrentQueue<BulletinWriteRequest>> _writeQueues = new();

        /// <summary>
        /// Dictionary of queues organized by section ID for delete request batch processing.
        /// Each section maintains its own queue of delete requests.
        /// </summary>
        private readonly ConcurrentDictionary<uint, ConcurrentQueue<BulletinDeleteRequest>> _deleteQueues = new();

        /// <summary>
        /// Enqueues a bulletin article write request to the appropriate section queue.
        /// Returns a task that completes when the article is written to the database.
        /// </summary>
        /// <param name="section">The bulletin section identifier.</param>
        /// <param name="user">The user identifier who is writing the article.</param>
        /// <param name="title">The article title.</param>
        /// <param name="contents">The article contents.</param>
        /// <returns>A task that completes with true if the write succeeds, false otherwise.</returns>
        public Task<bool> Write(uint section, uint user, string title, string contents)
        {
            var request = new BulletinWriteRequest
            {
                Section = section,
                User = user,
                Title = title,
                Contents = contents,
                CompletionSource = new TaskCompletionSource<bool>()
            };

            var queue = _writeQueues.GetOrAdd(section, _ => new ConcurrentQueue<BulletinWriteRequest>());
            queue.Enqueue(request);

            return request.CompletionSource.Task;
        }

        /// <summary>
        /// Enqueues a bulletin article delete request to the appropriate section queue.
        /// Returns a task that completes when the article is deleted from the database.
        /// </summary>
        /// <param name="section">The bulletin section identifier.</param>
        /// <param name="id">The article identifier to delete.</param>
        /// <param name="user">The user identifier who is deleting the article.</param>
        /// <returns>A task that completes with the deletion result code (1 for success, negative values for errors).</returns>
        public Task<int> Delete(uint section, uint id, uint user)
        {
            var request = new BulletinDeleteRequest
            {
                Section = section,
                Id = id,
                User = user,
                CompletionSource = new TaskCompletionSource<int>()
            };

            var queue = _deleteQueues.GetOrAdd(section, _ => new ConcurrentQueue<BulletinDeleteRequest>());
            queue.Enqueue(request);

            return request.CompletionSource.Task;
        }

        /// <summary>
        /// Dequeues write and delete requests from all section queues up to the specified batch size.
        /// Groups requests by section for efficient batch processing.
        /// </summary>
        /// <param name="maxBatchSize">The maximum number of requests to dequeue per section.</param>
        /// <returns>A tuple containing dictionaries mapping section IDs to their dequeued write and delete requests.</returns>
        public (Dictionary<uint, List<BulletinWriteRequest>> Writes, Dictionary<uint, List<BulletinDeleteRequest>> Deletes) DequeueBatch(int maxBatchSize)
        {
            // Dequeue write requests
            var writes = new Dictionary<uint, List<BulletinWriteRequest>>();
            foreach (var (section, queue) in _writeQueues)
            {
                var batch = new List<BulletinWriteRequest>();
                while (batch.Count < maxBatchSize && queue.TryDequeue(out var request))
                {
                    batch.Add(request);
                }
                if (batch.Count > 0)
                {
                    writes[section] = batch;
                }
            }

            // Dequeue delete requests (same approach as writes)
            var deletes = new Dictionary<uint, List<BulletinDeleteRequest>>();
            foreach (var (section, queue) in _deleteQueues)
            {
                var batch = new List<BulletinDeleteRequest>();
                while (batch.Count < maxBatchSize && queue.TryDequeue(out var request))
                {
                    batch.Add(request);
                }
                if (batch.Count > 0)
                {
                    deletes[section] = batch;
                }
            }

            return (writes, deletes);
        }
    }
}

