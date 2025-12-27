# Marketplace Game Server Implementation Guide

## Overview

This document describes the implementation requirements for integrating the marketplace service into the game server. The game server must communicate with the marketplace HTTP API using FlatBuffer protocol for serialization.

## Architecture

### Communication Flow

```
Game Client <---> Game Server <--HTTP/FlatBuffer--> Marketplace Server
```

- **Protocol**: FlatBuffer over HTTP POST
- **Content-Type**: `application/octet-stream`
- **Service Name**: `marketplace` (configured in game server config)

## Required Components

### 1. HTTP Client Integration

The game server already has an HTTP client (`fb::http_client`) that supports FlatBuffer serialization. Use the existing `post()` method:

```cpp
auto response = co_await server.http.post("marketplace", "/marketplace/list", request);
```

### 2. FlatBuffer Protocol Handlers

Create protocol handlers for each marketplace operation following the existing pattern in `server/game/lib/handler/protocol/`.

### 3. Character Integration

Integrate marketplace operations with the character's inventory and money system:
- **Item Removal**: Remove items from character inventory when listing
- **Item Addition**: Add items to character inventory when purchasing or receiving returns
- **Money Deduction**: Deduct listing fees and purchase prices
- **Money Addition**: Add seller revenue from completed transactions

## API Endpoints

### 1. List Item (`POST /marketplace/list`)

**Request**: `fb::protocol::marketplace::request::MarketplaceList`
- `character_id`: Seller's character ID
- `request_id`: Unique request ID for idempotency (UUID string)
- `item`: Item details (model, count, durability, custom_name)
- `price`: Listing price
- `expire_hours`: Hours until listing expires (default: 72)

**Response**: `fb::protocol::marketplace::response::MarketplaceList`
- `success`: Operation success status
- `listing_id`: Created listing ID
- `listing_fee`: Fee charged for listing
- `request_id`: Echo of request ID
- `error`: Error code (0 = success)

**Game Server Implementation**:
1. Validate character has the item in inventory
2. Validate character has enough money for listing fee
3. Generate UUID for `request_id`
4. Send request to marketplace server
5. If successful:
   - Remove item from character inventory
   - Deduct listing fee from character money
   - Store `request_id` for status checking
6. Handle errors appropriately

### 2. Cancel Listing (`POST /marketplace/cancel`)

**Request**: `fb::protocol::marketplace::request::MarketplaceCancel`
- `character_id`: Seller's character ID
- `listing_id`: Listing ID to cancel

**Response**: `fb::protocol::marketplace::response::MarketplaceCancel`
- `success`: Operation success status
- `error`: Error code

**Game Server Implementation**:
1. Validate character owns the listing
2. Send cancel request
3. If successful:
   - Item will be returned via pending returns (see Complete Return)
4. Handle errors appropriately

### 3. Purchase Item (`POST /marketplace/purchase`)

**Request**: `fb::protocol::marketplace::request::MarketplacePurchase`
- `buyer_id`: Buyer's character ID
- `listing_id`: Listing ID to purchase
- `request_id`: Unique request ID for idempotency (UUID string)

**Response**: `fb::protocol::marketplace::response::MarketplacePurchase`
- `success`: Operation success status
- `item`: Purchased item details
- `error`: Error code

**Game Server Implementation**:
1. Validate character has enough money (price + transaction fee)
2. Validate character has inventory space
3. Generate UUID for `request_id`
4. Send purchase request
5. If successful:
   - Deduct money (price + transaction fee) from buyer
   - Add item to buyer's inventory
   - Seller revenue will be available via pending transactions (see Complete Transaction)
6. Handle errors appropriately

### 4. Search Items (`POST /marketplace/search`)

**Request**: `fb::protocol::marketplace::request::MarketplaceSearch`
- `item_name`: Item name prefix (optional)
- `min_price`: Minimum price filter (optional)
- `max_price`: Maximum price filter (optional)
- `seller_id`: Filter by seller ID (optional)
- `sort_by`: Sort order (e.g., "price_asc", "price_desc", "name_asc") (optional)
- `page`: Page number (default: 1)
- `page_size`: Items per page (default: 20)

**Response**: `fb::protocol::marketplace::response::MarketplaceSearch`
- `success`: Operation success status
- `result`: Search results containing listings, total count, pagination info
- `error`: Error code

**Game Server Implementation**:
1. Send search request with filters
2. Display results to player
3. Handle pagination for large result sets

### 5. Check Listing Status (`POST /marketplace/check-status`)

**Request**: `fb::protocol::marketplace::request::MarketplaceCheckListingStatus`
- `request_id`: Request ID from list operation

**Response**: `fb::protocol::marketplace::response::MarketplaceCheckListingStatus`
- `success`: Operation success status
- `listing_id`: Listing ID if created
- `listing_fee`: Listing fee charged
- `error`: Error code

**Game Server Implementation**:
1. Use this to check if a listing was successfully created after async operation
2. Can be called periodically or on player login to verify listing status

### 6. Get Pending Transactions (`POST /marketplace/pending`)

**Request**: `fb::protocol::marketplace::request::MarketplaceGetPendingTransactions`
- `character_id`: Character ID
- `type`: Transaction type (0=Seller revenue, 1=Buyer item loss, 2=Expired returns)

**Response**: `fb::protocol::marketplace::response::MarketplaceGetPendingTransactions`
- `success`: Operation success status
- `transactions`: List of pending transactions (seller revenue)
- `returns`: List of pending returns (expired/cancelled listings)
- `error`: Error code

**Game Server Implementation**:
1. Call on player login to check for pending transactions/returns
2. Display pending items to player
3. Allow player to complete transactions/returns

### 7. Complete Pending Transaction (`POST /marketplace/complete-transaction`)

**Request**: `fb::protocol::marketplace::request::MarketplaceCompletePendingTransaction`
- `transaction_id`: Transaction ID to complete

**Response**: `fb::protocol::marketplace::response::MarketplaceCompletePendingTransaction`
- `success`: Operation success status

**Game Server Implementation**:
1. Send complete request
2. If successful:
   - Add seller revenue to character money
3. Handle errors appropriately

### 8. Complete Pending Return (`POST /marketplace/complete-return`)

**Request**: `fb::protocol::marketplace::request::MarketplaceCompletePendingReturn`
- `return_id`: Return ID to complete

**Response**: `fb::protocol::marketplace::response::MarketplaceCompletePendingReturn`
- `success`: Operation success status

**Game Server Implementation**:
1. Validate character has inventory space
2. Send complete request
3. If successful:
   - Add returned item to character inventory
4. Handle errors appropriately

## Error Codes

The marketplace server uses standard error codes defined in `ErrorCode` enum. Common errors:
- `ErrorCode::None` (0): Success
- `ErrorCode::MarketplaceListingNotFound`: Listing does not exist
- `ErrorCode::MarketplaceListingExpired`: Listing has expired
- `ErrorCode::MarketplaceListingAlreadySold`: Listing was already sold
- `ErrorCode::MarketplaceListingAlreadyCancelled`: Listing was already cancelled
- `ErrorCode::MarketplaceInsufficientFunds`: Not enough money
- `ErrorCode::MarketplaceInventoryFull`: Inventory is full
- `ErrorCode::MarketplaceMaxListingsReached`: Character has reached max listings
- `ErrorCode::Unhandled`: Unexpected server error

## Implementation Checklist

### Core Functionality
- [ ] Create marketplace protocol handler base class
- [ ] Implement `marketplace_list` handler
- [ ] Implement `marketplace_cancel` handler
- [ ] Implement `marketplace_purchase` handler
- [ ] Implement `marketplace_search` handler
- [ ] Implement `marketplace_check_status` handler
- [ ] Implement `marketplace_get_pending` handler
- [ ] Implement `marketplace_complete_transaction` handler
- [ ] Implement `marketplace_complete_return` handler

### Integration Points
- [ ] Integrate with character inventory system
- [ ] Integrate with character money system
- [ ] Add inventory space validation
- [ ] Add money validation
- [ ] Handle item removal on listing
- [ ] Handle item addition on purchase/return
- [ ] Handle money deduction on listing/purchase
- [ ] Handle money addition on transaction completion

### UI/UX
- [ ] Create marketplace UI window
- [ ] Display search results
- [ ] Display listing details
- [ ] Display pending transactions/returns
- [ ] Show error messages to player
- [ ] Implement pagination UI

### Background Tasks
- [ ] Periodic check for pending transactions/returns on player login
- [ ] Auto-complete pending transactions/returns (optional)
- [ ] Notification system for completed transactions

### Configuration
- [ ] Add marketplace server configuration (IP, port)
- [ ] Add marketplace service name to config
- [ ] Configure request timeouts
- [ ] Configure retry logic

### Error Handling
- [ ] Handle network errors
- [ ] Handle timeout errors
- [ ] Handle invalid responses
- [ ] Handle server errors gracefully
- [ ] Log errors appropriately

### Testing
- [ ] Test listing creation
- [ ] Test listing cancellation
- [ ] Test item purchase
- [ ] Test search functionality
- [ ] Test pending transactions
- [ ] Test pending returns
- [ ] Test error scenarios
- [ ] Test concurrent operations
- [ ] Test idempotency (duplicate requests)

## Code Structure

### Recommended File Organization

```
server/game/lib/handler/protocol/
├── marketplace_list.cpp
├── marketplace_list.h
├── marketplace_cancel.cpp
├── marketplace_cancel.h
├── marketplace_purchase.cpp
├── marketplace_purchase.h
├── marketplace_search.cpp
├── marketplace_search.h
├── marketplace_check_status.cpp
├── marketplace_check_status.h
├── marketplace_get_pending.cpp
├── marketplace_get_pending.h
├── marketplace_complete_transaction.cpp
├── marketplace_complete_transaction.h
├── marketplace_complete_return.cpp
└── marketplace_complete_return.h
```

### Handler Pattern Example

```cpp
// marketplace_list.h
#include <fb/game/handler/protocol.h>
#include <fb/protocol/marketplace/request/marketplace_list_generated.h>

namespace fb::game::handler::protocol
{
    class marketplace_list : public handler::protocol<server, marketplace_reqs::MarketplaceList>
    {
    public:
        marketplace_list(server& server);
        async::task<bool> handle(socket<character>& session, marketplace_reqs::MarketplaceList& request) override;
    };
}

// marketplace_list.cpp
#include <fb/game/handler/protocol/marketplace_list.h>
#include <fb/game/server.h>
#include <uuid/uuid.h>

using namespace fb::game::handler::protocol;

marketplace_list::marketplace_list(server& server) :
    handler::protocol<server, marketplace_reqs::MarketplaceList>(server)
{ }

async::task<bool> marketplace_list::handle(socket<character>& session, marketplace_reqs::MarketplaceList& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    // Validate item exists in inventory
    auto item = ch->items.at(request.item_index);
    if (item == nullptr)
    {
        ch->message(_TEXT(MESSAGE_EXCEPTION_ITEM_NOT_FOUND));
        co_return true;
    }

    // Validate money
    auto listing_fee = calculate_listing_fee(request.price);
    if (ch->money() < listing_fee)
    {
        ch->message(_TEXT(MESSAGE_EXCEPTION_INSUFFICIENT_FUNDS));
        co_return true;
    }

    // Generate request ID
    uuid_t uuid;
    uuid_generate(uuid);
    char uuid_str[37];
    uuid_unparse(uuid, uuid_str);
    request.request_id = uuid_str;

    // Build marketplace request
    marketplace_reqs::MarketplaceList marketplace_request;
    marketplace_request.character_id = ch->id;
    marketplace_request.request_id = uuid_str;
    marketplace_request.item.model = item->model().id;
    marketplace_request.item.count = item->count();
    marketplace_request.item.durability = item->durability();
    marketplace_request.item.custom_name = item->custom_name();
    marketplace_request.price = request.price;
    marketplace_request.expire_hours = request.expire_hours;

    try
    {
        // Send request to marketplace server
        auto response = co_await this->server().http.post("marketplace", "/marketplace/list", marketplace_request);

        if (response.success)
        {
            // Remove item from inventory
            ch->items.remove(request.item_index);

            // Deduct listing fee
            ch->money(ch->money() - response.listing_fee);

            // Send success response to client
            marketplace_resps::MarketplaceList client_response;
            client_response.success = true;
            client_response.listing_id = response.listing_id;
            client_response.listing_fee = response.listing_fee;
            session.send(client_response);
        }
        else
        {
            // Handle error
            handle_marketplace_error(ch, response.error);
        }
    }
    catch (const std::exception& e)
    {
        // Handle network/server error
        ch->message(_TEXT(MESSAGE_EXCEPTION_MARKETPLACE_ERROR));
        this->server().logger.error("Marketplace list failed: {}", e.what());
    }

    co_return true;
}
```

## Notes

1. **Idempotency**: All write operations (list, purchase) use `request_id` for idempotency. The game server should generate a UUID for each request and store it to handle retries.

2. **Async Operations**: Marketplace operations are asynchronous. The game server should handle timeouts and retries appropriately.

3. **Pending Transactions**: The marketplace server creates pending transactions/returns that must be completed by the game server. These should be checked on player login.

4. **Inventory Management**: The game server is responsible for managing character inventory. Items are removed/added immediately on successful operations.

5. **Money Management**: The game server is responsible for managing character money. Fees and prices are deducted/added immediately on successful operations.

6. **Error Handling**: All errors should be communicated to the player in a user-friendly manner.

7. **Configuration**: The marketplace server endpoint should be configurable via game server configuration files.

