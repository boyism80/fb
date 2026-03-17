const pulumi = require("@pulumi/pulumi");
const k8s = require("@pulumi/kubernetes");

module.exports = function () {

    return {
        setup: function (namespace, conf, dependsOn) {

            const resources = []
            // Get first world for gateway log (gateway is global, uses first world's log infrastructure)
            const firstWorldName = Object.keys(conf.worlds)[0]
            const firstWorld = conf.worlds[firstWorldName]
            
            const config = {
                id: 0,
                name: `gateway`,
                ip:  conf.host,
                port: conf.gateway.port,
                thread: {
                    logic: 12,
                    io: 12
                },
                log: {
                    ip: `log-${firstWorldName}`,
                    port: firstWorld.log.port.cluster,
                    level: ["info", "warn", "fatal"]
                },
                entrypoints: [],
                internal: {
                    ip: "internal",
                    port: conf.internal.port.cluster
                },
                amqp: {
                    internal: {
                        ip: "rabbitmq-internal",
                        port: conf["unified-infra"].rabbitmq.internal.port.amqp.cluster,
                        uid: "fb",
                        pwd: "admin"
                    },
                    log: {
                        ip: "rabbitmq-log",
                        port: conf["unified-infra"].rabbitmq.log.port.amqp.cluster,
                        uid: "fb",
                        pwd: "admin"
                    }
                },
                http: {
                    max_concurrent: 500
                },
            }

            for(const [worldName, worldConf] of Object.entries(conf.worlds)) {
                if (!worldConf.login) continue
                config.entrypoints.push({
                    name: worldConf.name,
                    desc: worldConf.desc,
                    ip: conf.host,
                    port: worldConf.login.port
                })
            }

            const configMap = new k8s.core.v1.ConfigMap(`gateway`, {
                metadata: { name: `gateway`, namespace: namespace.metadata.name },
                data: {
                    "config.json": JSON.stringify(config),
                },
            })

            const statefulSet = new k8s.apps.v1.StatefulSet(`gateway`, {
                metadata: {
                    name: `gateway`,
                    namespace: namespace.metadata.name,
                },
                spec: {
                    serviceName: "gateway",
                    replicas: conf.gateway.replicas,
                    selector: {
                        matchLabels: {
                            app: "gateway",
                        },
                    },
                    template: {
                        metadata: {
                            labels: {
                                app: "gateway",
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
                                    name: "gateway",
                                    image: "ghcr.io/boyism80/fb/gateway:latest",
                                    imagePullPolicy: "Always",
                                    securityContext: {
                                        capabilities: {
                                            add: ["SYS_PTRACE"]
                                        }
                                    },
                                    ports: [
                                        { containerPort: conf.gateway.port, name: `gateway` },
                                    ],
                                    readinessProbe: {
                                        tcpSocket: {
                                            port: `gateway`
                                        },
                                        initialDelaySeconds: 10,
                                        periodSeconds: 5,
                                        timeoutSeconds: 3,
                                        failureThreshold: 3
                                    },
                                    livenessProbe: {
                                        tcpSocket: {
                                            port: `gateway`
                                        },
                                        initialDelaySeconds: 30,
                                        periodSeconds: 10,
                                        timeoutSeconds: 3,
                                        failureThreshold: 3
                                    },
                                    command: ["./gateway"],
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

            const service = new k8s.core.v1.Service('gateway', {
                metadata: {
                    name: 'gateway',
                    namespace: namespace.metadata.name,
                },
                spec: {
                    type: "NodePort",
                    ports: [{
                        port: conf.gateway.port,
                        targetPort: `gateway`,
                        nodePort: conf.gateway.port 
                    }],
                    selector: {
                        app: "gateway",
                    }
                },
            }, { dependsOn: dependsOn })
            
            resources.push(service)
            return resources
        }
    }
}()