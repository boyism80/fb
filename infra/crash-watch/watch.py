import json
import os
import sys
import time
import urllib.error
import urllib.request

WATCH_APPS = {"game", "login", "gateway"}
COMPOSE_SERVICES = {"game", "login", "gateway", "game-cross"}
NAMESPACE = os.environ.get("WATCH_NAMESPACE", "fb")
BOT_TOKEN = os.environ.get("DISCORD_BOT_TOKEN", "").strip()
if BOT_TOKEN.startswith("Bot "):
    BOT_TOKEN = BOT_TOKEN[4:]
CHANNEL_ID = os.environ.get("DISCORD_CHANNEL_ID", "").strip()
DISCORD_ENABLED = BOT_TOKEN != "" and CHANNEL_ID != ""

DISCORD_BODY_LIMIT = 1800
REPORTED_ANNOTATION = "crash-watch.fb/"
RESTART_DELAY_SECONDS = 5

seen = set()


def crash_excerpt(log_text):
    start = log_text.rfind("*** FB CRASH ***")
    if start >= 0:
        end = log_text.find("*** END CRASH ***", start)
        if end >= 0:
            block = log_text[start : end + len("*** END CRASH ***")]
        else:
            block = log_text[start:]
        return block[:DISCORD_BODY_LIMIT]

    asan = log_text.rfind("ERROR: AddressSanitizer:")
    if asan < 0:
        asan = log_text.rfind("==ERROR: AddressSanitizer:")
    if asan < 0:
        asan = log_text.rfind("ERROR: LeakSanitizer:")
    if asan >= 0:
        return log_text[asan : asan + DISCORD_BODY_LIMIT]

    summary = log_text.rfind("SUMMARY: AddressSanitizer:")
    if summary < 0:
        summary = log_text.rfind("SUMMARY: LeakSanitizer:")
    if summary >= 0:
        begin = summary - (DISCORD_BODY_LIMIT - 120)
        if begin < 0:
            begin = 0
        return log_text[begin : begin + DISCORD_BODY_LIMIT]

    return log_text[-DISCORD_BODY_LIMIT:]


def notify(title, body):
    print(title, flush=True)
    print(body, flush=True)
    if DISCORD_ENABLED == False:
        return True

    content = title + "\n```\n" + body[:DISCORD_BODY_LIMIT] + "\n```"
    payload = json.dumps({"content": content}).encode("utf-8")
    request = urllib.request.Request(
        "https://discord.com/api/v10/channels/{}/messages".format(CHANNEL_ID),
        data=payload,
        headers={
            "Authorization": "Bot " + BOT_TOKEN,
            "Content-Type": "application/json",
            "User-Agent": "fb-crash-watch/1.0",
        },
        method="POST",
    )
    try:
        urllib.request.urlopen(request, timeout=10).read()
        return True
    except urllib.error.HTTPError as e:
        body = e.read().decode("utf-8", errors="replace")[:500]
        print("discord post failed: {} {}".format(e, body), file=sys.stderr, flush=True)
        return False
    except urllib.error.URLError as e:
        print("discord post failed: {}".format(e), file=sys.stderr, flush=True)
        return False


def pod_logs(api, name, container, previous):
    try:
        return api.read_namespaced_pod_log(
            name=name,
            namespace=NAMESPACE,
            container=container,
            previous=previous,
            tail_lines=1000,
            timestamps=False,
        )
    except Exception as e:
        text = "{}".format(e)
        if "(404)" in text or "not found" in text.lower():
            return None
        return "failed to read logs: {}".format(e)


def last_terminated_id(pod, status):
    terminated = status.last_state.terminated if status.last_state else None
    if terminated is None:
        return None
    if terminated.container_id:
        return terminated.container_id
    return "{}:{}:{}".format(pod.metadata.uid, status.name, status.restart_count)


def check_container(api, pod, status):
    terminated = status.last_state.terminated if status.last_state else None
    if terminated is None:
        return
    if terminated.exit_code == 0:
        return
    if terminated.reason == "Completed":
        return

    container_id = last_terminated_id(pod, status)
    annotation = REPORTED_ANNOTATION + status.name
    annotations = pod.metadata.annotations or {}
    if container_id in seen or annotations.get(annotation) == container_id:
        return

    log_text = pod_logs(api, pod.metadata.name, status.name, previous=True)
    if log_text is None:
        return

    excerpt = crash_excerpt(log_text)
    try:
        current = api.read_namespaced_pod(name=pod.metadata.name, namespace=NAMESPACE)
        for current_status in current.status.container_statuses or []:
            if current_status.name == status.name and last_terminated_id(current, current_status) != container_id:
                excerpt = "(crashed again while reading; this log may be a later instance)\n" + excerpt
    except Exception as e:
        print("pod re-read failed: {}".format(e), file=sys.stderr, flush=True)

    title = "crash {}/{} container={} exit={} reason={}".format(
        NAMESPACE,
        pod.metadata.name,
        status.name,
        terminated.exit_code,
        terminated.reason or "unknown",
    )
    if notify(title, excerpt) == False:
        return

    seen.add(container_id)
    try:
        api.patch_namespaced_pod(
            name=pod.metadata.name,
            namespace=NAMESPACE,
            body={"metadata": {"annotations": {annotation: container_id}}},
        )
    except Exception as e:
        print("annotation patch failed: {}".format(e), file=sys.stderr, flush=True)


def check_pod(api, pod):
    labels = pod.metadata.labels or {}
    app = labels.get("app", "")
    if app not in WATCH_APPS:
        return
    if pod.metadata.deletion_timestamp is not None:
        return
    if pod.status is None or pod.status.container_statuses is None:
        return
    for status in pod.status.container_statuses:
        check_container(api, pod, status)


def watch_kubernetes():
    from kubernetes import client, config, watch

    try:
        config.load_incluster_config()
    except config.ConfigException:
        config.load_kube_config()

    api = client.CoreV1Api()
    print(
        "watching pods in namespace {} (discord {})".format(NAMESPACE, "enabled" if DISCORD_ENABLED else "disabled"),
        flush=True,
    )

    while True:
        try:
            pods = api.list_namespaced_pod(namespace=NAMESPACE)
            for pod in pods.items:
                check_pod(api, pod)

            w = watch.Watch()
            for event in w.stream(
                api.list_namespaced_pod, namespace=NAMESPACE, resource_version=pods.metadata.resource_version
            ):
                if event.get("type") == "DELETED":
                    continue
                check_pod(api, event["object"])
        except Exception as e:
            print("watch restarting: {}".format(e), file=sys.stderr, flush=True)
            time.sleep(RESTART_DELAY_SECONDS)


def watch_docker():
    import docker

    docker_client = docker.from_env()
    print("watching docker container deaths (discord {})".format("enabled" if DISCORD_ENABLED else "disabled"), flush=True)

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
            log_text = container.logs(stdout=True, stderr=True, tail=1000).decode("utf-8", errors="replace")
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
