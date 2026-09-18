# HW1 — Lifecycle Fake IMU + Watchdog

## 1. 패키지 구조
`fake_imu`는 `rate_hz`, `stamp_offset_sec` 파라미터를 갖는 lifecycle 노드로 `/imu`를 발행한다. `watchdog`(노드 이름: `imu_watchdog`)은 `timeout_sec`, `check_rate_hz` 파라미터로 `/imu`를 감시하고 `/imu_checked`로 재발행한다.

## 2. Lifecycle 콜백 설계
퍼블리셔·구독·클라이언트는 `on_configure`에서 생성하고, 타이머는 `on_activate`에서 생성해 `on_deactivate`에서 해제한다. `LifecycleNode::on_activate(state)`를 호출해 `LifecyclePublisher`를 자동 활성화하도록 구현했다.

## 3. 체크포인트 1~4
Unconfigured 상태에서는 `/imu`에 데이터가 없고, activate 후 `rate_hz` 기준 약 2Hz로 발행되는 것을 확인했다. `timeout_sec=0`으로 설정 시 `on_configure`가 FAILURE를 반환해 Unconfigured에 머무는 것을 확인했고, fake_imu를 deactivate하면 `[STALE]` 경고가, 다시 activate하면 `[RECOVERED]`가 정확히 한 번 출력됐다.

## 4. 과제 A — imu_watchdog 상태 확인 타이머

### 4-1. 블로킹 구현 (의도적 실패)
`future.get()`으로 서비스 응답을 블로킹 대기하도록 구현하니, `on_activate` 이후 모든 로그와 `ros2 lifecycle get /imu_watchdog` 응답이 완전히 멈췄다. SingleThreadedExecutor의 유일한 스레드가 응답 대기에 갇혀 스스로를 막는 데드락이 원인이다.

![블로킹 데드락 증거](/img/2-1.png)

<video src="/video/2-1.mp4" controls></video>

### 4-2. 비동기 수정
`async_send_request(request, 콜백)`으로 바꾸자 `fake_imu state: active` 로그가 1초 간격으로 정상 출력되고, `ros2 lifecycle get`도 즉시 응답했다. 요청 전송과 응답 처리가 분리되어 executor 스레드를 막지 않기 때문이다.

<video src="/video/2-2.mp4" controls></video>

### 4-3. 상태 따라가기
fake_imu를 deactivate → cleanup → configure → activate 순서로 전이시키자, 워치독의 `fake_imu state:` 로그가 각 전이를 1초 이내로 따라갔다. activate 직후에는 `[RECOVERED]` 로그가 정확히 한 번만 출력되어 age 검사와 상태 확인 타이머가 서로 간섭 없이 동작함을 확인했다.

<video src="/video/2-3.mp4" controls></video>

### 4-4. 선택 과제 (callback group + MultiThreadedExecutor)
미실행.

## 5. 체크포인트 5~7 (bag / sim time / wall timer)
미실행.

## 6. 체크포인트 8 (launch 자동화)
이번 제출에서는 제외했다.

## 7. 트러블슈팅
`create_timer`는 `rclcpp::Duration`이 아닌 `std::chrono::duration`을 요구해 타입 오류가 났고, age 판정은 수신 시각이 아닌 `header.stamp` 기준이어야 `stamp_offset_sec` 트릭을 감지할 수 있었다. 옛 빌드 캐시로 실행 파일명이 갱신되지 않아 `build`/`install` 삭제 후 재빌드로 해결했다.
