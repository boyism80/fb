using Fb.Model;
using Http.Extension;
using Marketplace.Extension;
using Marketplace.Model;

namespace Marketplace.Services;

public static class ListingReturnParcel
{
    public static ulong RegistrationFee(MarketplaceListing listing)
    {
        var totalPrice = listing.Price * listing.RemainingCount;
        return (ulong)(totalPrice * Fb.Model.ConstValue.Marketplace.ListingFee);
    }

    public static MarketplaceParcel WithFeeRefund(MarketplaceListing listing, string externalRef, string title, string message, string messageWithFee)
    {
        var registrationFee = RegistrationFee(listing);

        var itemName = "unknown item name";
        if (Table.Item.TryGetValue(listing.ItemModel, out var model))
        {
            itemName = model.Name;
        }

        var attachments = new List<Fb.Model.Dsl>
        {
            new Fb.Model.Dsl.Item
            {
                Id = listing.ItemModel,
                Count = listing.RemainingCount,
                Durability = listing.ItemDurability,
                CustomName = listing.ItemCustomName,
                Percent = 100.0,
                Uid = listing.ItemUid
            }.ToDSL()
        };

        if (registrationFee > 0)
        {
            attachments.Add(new Fb.Model.Dsl.Money { Value = registrationFee }.ToDSL());
        }

        var body = registrationFee > 0
            ? string.Format(messageWithFee.ToCSharpFormat(), itemName, listing.RemainingCount, registrationFee)
            : string.Format(message.ToCSharpFormat(), itemName, listing.RemainingCount);

        return new MarketplaceParcel
        {
            ExternalRef = externalRef,
            World = listing.World,
            User = listing.SellerId,
            Title = title,
            Message = body,
            Attachments = attachments
        };
    }
}
