# std::function 파라미터 사용 가이드라인

## 분석 결과 요약

- **총 119개**의 `std::function` 파라미터 사용 발견
- **45개 파일**에서 사용 중
- **분포**:
  - `by_value`: 72개 (60.5%)
  - `const_ref`: 35개 (29.4%)
  - `ref`: 10개 (8.4%)
  - `pointer`: 2개 (1.7%)
  - `rvalue_ref`: 0개 (0%)

## 권장 사항

### 1. `const std::function&` (const reference) - **권장**

**사용 시기:**
- 함수가 **한 번만 호출**되거나 **여러 번 호출**되지만 저장하지 않는 경우
- Predicate 함수 (조건 검사용)
- Callback이 즉시 실행되고 끝나는 경우

**장점:**
- 복사 오버헤드 없음
- 의도가 명확함 (읽기 전용)
- lvalue와 rvalue 모두 받을 수 있음

**단점:**
- 파라미터에서 move할 수 없음

**현재 사용 예시:**
```cpp
// include/fb/game/map.h:60
bool movable(const fb::model::point16_t& position, 
             const std::function<bool(const object&)>& predicate) const;

// include/fb/table.h:21
uint32_t load(std::string_view path, 
              const handle_callback& callback, 
              const handle_error& error, 
              bool async = true);
```

**개선 제안:**
현재 `by_value`로 사용 중인 즉시 실행 콜백들을 `const std::function&`로 변경 권장

---

### 2. `std::function&&` (rvalue reference) - **새로 도입 권장**

**사용 시기:**
- 함수가 **멤버 변수로 저장**되거나 **컨테이너에 저장**되는 경우
- 함수가 **한 번만 사용**되고 이후 필요 없는 경우
- 성능이 중요한 경로에서 불필요한 복사를 피하고 싶은 경우

**장점:**
- Move semantics로 효율적
- 불필요한 복사 방지
- 저장 시 최적화

**단점:**
- 호출자가 `std::move()`를 사용해야 함
- 현재 코드베이스에서 사용되지 않음 (0개)

**권장 적용 예시:**
```cpp
// 현재: include/fb/thread.h:92-94
void enqueue(const handle_func_type<void>& fn,
             const handle_error_type& error,
             const std::function<void()>& callback);

// 개선 제안:
void enqueue(handle_func_type<void>&& fn,
             handle_error_type&& error,
             std::function<void()>&& callback);
```

**적용 대상:**
- `include/fb/thread.h` - `enqueue()` 함수들
- `include/fb/thread_container.h` - `enqueue()` 함수들
- `include/fb/game/worker.h` - `on_work()` 함수들
- `include/fb/bot/controller.h` - `bind()` 함수

---

### 3. `std::function` (by value) - **제한적 사용**

**사용 시기:**
- 함수가 **멤버 변수로 복사 저장**되어야 하는 경우
- Template 함수에서 타입 추론이 필요한 경우
- 간단한 유틸리티 함수

**장점:**
- 사용이 간단함
- lvalue와 rvalue 모두 자동 처리
- Template과 잘 맞음

**단점:**
- 불필요한 복사 발생 가능
- 성능 오버헤드

**현재 사용 예시:**
```cpp
// include/fb/async_executor.h:87
void bind_thread_timer(std::function<async::task<void>(const fb::model::datetime&, std::thread::id)> fn,
                       const std::chrono::steady_clock::duration& duration);

// include/fb/game/worker.h:64-66
void on_work(const std::function<async::task<void>()>& value) override final;
```

**개선 제안:**
저장되지 않고 즉시 사용되는 경우 → `const std::function&`로 변경
저장되는 경우 → `std::function&&`로 변경

---

### 4. `std::function&` (non-const reference) - **특수한 경우만**

**사용 시기:**
- 함수 내부에서 **파라미터를 수정**해야 하는 경우
- Callback이 **상태를 변경**해야 하는 경우

**장점:**
- 파라미터 수정 가능

**단점:**
- 사용 사례가 제한적
- 의도가 불명확할 수 있음

**현재 사용 예시:**
```cpp
// include/fb/game/channel/storage_pending_channel.h:30
void write(std::function<void(pending_map&)> fn);

// include/fb/table.h:17-18
using handle_callback = std::function<void(Json::Value&, Json::Value&, double)>;
using handle_error = std::function<void(Json::Value&, Json::Value&, std::string_view error)>;
```

**권장:**
현재 사용은 적절함. Json::Value나 pending_map을 수정해야 하므로 `ref` 사용이 맞음.

---

### 5. `std::function*` (pointer) - **Optional callback용**

**사용 시기:**
- Callback이 **선택적(optional)**인 경우
- nullptr로 callback 없음을 표현해야 하는 경우

**장점:**
- Optional 표현 가능
- nullptr 체크로 안전성 확보

**단점:**
- nullptr 체크 필요
- 포인터 역참조 오버헤드

**현재 사용 예시:**
```cpp
// include/fb/thread.h:47, 76
std::unique_ptr<void, std::function<void(void*)>> _data;
```

**권장:**
현재 사용은 적절함. `std::optional<std::function<...>>`도 고려 가능하지만, 포인터가 더 간단함.

---

## 구체적인 개선 제안

### 우선순위 1: 즉시 실행 콜백 → `const std::function&`

**대상 파일:**
- `include/fb/game/map.h:60` - `movable()` predicate
- `include/fb/bot/hook_params.h:12-13` - condition/matched callbacks
- `include/fb/mst.h:87` - `travel()` predicate

**변경 전:**
```cpp
bool movable(const fb::model::point16_t& position, 
             const std::function<bool(const object&)>& predicate) const;
```

**변경 후:** (이미 `const_ref`이므로 변경 불필요)

---

### 우선순위 2: 저장되는 함수 → `std::function&&`

**대상 파일:**
- `include/fb/thread.h:92-94, 160` - `enqueue()` 함수들
- `include/fb/thread_container.h:105, 193, 228` - `enqueue()` 함수들
- `include/fb/game/worker.h:64-66` - `on_work()` 함수들
- `include/fb/bot/controller.h:281` - `bind()` 함수

**변경 예시:**
```cpp
// include/fb/thread.h:92-94
// 변경 전:
void enqueue(const handle_func_type<void>& fn,
             const handle_error_type& error,
             const std::function<void()>& callback);

// 변경 후:
void enqueue(handle_func_type<void>&& fn,
             handle_error_type&& error,
             std::function<void()>&& callback);
```

**호출부 변경:**
```cpp
// 변경 전:
thread.enqueue(func, error, callback);

// 변경 후:
thread.enqueue(std::move(func), std::move(error), std::move(callback));
```

---

### 우선순위 3: Template 함수 → Perfect Forwarding 고려

**대상:**
- `include/fb/thread_container.h` - Template `enqueue()` 함수들

**개선 제안:**
Template 함수의 경우 Perfect Forwarding을 사용하여 lvalue는 참조로, rvalue는 move로 처리:

```cpp
template <typename Func>
void enqueue(Func&& fn) {
    // lvalue: 복사, rvalue: move
    _queue.push(std::forward<Func>(fn));
}
```

하지만 `std::function`의 경우 타입 삭제가 필요하므로, 현재 방식 유지하고 `std::function&&` 사용 권장.

---

## 요약 권장사항

1. **즉시 실행 콜백**: `const std::function&` 사용
2. **저장되는 함수**: `std::function&&` 사용 (새로 도입)
3. **수정되는 파라미터**: `std::function&` 유지
4. **Optional callback**: `std::function*` 유지
5. **Template 함수**: 현재 `by_value` 유지하되, 가능하면 `std::function&&` 고려

## 성능 고려사항

- `std::function`은 타입 삭제(type erasure)를 사용하므로 복사 비용이 큼
- 작은 함수 객체는 `std::function` 대신 template parameter로 받는 것이 더 효율적
- 하지만 타입 삭제가 필요한 경우 `std::function` 사용이 적절함

## 마이그레이션 전략

1. **1단계**: 새로운 코드부터 `std::function&&` 사용
2. **2단계**: 저장되는 함수 파라미터를 `std::function&&`로 변경
3. **3단계**: 즉시 실행 콜백을 `const std::function&`로 통일
4. **4단계**: 기존 `by_value` 사용 검토 및 최적화
