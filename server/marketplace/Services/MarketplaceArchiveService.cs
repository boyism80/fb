using fb.protocol.marketplace;
using Fb.Model;
using Http.Service;
using Marketplace.Extension;
using Marketplace.Model;

namespace Marketplace.Services
{
    /// <summary>
    /// Provides marketplace archive functionality.
    /// Handles archiving of completed, cancelled, and expired listings.
    /// </summary>
    public class MarketplaceArchiveService
    {
        private readonly Service.DbContext _dbContext;
        private readonly StorageService _storageService;
        private readonly LogService _logService;
        private readonly ILogger<MarketplaceArchiveService> _logger;

        private const int ArchiveBatchSize = 1000;

        /// <summary>
        /// Initializes a new instance of the <see cref="MarketplaceArchiveService"/> class.
        /// </summary>
        public MarketplaceArchiveService(
            Service.DbContext dbContext,
            StorageService storageService,
            LogService logService,
            ILogger<MarketplaceArchiveService> logger)
        {
            _dbContext = dbContext;
            _storageService = storageService;
            _logService = logService;
            _logger = logger;
        }

        /// <summary>
        /// Archives marketplace listings that are completed, cancelled, or expired.
        /// Processes listings in batches and handles expired listings by returning items and registration fee to sellers.
        /// </summary>
        /// <param name="cancellationToken">Cancellation token for the operation.</param>
        /// <returns>Task representing the asynchronous operation.</returns>
        public async Task ArchiveListingsAsync(CancellationToken cancellationToken)
        {
            // Use transaction to ensure atomicity
            await using var conn = _dbContext.Connection(-1);
            await conn.OpenAsync(cancellationToken);
            await using var transaction = await conn.BeginTransactionAsync(cancellationToken);

            try
            {
                // Get listings to archive with row lock
                var listingsToArchive = await _dbContext.Marketplace.GetListingsToArchiveForUpdateAsync(
                    ArchiveBatchSize,
                    transaction);

                if (listingsToArchive.Count == 0)
                {
                    await transaction.CommitAsync(cancellationToken);
                    return;
                }

                // Classify listings by status
                var soldOrCancelled = listingsToArchive
                    .Where(l => l.Status == ListingState.SOLD || l.Status == ListingState.CANCELLED)
                    .ToList();

                var expired = listingsToArchive
                    .Where(l => l.Status == ListingState.EXPIRED ||
                               (l.Status == ListingState.ACTIVE && l.ExpireDate < DateTime.UtcNow))
                    .ToList();

                // Process SOLD/CANCELLED listings
                if (soldOrCancelled.Count > 0)
                {
                    await _dbContext.Marketplace.ArchiveListingsAsync(soldOrCancelled, transaction);
                    await _dbContext.Marketplace.DeleteListingsAsync(
                        soldOrCancelled.Select(l => l.Id).ToList(),
                        transaction);
                }

                // Process EXPIRED listings
                if (expired.Count > 0)
                {
                    // Archive and delete expired listings
                    await _dbContext.Marketplace.ArchiveListingsAsync(expired, transaction);
                    await _dbContext.Marketplace.DeleteListingsAsync(
                        expired.Select(l => l.Id).ToList(),
                        transaction);
                }

                // Commit transaction first
                await transaction.CommitAsync(cancellationToken);

                // Process expired listings to return items with registration fee refund
                // This is done after transaction commit to ensure atomicity of archive operations
                if (expired.Count > 0)
                {
                    foreach (var listing in expired)
                    {
                        await ProcessExpiredListingAsync(listing, cancellationToken);
                    }
                }

                // Log success
                _logService?.Write("marketplace_archive_success", new
                {
                    sold_cancelled_count = soldOrCancelled.Count,
                    expired_count = expired.Count,
                    total_count = listingsToArchive.Count
                });
            }
            catch (Exception ex)
            {
                await transaction.RollbackAsync(cancellationToken);
                _logger.LogError(ex, "Error archiving marketplace listings");
                _logService?.Write("marketplace_archive_failed", new
                {
                    error = ex.Message
                });
                throw;
            }
        }

        /// <summary>
        /// Processes an expired listing by returning items and registration fee to the seller.
        /// </summary>
        /// <param name="listing">The expired listing to process.</param>
        /// <param name="cancellationToken">Cancellation token for the operation.</param>
        /// <returns>Task representing the asynchronous operation.</returns>
        private async Task ProcessExpiredListingAsync(
            MarketplaceListing listing,
            CancellationToken cancellationToken)
        {
            // Calculate registration fee that was paid when listing was created
            var totalPrice = listing.Price * listing.RemainingCount;
            var registrationFee = (uint)(totalPrice * Fb.Model.ConstValue.Marketplace.ListingFee);

            // Get item name for message
            var itemName = "unknown item name";
            if (Table.Item.TryGetValue(listing.ItemModel, out var model))
            {
                itemName = model.Name;
            }

            // Create attachments: item + registration fee refund
            var attachments = new List<Fb.Model.Dsl>
            {
                new Fb.Model.Dsl.Item
                {
                    Id = listing.ItemModel,
                    Count = listing.RemainingCount,
                    Durability = listing.ItemDurability,
                    CustomName = listing.ItemCustomName,
                    Percent = 100.0
                }.ToDSL()
            };

            // Add registration fee refund
            if (registrationFee > 0)
            {
                attachments.Add(new Fb.Model.Dsl.Money { Value = registrationFee }.ToDSL());
            }

            // Use StorageService to create pending box (handles Redis caching and DB write-back automatically)
            var message = registrationFee > 0
                ? string.Format(Fb.Model.ConstValue.String.MessageMarketplaceListingExpiredMessageWithFee.ToCSharpFormat(), itemName, listing.RemainingCount, registrationFee)
                : string.Format(Fb.Model.ConstValue.String.MessageMarketplaceListingExpiredMessage.ToCSharpFormat(), itemName, listing.RemainingCount);

            await _storageService.CreatePendingAsync(
                Fb.Model.ConstValue.String.MessageMarketplaceListingExpiredTitle,
                message,
                listing.SellerId,
                null, // Unlimited expiry for marketplace items
                attachments);

            // Log expired listing processing
            _logService?.Write("marketplace_expired_listing_processed", new
            {
                listing_id = listing.Id,
                seller_id = listing.SellerId,
                item_model = listing.ItemModel,
                remaining_count = listing.RemainingCount,
                total_price = totalPrice,
                registration_fee = registrationFee
            });
        }
    }
}

