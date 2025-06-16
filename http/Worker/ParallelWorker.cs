using System.Collections.Concurrent;

namespace Http.Worker
{
    /// <summary>
    /// Provides an abstract base class for parallel processing of work items.
    /// Implements a multi-threaded worker pattern with progress tracking and error handling.
    /// </summary>
    /// <typeparam name="T">The type of work items to be processed.</typeparam>
    public abstract class ParallelWorker<T>
    {
        private readonly ConcurrentQueue<T> _queue = new ConcurrentQueue<T>();
        private readonly ILogger _logger;

        /// <summary>
        /// Initializes a new instance of the <see cref="ParallelWorker{T}"/> class.
        /// </summary>
        /// <param name="logger">The logger instance for logging operations and errors.</param>
        protected ParallelWorker(ILogger logger)
        {
            _logger = logger;
        }

        /// <summary>
        /// When overridden in a derived class, provides the collection of work items to be processed.
        /// </summary>
        /// <returns>An enumerable collection of work items ready for processing.</returns>
        protected abstract IEnumerable<T> OnReady();

        /// <summary>
        /// When overridden in a derived class, performs initialization before work processing begins.
        /// </summary>
        protected abstract void OnStart();

        /// <summary>
        /// When overridden in a derived class, processes a single work item.
        /// </summary>
        /// <param name="value">The work item to process.</param>
        protected abstract void OnWork(T value);

        /// <summary>
        /// When overridden in a derived class, handles completion of a work item with progress information.
        /// </summary>
        /// <param name="input">The work item that was completed.</param>
        /// <param name="percent">The completion percentage (0-100) of all work items.</param>
        protected virtual void OnWorked(T input, int percent)
        { }

        /// <summary>
        /// When overridden in a derived class, calculates the total count of work items for progress tracking.
        /// </summary>
        /// <param name="inputs">The read-only list of work items.</param>
        /// <returns>The total count of work items to be processed.</returns>
        protected virtual int TotalCount(IReadOnlyList<T> inputs)
        {
            return inputs.Count;
        }

        /// <summary>
        /// When overridden in a derived class, provides additional count for runtime-generated work items.
        /// </summary>
        /// <returns>The additional count of work items that may be generated during runtime.</returns>
        protected virtual int RuntimeAdditionalCount()
        {
            return 0;
        }

        /// <summary>
        /// When overridden in a derived class, handles errors that occur during work item processing.
        /// </summary>
        /// <param name="input">The work item that caused the error.</param>
        /// <param name="e">The exception that occurred during processing.</param>
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

        /// <summary>
        /// When overridden in a derived class, handles the start of processing for a specific work item.
        /// </summary>
        /// <param name="input">The work item that is starting to be processed.</param>
        /// <param name="percent">The current completion percentage (0-100) of all work items.</param>
        protected virtual void OnStart(T input, int percent)
        { }

        /// <summary>
        /// When overridden in a derived class, performs cleanup after all work processing is complete.
        /// </summary>
        protected virtual void OnFinish()
        { }

        /// <summary>
        /// Adds a work item to the processing queue.
        /// </summary>
        /// <param name="value">The work item to add to the queue.</param>
        private void Enqueue(T value)
        {
            _queue.Enqueue(value);
        }

        /// <summary>
        /// Executes the parallel processing of all work items.
        /// Creates worker tasks equal to the processor count and processes items concurrently.
        /// </summary>
        /// <exception cref="AggregateException">Thrown when one or more errors occur during processing.</exception>
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
