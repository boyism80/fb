# std::function 파라미터 개선 체크리스트

## 분류 기준

각 `std::function` 파라미터는 다음 세 가지 케이스 중 하나로 분류됩니다:

1. **즉시 실행용** → `const std::function&` 사용
   - 함수가 즉시 호출되고 저장되지 않는 경우
   - Predicate 함수, 즉시 실행 콜백

2. **저장용** → `std::function&&` 사용
   - 함수가 멤버 변수나 컨테이너에 저장되는 경우
   - Queue, Map, Timer 등에 저장

3. **코루틴 생명주기 문제** → `by value` 유지
   - 코루틴 내에서 함수를 나중에 사용해야 하는 경우
   - Lambda capture와 생명주기 문제가 있는 경우

---

## 체크리스트

### ✅ Case 1: 즉시 실행용 → `const std::function&`

| 파일 | 라인 | 함수명 | 현재 스타일 | 권장 스타일 | 상태 | 비고 |
|------|------|--------|-------------|-------------|------|------|
| `include/fb/game/map.h` | 60 | `movable()` | `const_ref` | `const_ref` | ✅ 적절 | Predicate 즉시 실행 |
| `include/fb/acceptor.h` | 528 | `access_sockets()` | `const_ref` | `const_ref` | ✅ 적절 | 즉시 실행 |
| `include/fb/mst.h` | 87 | `travel()` | `const_ref` | `const_ref` | ✅ 적절 | Predicate 즉시 실행 |
| `include/fb/bot/hook_params.h` | 12-13 | `condition`, `matched` | `const_ref` | `const_ref` | ✅ 적절 | 즉시 실행 |
| `include/fb/bot/bot.h` | 25, 119, 126 | `condition`, `request()` | `const_ref` | `const_ref` | ✅ 적절 | 즉시 실행 |
| `include/fb/bot/controller.h` | 378, 437 | `request()` | `const_ref` | `const_ref` | ✅ 적절 | 즉시 실행 |
| `include/fb/game/channel/storage_pending_channel.h` | 28 | `read()` | `const_ref` | `const_ref` | ✅ 적절 | 즉시 실행 |
| `include/fb/game/channel/system_mail_channel.h` | 25 | `read()` | `const_ref` | `const_ref` | ✅ 적절 | 즉시 실행 |
| `include/fb/table.h` | 21 | `load()` | `const_ref` | `const_ref` | ✅ 적절 | 즉시 실행 (callback) |
| `include/fb/thread_container.h` | 55, 57, 58, 91, 92, 120, 122, 123, 159, 192, 262 | `enqueue()` | `const_ref` | `const_ref` | ✅ 적절 | 즉시 실행 (조건/에러) |
| `include/fb/thread.h` | 94, 98 | `enqueue()` | `const_ref` | `const_ref` | ✅ 적절 | 즉시 실행 (callback) |
| `include/fb/shard_container.h` | 546 | Hash function | `const_ref` | `const_ref` | ✅ 적절 | 즉시 실행 |
| `server/game/lib/map/map.cpp` | 157 | `movable()` | `const_ref` | `const_ref` | ✅ 적절 | Predicate 즉시 실행 |
| `server/game/lib/channel/storage_pending_channel.cpp` | 90 | `read()` | `const_ref` | `const_ref` | ✅ 적절 | 즉시 실행 |
| `server/game/lib/channel/system_mail_channel.cpp` | 89 | `read()` | `const_ref` | `const_ref` | ✅ 적절 | 즉시 실행 |
| `server/bot/lib/game_bot.cpp` | 1081, 1159 | `spawn_monster_with_validator()` | `const_ref` | `const_ref` | ✅ 적절 | 즉시 실행 |

---

### 🔄 Case 2: 저장용 → `std::function&&` (변경 필요)

| 파일 | 라인 | 함수명 | 현재 스타일 | 권장 스타일 | 상태 | 비고 |
|------|------|--------|-------------|-------------|------|------|
| `include/fb/thread.h` | 92-94 | `enqueue()` | `const_ref` | `std::function&&` | ✅ 완료 | Queue에 저장 (`queue.push`) |
| `include/fb/thread.h` | 96-98 | `enqueue()` (template) | `const_ref` | `std::function&&` | ✅ 완료 | Queue에 저장 (`queue.push`) |
| `include/fb/thread.h` | 88 | `settimer()` | `const_ref` | `std::function&&` | ✅ 완료 | Timer 객체에 저장 (`_timers.push_back`) |
| `include/fb/thread.h` | 128, 142 | `dispatch()` | `const_ref` | `std::function&&` | ✅ 완료 | `enqueue()` 호출하므로 함께 변경 |
| `include/fb/thread_container.h` | 56, 92, 105, 119, 158, 193, 228, 261, 298 | `enqueue()`, `dispatch()` | `by_value`/`const_ref` | `std::function&&` | ✅ 완료 | `thread->enqueue()` 호출하여 Queue에 저장 |
| `include/fb/bot/controller.h` | 281 | `bind()` | `const_ref` | `std::function&&` | ✅ 완료 | Handler map에 저장 (`_handler.insert`) |
| `include/fb/bot/bot.h` | 39-40 | `base_bot()` | `by_value` | `std::function&&` | ✅ 완료 | `fb::socket`에 전달하여 저장됨 (참고: `fb::socket` 생성자도 `const_ref` → `&&` 변경 필요) |
| `include/fb/bot/container.h` | 86 | `dispatch()` | `&&` | `&&` | ✅ 적절 | 이미 `&&`로 되어 있음, `thread->dispatch()`에 전달 (체크리스트 오류) |
| `include/fb/bot/game_bot.h` | 76 | `pattern_params` struct | N/A | N/A | ✅ 적절 | 구조체 멤버 타입, 함수 파라미터 아님 |
| `include/fb/async_executor.h` | 87 | `bind_thread_timer()` | `by_value` | `std::function&&` | ✅ 완료 | Timer에 저장 (`settimer`) |
| `include/fb/async_executor.h` | 123 | `bind_timer()` | `by_value` | `std::function&&` | ✅ 완료 | Timer에 저장 |
| `server/fb/src/thread_container.cpp` | 132 | `settimer()` 호출부 | N/A | N/A | ✅ 완료 | `thread.settimer(std::move(fn), duration)` 수정 |
| `include/fb/game/worker.h` | 64-66 | `on_work()` | `const_ref` | `const_ref` | ✅ 적절 | 이미 `const_ref`로 되어 있음 (체크리스트 오류) |
| `include/fb/amqp/queue.h` | 47 | `handler()` (template) | `by_value` | `by_value` | ✅ 완료 | Lambda `[this, fn]` capture 후 `co_await fn()` 사용 (생명주기 문제) |
| `include/fb/amqp/queue.h` | 56 | `handler()` (non-template) | `const_ref` | `std::function&&` | ✅ 완료 | `_handler` map에 저장 (`_handler.insert()`) |
| `include/fb/locker.h` | 25 | `enter()` | `by_value` | `by_value` | ✅ 유지 | 코루틴 내에서 즉시 실행 (`co_await func()`) |
| `include/fb/lua.h` | 542, 546 | `ensure_yield()`, `ensure_resume()` | `by_value` | `by_value` | ✅ 완료 | `async::awaitable_then` lambda capture 후 사용 (생명주기 문제) |
| `server/bot/lib/bot.cpp` | 9-10 | `base_bot()` | `by_value` | `std::function&&` | ✅ 완료 | `fb::socket`에 전달하여 저장됨 |
| `server/bot/lib/controller.cpp` | 62 | `dispatch()` | `&&` | `&&` | ✅ 적절 | 이미 `&&`로 되어 있음, `thread->dispatch()`에 전달 (체크리스트 오류) |
| `server/fb/src/thread.cpp` | 21 | Constructor | N/A | N/A | ✅ 적절 | `std::function<void()>` 로컬 변수, 변경 불필요 |
| `server/fb/src/thread.cpp` | 142 | `enqueue()` | `const_ref` | `std::function&&` | ✅ 완료 | 이미 변경 완료 (우선순위 1) |
| `server/game/lib/worker/worker.script.cpp` | 10 | `on_ready()` | `by_value` | N/A | ✅ 적절 | Generator 반환, 함수 저장 안 함 |
| `server/game/lib/worker/worker.script.cpp` | 100, 105, 110 | `on_work()` | `const_ref` | `const_ref` | ✅ 적절 | 이미 `const_ref`로 되어 있음 (체크리스트 오류) |
| `server/game/lib/server/server.group.cpp` | 213 | `handle_group_action()` | N/A | N/A | ✅ 적절 | 로컬 변수 타입 정의이므로 변경 불필요 |
| `server/fb/src/lua.cpp` | 464, 499 | `ensure_yield()`, `ensure_resume()` | `by_value` | `by_value` | ✅ 완료 | `async::awaitable_then` lambda capture 후 사용 (생명주기 문제) |
| `include/fb/game/character.h` | 302-303 | `invoke()`, `invoke_async()` | `by_value` | `by_value` | ✅ 완료 | `co_await` 후 사용하므로 생명주기 문제 (수정됨) |
| `include/fb/game/channel/storage_pending_channel.h` | 30 | `write()` | `by_value` | `const std::function&` | ✅ 완료 | `locker::write()`에 전달하여 즉시 실행 |
| `include/fb/bot/integration/test_case.h` | 106 | `parallel_scenarios()` | `by_value` | `std::vector<...>&&` | ✅ 완료 | Vector 내부의 `scenario_t`를 queue에 저장 |
| `include/fb/bot/integration/skill_test.h` | 14 | `test_function` | N/A | N/A | ✅ 적절 | 타입 별칭, 함수 파라미터 아님 |
| `include/fb/bot/integration/skill_test.h` | 45, 48, 57 | 구조체 멤버 | N/A | N/A | ✅ 적절 | 구조체 멤버 타입, 함수 파라미터 아님 |
| `server/bot/lib/integration/test/*.cpp` | Various | Test struct members | N/A | N/A | ✅ 적절 | 구조체 멤버 타입, 함수 파라미터 아님 |

---

### ⚠️ Case 3: 코루틴 생명주기 문제 → `by value` 유지

| 파일 | 라인 | 함수명 | 현재 스타일 | 권장 스타일 | 상태 | 비고 |
|------|------|--------|-------------|-------------|------|------|
| `include/fb/game/channel/storage_pending_channel.h` | 29 | `read_async()` | `by_value` | `by_value` | ✅ 유지 | 코루틴 내에서 사용 (`co_await`) |
| `include/fb/game/channel/system_mail_channel.h` | 26 | `read_async()` | `by_value` | `by_value` | ✅ 유지 | 코루틴 내에서 사용 (`co_await`) |
| `include/fb/game/character.h` | 293-295 | `foreach()` | `&&` | `by_value` | ✅ 완료 | Lambda capture 후 `co_await`로 전달, 생명주기 문제 |
| `include/fb/game/character.h` | 295-296 | `foreach_async()` | `&&` | `by_value` | ✅ 완료 | `co_await fn()` 사용, 생명주기 문제 |
| `include/fb/game/character.h` | 297-298 | `foreach()`, `foreach_async()` (names) | `&&` | `by_value` | ✅ 완료 | Lambda capture 후 `co_await`로 전달 |
| `include/fb/game/character.h` | 299-301 | `foreach_enqueue()` | `&&` | `std::function&&` | ✅ 적절 | `shared_ptr`로 감싸서 저장 (`fn_holder`) |
| `include/fb/locker.h` | 25 | `enter()` | `by_value` | `by_value` | ✅ 유지 | 코루틴 내에서 즉시 실행 (`co_await func()`) |
| `server/game/lib/channel/storage_pending_channel.cpp` | 96 | `read_async()` | `by_value` | `by_value` | ✅ 유지 | 코루틴 내에서 사용 (`co_await`) |
| `server/game/lib/channel/system_mail_channel.cpp` | 94 | `read_async()` | `by_value` | `by_value` | ✅ 유지 | 코루틴 내에서 사용 (`co_await`) |

**참고**: 코루틴 내에서 함수를 나중에 사용하는 경우, 참조로 전달하면 생명주기 문제가 발생할 수 있습니다. `by value`로 복사하여 안전하게 사용합니다.

**`co_await fn()` 생명주기 분석**:

1. **직접 호출 (`co_await fn()`)**: 
   - `fn`이 함수의 로컬 변수로 저장되므로, `co_await`가 suspend되어도 함수의 스택 프레임이 유지되어 `fn`은 유효합니다.
   - `invoke_async()`: `fn`을 `by value`로 받아 `co_await fn(ch)` 직접 호출 → **안전** ✅
   - `foreach_async()`: `fn`을 `&&`로 받아 `co_await fn(shared_ptr)` 직접 호출 → **안전** ✅ (함수 내부에서 직접 사용)

2. **Lambda capture 후 전달**:
   - `foreach()`: `fn`을 `&&`로 받아 lambda `[fn]`으로 capture 후 `co_await`로 전달
   - `[fn]`은 복사이므로 안전하지만, `&&`로 받으면 move되므로 호출자의 원본이 파괴됩니다.
   - 더 안전하게 하려면 `by value`로 받아서 복사본을 lambda에 저장하는 것이 좋습니다.

3. **다른 함수에 전달**:
   - `read_async()`: `fn`을 `by value`로 받아 `co_await this->_data.async_read(fn)` 전달
   - `async_read` 내부에서 `fn`을 어떻게 사용하는지에 따라 달라집니다. `by value`로 받았으므로 안전합니다.

**결론**: `co_await fn()`을 직접 호출하는 경우는 생명주기 문제가 없습니다. 하지만 lambda capture나 다른 함수에 전달하는 경우는 `by value`로 받는 것이 더 안전합니다.

---

### 🔧 Case 4: 함수 내부 파라미터 수정 (즉시 실행) → `const std::function&` 변경 가능

| 파일 | 라인 | 함수명 | 현재 스타일 | 권장 스타일 | 상태 | 비고 |
|------|------|--------|-------------|-------------|------|------|
| `include/fb/game/channel/storage_pending_channel.h` | 30 | `write()` | `by_value` | `const std::function&` | 🔄 변경 필요 | `locker::write()`에 전달하여 즉시 실행 |
| `include/fb/game/channel/storage_pending_channel.h` | 28 | `read()` | `const_ref` | `const_ref` | ✅ 적절 | `locker::read()`에 전달하여 즉시 실행 |
| `include/fb/table.h` | 21 | `load()` | `const_ref` | `const_ref` | ✅ 적절 | Callback 즉시 실행 |
| `include/fb/game/character.h` | 302-303 | `invoke()`, `invoke_async()` | `by_value` | `const std::function&` | 🔄 변경 필요 | 즉시 실행 (`fn(ch)`, `co_await fn(ch)`) |
| `include/fb/game/object.h` | 128 | `foreach()` | `const_ref` | `const_ref` | ✅ 적절 | 즉시 실행 |
| `server/game/lib/channel/storage_pending_channel.cpp` | 101 | `write()` | `by_value` | `const std::function&` | 🔄 변경 필요 | `locker::write()`에 전달하여 즉시 실행 |

---

### 🔘 Case 5: Optional callback → `std::function*` 유지

| 파일 | 라인 | 함수명 | 현재 스타일 | 권장 스타일 | 상태 | 비고 |
|------|------|--------|-------------|-------------|------|------|
| `include/fb/thread.h` | 47, 76 | `_data` deleter | `pointer` | `pointer` | ✅ 유지 | Optional deleter (`std::unique_ptr`) |

---

## 변경 우선순위

### 우선순위 1: 명확히 저장되는 경우 (즉시 변경)
1. `include/fb/thread.h` - `enqueue()`, `settimer()` - Queue/Timer에 저장
2. `include/fb/bot/controller.h` - `bind()` - Handler map에 저장
3. `include/fb/async_executor.h` - `bind_thread_timer()`, `bind_timer()` - Timer에 저장

### 우선순위 2: 저장 가능성 높은 경우 (확인 후 변경)
1. `include/fb/thread_container.h` - `enqueue()` - Thread에 전달
2. `include/fb/bot/bot.h` - `base_bot()` - 멤버 변수 저장 가능성
3. `include/fb/game/worker.h` - `on_work()` - 저장 가능성 확인 필요

### 우선순위 3: 코루틴 사용 확인 (유지 또는 변경)
1. `include/fb/game/channel/*_channel.h` - `read_async()` - 코루틴 사용 확인
2. `include/fb/game/character.h` - `dispatch()` - 코루틴 사용 확인

---

## 변경 가이드

### `const std::function&` → `std::function&&` 변경 예시

**변경 전:**
```cpp
void enqueue(const handle_func_type<void>& fn,
             const handle_error_type& error,
             const std::function<void()>& callback);
```

**변경 후:**
```cpp
void enqueue(handle_func_type<void>&& fn,
             handle_error_type&& error,
             std::function<void()>&& callback);
```

**구현부 변경:**
```cpp
// 변경 전:
void fb::thread::enqueue(const handle_func_type<void>& fn,
                         const handle_error_type&      error,
                         const std::function<void()>&  callback)
{
    this->_queue.write([=, this](auto& queue) {
        queue.push([=, this]() {
            async::awaitable_then(fn(*this), [=](async::awaitable_result<void> result) {
                // ... callback(), error(e) 호출 ...
            });
        });
    });
}

// 변경 후:
void fb::thread::enqueue(handle_func_type<void>&& fn,
                         handle_error_type&&      error,
                         std::function<void()>&& callback)
{
    this->_queue.write([fn = std::move(fn), error = std::move(error), callback = std::move(callback), this](auto& queue) {
        queue.push([fn = std::move(fn), error = std::move(error), callback = std::move(callback), this]() {
            async::awaitable_then(fn(*this), [fn = std::move(fn), error = std::move(error), callback = std::move(callback)](async::awaitable_result<void> result) {
                // ... callback(), error(e) 호출 ...
            });
        });
    });
}
```

**호출부 변경:**
```cpp
// 변경 전:
thread.enqueue(func, error, callback);

// 변경 후:
thread.enqueue(std::move(func), std::move(error), std::move(callback));
```

### `by value` → `std::function&&` 변경 예시

**헤더 변경:**
```cpp
// 변경 전:
void bind_thread_timer(std::function<async::task<void>(const fb::model::datetime&, std::thread::id)> fn,
                       const std::chrono::steady_clock::duration& duration);

// 변경 후:
void bind_thread_timer(std::function<async::task<void>(const fb::model::datetime&, std::thread::id)>&& fn,
                       const std::chrono::steady_clock::duration& duration);
```

**구현부 변경:**
```cpp
// 변경 전:
void async_executor::bind_thread_timer(std::function<async::task<void>(const fb::model::datetime&, std::thread::id)> fn,
                                       const std::chrono::steady_clock::duration& duration)
{
    // fn을 복사하여 저장
    this->threads.enqueue(...);
    thread->settimer(fn, duration);
}

// 변경 후:
void async_executor::bind_thread_timer(std::function<async::task<void>(const fb::model::datetime&, std::thread::id)>&& fn,
                                       const std::chrono::steady_clock::duration& duration)
{
    // fn을 move하여 저장
    this->threads.enqueue(...);
    thread->settimer(std::move(fn), duration);
}
```

### `by value` → `const std::function&` 변경 예시 (즉시 실행)

**헤더 변경:**
```cpp
// 변경 전:
void on_work(const std::function<async::task<void>()>& value);

// 변경 후:
void on_work(const std::function<async::task<void>()>& value);  // 이미 const_ref이므로 변경 불필요
// 또는 by_value인 경우:
void on_work(std::function<async::task<void>()> value);  // 변경 전

void on_work(const std::function<async::task<void>()>& value);  // 변경 후
```

**구현부 변경:**
```cpp
// 변경 전:
void script_loader::on_work(const std::function<async::task<void>()>& value)
{
    async::awaitable_get(value());  // 즉시 실행
}

// 변경 후: (동일, const_ref이므로 복사 없이 참조로 사용)
void script_loader::on_work(const std::function<async::task<void>()>& value)
{
    async::awaitable_get(value());  // 즉시 실행
}
```

---

## 주의사항

1. **코루틴 생명주기**: 코루틴 내에서 함수를 나중에 사용하는 경우 `by value` 유지
2. **호출부 수정**: `std::function&&`로 변경 시 모든 호출부에서 `std::move()` 사용 필요
3. **테스트**: 변경 후 반드시 테스트 수행
4. **점진적 변경**: 한 번에 모든 파일을 변경하지 말고, 우선순위에 따라 단계적으로 변경

---

## 진행 상황

- [x] 우선순위 1 파일 변경 완료
  - [x] `include/fb/thread.h` - `enqueue()`, `settimer()`, `dispatch()` 변경 완료
  - [x] `include/fb/bot/controller.h` - `bind()` 변경 완료
  - [x] `include/fb/async_executor.h` - `bind_thread_timer()`, `bind_timer()` 변경 완료
- [x] 우선순위 2 파일 확인 및 변경 완료
  - [x] `include/fb/thread_container.h` - `enqueue()`, `dispatch()` 변경 완료
  - [x] `include/fb/bot/bot.h` / `server/bot/lib/bot.cpp` - `base_bot()` 변경 완료
- [x] 코루틴 생명주기 함수들 변경 완료
  - [x] `include/fb/game/character.h` - `foreach()`, `foreach_async()` 변경 완료 (`&&` → `by value`)
- [x] 우선순위 3 파일 확인 및 변경 완료
  - [x] `include/fb/amqp/queue.h` - `handler()` 변경 완료
  - [x] `include/fb/bot/integration/test_case.h` - `parallel_scenarios()` 변경 완료
- [x] 모든 주요 변경 사항 완료
- [ ] 테스트 완료 (사용자 확인 필요)

## 변경 이력

### 2024-12-19: 우선순위 1 변경 완료

**변경된 파일:**
1. `include/fb/thread.h` / `server/fb/src/thread.cpp`
   - `settimer()`: `const_ref` → `&&`
   - `enqueue()` (void 버전): `const_ref` → `&&` (모든 파라미터)
   - `enqueue()` (template 버전): `const_ref` → `&&` (모든 파라미터)
   - `dispatch()`: `const_ref` → `&&` (문서에 없었지만 `enqueue()`를 호출하므로 함께 변경)

2. `include/fb/bot/controller.h`
   - `bind()`: `const_ref` → `&&`
   - `bind()` (멤버 함수 포인터 오버로드): `std::bind()` 결과를 `std::function`으로 명시적 변환

3. `include/fb/async_executor.h`
   - `bind_thread_timer()`: `by_value` → `&&`
   - `bind_timer()`: `by_value` → `&&`

**구현부 변경 사항:**
- Lambda capture에서 `[=, this]` → `[fn = std::move(fn), error = std::move(error), callback = std::move(callback), this]` 패턴 사용
- 중첩된 lambda에서도 동일하게 move 적용
- `settimer()` 내부 lambda에서 `[this, fn]` → `[this, fn = std::move(fn)]` 패턴 사용
- `bind_timer()` 내부 lambda에서 `[weak_this, fn, interval]` → `[weak_this, fn = std::move(fn), interval]` 패턴 사용

**호출부 확인 및 수정:**
- 대부분의 호출부는 lambda를 직접 전달하므로 `std::move()` 불필요
- `server/fb/src/thread_container.cpp:132`의 `settimer()` 호출 수정 완료
  - 문제: `thread.settimer(fn, duration)` 호출 시 `fn`을 move하지 않음
  - 수정: `thread.settimer(std::move(fn), duration)`로 변경
  - 추가: lambda capture도 `[fn]` → `[fn = std::move(fn)]`으로 변경 (각 thread에 복사본 생성)

### 2024-12-19: 우선순위 2 변경 완료

**변경된 파일:**
1. `include/fb/thread_container.h`
   - `enqueue()` (모든 오버로드): `by_value`/`const_ref` → `&&` (fn, error, callback)
   - `dispatch()` (모든 오버로드): `const_ref` → `&&` (fn)
   - `condition` 파라미터는 즉시 실행되므로 `const_ref` 유지
   - 내부 lambda에서 `fn = std::move(fn)` 패턴 사용

2. `include/fb/bot/bot.h` / `server/bot/lib/bot.cpp`
   - `base_bot()` 생성자: `on_receive`, `on_closed` `by_value` → `&&`
   - `fb::socket` 생성자에 `std::move()`로 전달
   - **참고**: `fb::socket` 생성자도 `const_ref` → `&&`로 변경하면 완전히 최적화됨 (체크리스트에 없음)

**구현부 변경 사항:**
- `thread_container::enqueue()` 내부 lambda에서 `fn`, `error`, `callback`을 move하여 `thread->enqueue()`에 전달
- `base_bot()` 생성자에서 `on_receive`, `on_closed`를 `std::move()`로 `fb::socket` 생성자에 전달

**추가 발견 사항:**
- `include/fb/socket.h` (line 112, 121): `socket()` 생성자가 `const_ref`를 받아 멤버 변수에 저장
  - `_handle_received`, `_handle_closed`에 저장되므로 `&&`로 변경하는 것이 좋음
  - 하지만 체크리스트에 없으므로 별도로 처리 필요
  - `base_bot()`는 이미 `&&`로 변경했으므로, `fb::socket` 생성자도 변경하면 완전히 최적화됨

### 2024-12-19: 즉시 실행용 함수들 변경 완료

**변경된 파일:**
1. `include/fb/game/character.h` / `server/game/lib/object/character.container.cpp`
   - `invoke()`: `by_value` 유지 (초기 `const_ref` 변경은 오류였음, `co_await` 후 사용하므로 생명주기 문제)
   - `invoke_async()`: `by_value` 유지 (초기 `const_ref` 변경은 오류였음, `co_await` 후 사용하므로 생명주기 문제)

2. `include/fb/game/channel/storage_pending_channel.h` / `server/game/lib/channel/storage_pending_channel.cpp`
   - `write()`: `by_value` → `const_ref`

3. `include/fb/lua.h` / `server/fb/src/lua.cpp`
   - `ensure_yield()`: `by_value` 유지 (초기 `const_ref` 변경은 오류였음, `async::awaitable_then` lambda capture 후 사용)
   - `ensure_resume()`: `by_value` 유지 (초기 `const_ref` 변경은 오류였음, `async::awaitable_then` lambda capture 후 사용)

**체크리스트 오류 수정:**
- `include/fb/game/worker.h` (64-66): 이미 `const_ref`로 되어 있음
- `server/bot/lib/controller.cpp` (62): 이미 `&&`로 되어 있음
- `server/game/lib/worker/worker.script.cpp` (100, 105, 110): 이미 `const_ref`로 되어 있음
- `server/game/lib/server/server.group.cpp` (213): 로컬 변수 타입 정의이므로 변경 불필요
- `server/fb/src/thread.cpp` (21): 로컬 변수이므로 변경 불필요

### 2024-12-19: 코루틴 생명주기 함수들 변경 완료

**변경된 파일:**
1. `include/fb/game/character.h` / `server/game/lib/object/character.container.cpp`
   - `foreach()` (모든 오버로드): `&&` → `by value`
   - `foreach_async()` (모든 오버로드): `&&` → `by value`
   - Lambda `[fn]` capture 후 `co_await`로 전달하므로 `by value`로 받아서 안전하게 복사

**구현부 변경 사항:**
- `foreach()` 내부에서 lambda `[fn]`으로 capture하여 `foreach_async()`에 전달
- `foreach_async()` 내부에서 `co_await fn(shared_ptr)` 직접 호출
- `by value`로 받아서 lambda에 복사본을 저장하므로 코루틴 suspend 시에도 안전
- 내부에서 다른 `foreach_async()`를 호출할 때는 `std::move(fn)` 사용 (이미 복사본이므로 move 가능)

**추가 수정 사항 (2024-12-19):**
- `invoke()`, `invoke_async()`: 초기에는 `const_ref`로 변경했으나, `co_await this->_server.threads.switching(weak)` 후에 `fn`을 사용하므로 `by value`로 유지해야 함
  - `co_await`로 suspend된 후 `fn`을 사용하므로, 임시 객체가 파괴될 수 있음
  - `foreach()`, `foreach_async()`와 동일한 이유로 `by value`가 올바름

---

**최종 업데이트**: 2024-12-19 (모든 주요 변경 사항 완료)
**총 항목 수**: 115개
- ✅ 적절한 사용: 49개 (즉시 실행용 3개, 코루틴 생명주기 3개 추가)
  - 즉시 실행용 (`const std::function&`): 36개 (1개 추가: `write()`)
  - 코루틴 생명주기 (`by value` 유지): 12개 (7개 추가: `foreach()`, `foreach_async()`, `invoke()`, `invoke_async()`, `ensure_yield()`, `ensure_resume()`)
  - 이미 rvalue ref 사용 (`std::function&&`): 1개 (`foreach_enqueue`)
  - Optional (`std::function*`): 2개
- ✅ 변경 완료: 모든 주요 항목 변경 완료
  - 저장용 → `std::function&&`: 완료 (10개 함수 변경)
  - 즉시 실행용 → `const std::function&`: 완료 (1개 함수 변경)
  - 코루틴 생명주기 → `by value`: 완료 (6개 함수 확인/변경)

## 주요 발견 사항

### 1. 저장되는 함수들 (→ `std::function&&` 필요)
- `thread::enqueue()` - Queue에 저장 (`queue.push`)
- `thread::settimer()` - Timer 객체에 저장 (`_timers.push_back`)
- `controller::bind()` - Handler map에 저장 (`_handler.insert`)
- `bot::base_bot()` - Socket 멤버 변수에 저장 (`_handle_received`, `_handle_closed`)
- `async_executor::bind_thread_timer()` - Timer에 저장 (`settimer`)
- `amqp::queue::handler()` - Handler map에 저장 (`_handler.insert`)
- `game_bot::pattern_params` - Vector에 저장 (`_pattern_params`)
- `test_case::scenario_t` - Queue에 저장 (`_scenario_queue.push`)
- `skill_test::test_function` - Vector에 저장 (`_test_functions`)
- 테스트 코드 구조체 멤버들 - 구조체 멤버로 저장됨

### 2. 즉시 실행 함수들 (→ `const std::function&` 변경)
- `worker::on_work()` - `async::awaitable_get()`로 즉시 실행
- `map::movable()` - Predicate 즉시 실행
- `table::load()` - Callback 즉시 실행
- `bot_container::dispatch()` - `co_await thread->dispatch(fn)` 즉시 실행
- `lua::ensure_yield()`, `ensure_resume()` - `fn()` 즉시 실행
- `server::handle_group_action()` - 로컬 변수로만 사용
- `character::container::invoke()`, `invoke_async()` - `fn(ch)`, `co_await fn(ch)` 즉시 실행
- `storage_pending_channel::write()` - `locker::write()`에 전달하여 즉시 실행

### 3. 코루틴 생명주기 문제 (→ `by value` 유지 또는 변경)
**유지 필요 (현재 `by value`)**:
- `storage_pending_channel::read_async()` - `co_await` 내에서 사용
- `system_mail_channel::read_async()` - `co_await` 내에서 사용
- `locker::enter()` - 코루틴 내에서 즉시 실행 (`co_await func()`)

**변경 필요 (현재 `&&` 사용 중)**:
- `character::container::foreach()` - Lambda `[fn]` capture 후 `co_await`로 전달, 생명주기 문제
- `character::container::foreach_async()` - `co_await fn()` 직접 사용, 생명주기 문제
- `character::container::foreach()`, `foreach_async()` (names) - Lambda capture 후 `co_await`로 전달

### 4. 추가 발견 및 수정 사항
**저장되는 경우 (→ `std::function&&` 필요)**:
- `test_case::scenario_t` - `_scenario_queue`에 저장 (`queue.push`)
- `skill_test::test_function` - `_test_functions` vector에 저장
- 테스트 코드 구조체 멤버들 (`calculator`, `condition_check` 등) - 구조체 멤버로 저장

**즉시 실행되는 경우 (→ `const std::function&` 변경)**:
- `character::container::invoke()` - `fn(ch)` 즉시 실행
- `character::container::invoke_async()` - `co_await fn(ch)` 즉시 실행
- `storage_pending_channel::write()` - `locker::write()`에 전달하여 즉시 실행

**코루틴 생명주기 문제 (→ `by value` 변경)**:
- `character::container::foreach()` - Lambda `[fn]` capture 후 `co_await`로 전달 (현재 `&&` 사용 중)
- `character::container::foreach_async()` - `co_await fn()` 직접 사용 (현재 `&&` 사용 중)

**수정 사항**:
- `storage_pending_channel::write()` - 이전에 `ref`로 잘못 분류되었으나, 실제로는 `by value`이며 `const std::function&`로 변경 가능
- `character::container::foreach()` - 이전에 `&&` 유지로 분류되었으나, 코루틴 생명주기 문제로 `by value`로 변경 필요
