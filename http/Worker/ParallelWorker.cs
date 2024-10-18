using System.Collections.Concurrent;

namespace http.Worker
{
    public abstract class ParallelWorker<T>
    {
        private readonly ConcurrentQueue<T> _queue = new ConcurrentQueue<T>();
        private readonly ILogger _logger;

        protected ParallelWorker(ILogger logger)
        {
            _logger = logger;
        }

        protected abstract IEnumerable<T> OnReady();

        protected abstract void OnStart();

        protected abstract void OnWork(T value);

        protected virtual void OnWorked(T input, int percent)
        { }

        protected virtual int TotalCount(IReadOnlyList<T> inputs)
        {
            return inputs.Count;
        }

        protected virtual int RuntimeAdditionalCount()
        {
            return 0;
        }

        protected virtual void OnError(T input, Exception e)
        {
            switch (e)
            {
                case AggregateException ae:
                    foreach (var x in ae.InnerExceptions)
                        _logger.LogError(x.Message);
                    break;

                default:
                    _logger.LogError(e.Message);
                    break;
            }
        }

        protected virtual void OnStart(T input, int percent)
        { }

        protected virtual void OnFinish()
        { }

        private void Enqueue(T value)
        {
            _queue.Enqueue(value);
        }

        public void Run()
        {
            var mutexWorked = new Mutex();
            var mutexErrors = new Mutex();
            var indices = new Dictionary<T, int>();
            foreach (var (input, i) in OnReady().Select((x, i) => (x, i)))
            {
                Enqueue(input);
                if (indices.ContainsKey(input) == false)
                    indices.Add(input, indices.Count);
            }

            var processed = 0;
            var totalCount = TotalCount(_queue.ToList());
            var unhandledErrors = new ConcurrentBag<Exception>();
            var tasks = Enumerable.Range(0, Environment.ProcessorCount).Select(_ => new Task(() =>
            {
                while (true)
                {
                    var exists = _queue.TryDequeue(out var input);
                    if (exists == false)
                        break;

                    try
                    {
                        OnWork(input);
                        mutexWorked.WaitOne();
                        processed++;
                        OnWorked(input, (processed * 100) / totalCount);
                        mutexWorked.ReleaseMutex();
                    }
                    catch (AggregateException e)
                    {
                        var stack = new Stack<Exception>();
                        stack.Push(e);

                        while (stack.TryPop(out var error))
                        {
                            switch (error)
                            {
                                case AggregateException aggregateException:
                                    foreach (var inner in aggregateException.InnerExceptions)
                                    {
                                        stack.Push(inner);
                                    }
                                    break;

                                default:
                                    unhandledErrors.Add(e);
                                    break;
                            }
                        }
                    }
                    catch (Exception e)
                    {
                        unhandledErrors.Add(e);
                    }
                }
            })).ToArray();

            OnStart();
            foreach (var task in tasks)
            {
                task.Start();
            }

            Task.WaitAll(tasks);
            if (!unhandledErrors.IsEmpty)
            {
                throw new AggregateException(unhandledErrors);
            }

            OnFinish();
        }
    }
}
