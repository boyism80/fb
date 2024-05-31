namespace MultiwaySearchTree
{
    internal class Program
    {
        static void Main(string[] args)
        {
            var syncService = new SyncService();
            try
            {
                syncService.Sync("lock1", trans =>
                {
                    syncService.Sync("lock2", trans =>
                    {
                        syncService.Sync("lock3", trans =>
                        {
                            syncService.Sync("lock4", trans =>
                            {
                                syncService.Sync("lock5", trans =>
                                {
                                }, trans);
                            }, trans);
                        }, trans);
                    }, trans);
                });

                syncService.Sync("lock5", trans =>
                {
                    syncService.Sync("lock2", trans =>
                    {
                    }, trans);
                });

                syncService.Sync("lock2", trans =>
                {
                    syncService.Sync("lock5", trans =>
                    {
                        syncService.Sync("lock3", trans =>
                        {
                        }, trans);
                    }, trans);
                });
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
            }
        }
    }
}