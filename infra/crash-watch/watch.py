import json
import os
import sys
import urllib.error
import urllib.request

WATCH_APPS = {"game", "login", "gateway"}
COMPOSE_SERVICES = {"game", "login", "gateway", "game-cross"}
NAMESPACE = os.environ.get("WATCH_NAMESPACE", "fb")
WEBHOOK = os.environ.get("DISCORD_WEBHOOK_URL", "").strip()

seen = set()


def incoming_webhook_url(url):
    if url.endswith("/github"):
        return url[:-7]
    return url


def crash_excerpt(log_text):
    start = log_text.rfind("*** FB CRASH ***")
    if start < 0:
        return log_text[-4000:]
    end = log_text.find("*** END CRASH ***", start)
    if end >= 0:
        return log_text[start : end + len("*** END CRASH ***")]
    return log_text[start:]


def notify(title, body):
    print(title, flush=True)
    print(body, flush=True)
    if WEBHOOK == "":
        return

    content = title + "\n```\n" + body[:1800] + "\n```"
    payload = json.dumps({"content": content}).encode("utf-8")
    request = urllib.request.Request(
        incoming_webhook_url(WEBHOOK),
        data=payload,
        headers={"Content-Type": "application/json"},
        method="POST",
    )
    try:
        urllib.request.urlopen(request, timeout=10).read()
    except urllib.error.URLError as e:
        print("discord post failed: {}".format(e), file=sys.stderr, flush=True)


def pod_logs(api, name, container, previous):
    try:
        return api.read_namespaced_pod_log(
            name=name,
            namespace=NAMESPACE,
            container=container,
            previous=previous,
            tail_lines=200,
            timestamps=False,
        )
    except Exception as e:
        return "failed to read logs: {}".format(e)


def check_container(api, pod, status):
    terminated = None
    if status.state and status.state.waiting and status.state.waiting.reason == "CrashLoopBackOff":
        if status.last_state and status.last_state.terminated:
            terminated = status.last_state.terminated
    elif status.state and status.state.terminated:
        terminated = status.state.terminated

    if terminated is None:
        return
    if terminated.exit_code == 0:
        return
    if terminated.reason == "Completed":
        return

    key = "{}:{}:{}".format(pod.metadata.uid, status.name, status.restart_count)
    if key in seen:
        return
    seen.add(key)

    log_text = pod_logs(api, pod.metadata.name, status.name, previous=status.restart_count > 0)
    excerpt = crash_excerpt(log_text)
    title = "crash {}/{} container={} exit={} reason={}".format(
        NAMESPACE,
        pod.metadata.name,
        status.name,
        terminated.exit_code,
        terminated.reason or "unknown",
    )
    notify(title, excerpt)


def watch_kubernetes():
    from kubernetes import client, config, watch

    try:
        config.load_incluster_config()
    except config.ConfigException:
        config.load_kube_config()

    api = client.CoreV1Api()
    w = watch.Watch()
    print(
        "watching pods in namespace {} (webhook {})".format(NAMESPACE, "enabled" if WEBHOOK else "disabled"),
        flush=True,
    )

    for event in w.stream(api.list_namespaced_pod, namespace=NAMESPACE):
        pod = event["object"]
        labels = pod.metadata.labels or {}
        app = labels.get("app", "")
        if app not in WATCH_APPS:
            continue
        if pod.status is None or pod.status.container_statuses is None:
            continue
        for status in pod.status.container_statuses:
            check_container(api, pod, status)


def watch_docker():
    import docker

    docker_client = docker.from_env()
    print("watching docker container deaths (webhook {})".format("enabled" if WEBHOOK else "disabled"), flush=True)

    for event in docker_client.events(decode=True, filters={"type": "container", "event": ["die", "oom"]}):
        attrs = event.get("Actor", {}).get("Attributes", {})
        service = attrs.get("com.docker.compose.service", "")
        if service not in COMPOSE_SERVICES:
            continue

        exit_code = attrs.get("exitCode", "")
        if exit_code == "0":
            continue

        container_id = event.get("id", "")
        key = "{}:{}:{}".format(container_id, event.get("status", ""), exit_code)
        if key in seen:
            continue
        seen.add(key)

        log_text = ""
        try:
            container = docker_client.containers.get(container_id)
            log_text = container.logs(stdout=True, stderr=True, tail=200).decode("utf-8", errors="replace")
        except Exception as e:
            log_text = "failed to read logs: {}".format(e)

        name = attrs.get("name", container_id)
        title = "crash docker service={} name={} exit={} status={}".format(
            service,
            name,
            exit_code or "unknown",
            event.get("status", "unknown"),
        )
        notify(title, crash_excerpt(log_text))


def main():
    if os.environ.get("KUBERNETES_SERVICE_HOST"):
        watch_kubernetes()
        return

    if os.path.exists("/var/run/docker.sock"):
        watch_docker()
        return

    watch_kubernetes()


if __name__ == "__main__":
    main()
