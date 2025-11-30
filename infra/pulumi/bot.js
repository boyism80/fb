const pulumi = require("@pulumi/pulumi");
const k8s = require("@pulumi/kubernetes");

module.exports = function () {

    return {
        setup: function (namespace, conf, dependsOn) {

            const resources = []
            for(const [section, sectionConf] of Object.entries(conf.bot)) {
                const config = {
                    log: ["info", "warn", "fatal"],
                    ip: conf.host,
                    port: conf.gateway.port,
                    io_size: sectionConf.io_size,
                    interval: sectionConf.interval,
                    spawn_per_interval: sectionConf.spawn_per_interval,
                    spawn_count: sectionConf.spawn_count,
                    thread: {
                        logic: sectionConf.thread.logic
                    }
                }

                const configMap = new k8s.core.v1.ConfigMap(`bot-${section}`, {
                    metadata: { name: `bot-${section}`, namespace: namespace.metadata.name },
                    data: {
                        "config.json": JSON.stringify(config),
                    },
                })

                const statefulSet = new k8s.apps.v1.StatefulSet(`bot-${section}`, {
                    metadata: {
                        name: `bot-${section}`,
                        namespace: namespace.metadata.name,
                    },
                    spec: {
                        serviceName: "bot",
                        replicas: sectionConf.replicas,
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
                                nodeSelector: {
                                    cpu: "epyc"
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
                                        command: ["./app"],
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