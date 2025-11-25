using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace Internal.Service
{
    public class BulletinService
    {
        private readonly ConcurrentDictionary<uint, ConcurrentQueue<BulletinWriteRequest>> _writeQueues = new();
        private readonly ConcurrentDictionary<uint, ConcurrentQueue<BulletinDeleteRequest>> _deleteQueues = new();
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

