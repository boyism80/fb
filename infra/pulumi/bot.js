const pulumi = require("@pulumi/pulumi");
const k8s = require("@pulumi/kubernetes");

module.exports = function () {

    return {
        setup: function (namespace, conf, dependsOn) {

            const resources = []
            for(const [worldName, worldConf] of Object.entries(conf.worlds)) {
                if (!worldConf.bot) continue
                const botConf = worldConf.bot
                const config = {
                    log: ["info", "warn", "fatal"],
                    ip: conf.host,
                    port: conf.gateway.port,
                    io_size: botConf.io_size,
                    interval: botConf.interval,
                    spawn_per_interval: botConf.spawn_per_interval,
                    spawn_count: botConf.spawn_count,
                    thread: {
                        logic: botConf.thread.logic
                    }
                }

                const configMap = new k8s.core.v1.ConfigMap(`bot-${worldName}`, {
                    metadata: { name: `bot-${worldName}`, namespace: namespace.metadata.name },
                    data: {
                        "config.json": JSON.stringify(config),
                    },
                })

                const statefulSet = new k8s.apps.v1.StatefulSet(`bot-${worldName}`, {
                    metadata: {
                        name: `bot-${worldName}`,
                        namespace: namespace.metadata.name,
                    },
                    spec: {
                        serviceName: "bot",
                        replicas: botConf.replicas,
                        selector: {
                            matchLabels: {
                                app: "bot",
                            },
                        },
                        template: {
                            metadata: {
                                labels: {
                                    app: "bot",
                                },
                            },
                            spec: {
                                affinity: {
                                    nodeAffinity: {
                                        preferredDuringSchedulingIgnoredDuringExecution: [{
                                            weight: 100,
                                            preference: {
                                                matchExpressions: [{
                                                    key: "cpu",
                                                    operator: "In",
                                                    values: ["epyc"]
                                                }]
                                            }
                                        }]
                                    }
                                },
                                containers: [
                                    {
                                        name: "bot",
                                        image: "ghcr.io/boyism80/fb/bot:latest",
                                        imagePullPolicy: "Always",
                                        securityContext: {
                                            capabilities: {
                                                add: ["SYS_PTRACE"]
                                            }
                                        },
                                        command: ["./bot"],
                                        args: ["-c", "config.json"],
                                        volumeMounts: [{
                                            name: "config-volume",
                                            mountPath: "/app/config.json",
                                            subPath: "config.json"
                                        }],
                                    },
                                ],
                                volumes: [{
                                    name: "config-volume",
                                    configMap: {
                                        name: configMap.metadata.name,
                                    },
                                }],
                            },
                        },
                    },
                }, { dependsOn: dependsOn })
                
                // Collect all resources
                resources.push(configMap)
                resources.push(statefulSet)
            }
            
            return resources
        }
    }
}()