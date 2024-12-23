const pulumi = require("@pulumi/pulumi");
const k8s = require("@pulumi/kubernetes");

module.exports = function () {

    return {
        setup: function (namespace, conf, dependsOn) {

            let index = 0
            const ports = []
            for(const [section, sectionConf] of Object.entries(conf.bot)) {
                const config = {
                    log: ["debug", "info", "warn", "fatal"],
                    ip: conf.host,
                    port: conf.gateway.port,
                    io_size: sectionConf.io_size,
                    count: sectionConf.count
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
                                        volumeMounts: [{
                                            name: "config-volume",
                                            mountPath: "/app/config/config.json",
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
            }
        }
    }
}()