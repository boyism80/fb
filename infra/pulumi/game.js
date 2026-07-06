const pulumi = require("@pulumi/pulumi");
const k8s = require("@pulumi/kubernetes");

module.exports = function () {

    return {
        setup: function (namespace, conf, dependsOn) {

            const resources = []
            let index = 0
            const ports = []
            for(const [worldName, worldConf] of Object.entries(conf.worlds)) {
                if (!worldConf.game || !worldConf.game.containers) continue
                for(const [i, container] of Object.entries(worldConf.game.containers)) {
                    const config = {
                        id: parseInt(i),
                        name: `game-${worldName}-${i}`,
                        world: worldConf.id,
                        delay: 5,
                        ip: conf.host,
                        port: container.port,
                        thread: {
                            logic: 32,
                            io: 12
                        },
                        save: 600,
                        internal: {
                            ip: "internal",
                            port: conf.internal.port.cluster
                        },
                        marketplace: {
                            ip: "marketplace",
                            port: conf.marketplace.port.cluster
                        },
                        matchmaking: {
                            ip: "matchmaking",
                            port: conf.matchmaking.port.cluster
                        },
                        login: { ip: conf.host, port: worldConf.login.port },
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
                        log: {
                            ip: `log-${worldName}`,
                            port: worldConf.log.port.cluster,
                            level: ["info", "warn", "fatal"]
                        },
                        lazy_load_maps: false,
                        exp_multiplier: 1.0,
                        drop_rate_multiplier: 1.0,
                        http: {
                            max_concurrent: 128
                        }
                    }

                    const configMap = new k8s.core.v1.ConfigMap(`game-${worldName}-${i}`, {
                        metadata: { name: `game-${worldName}-${i}`, namespace: namespace.metadata.name },
                        data: {
                            "config.json": JSON.stringify(config),
                        },
                    })

                    const statefulSet = new k8s.apps.v1.StatefulSet(`game-${worldName}-${i}`, {
                        metadata: {
                            name: `game-${worldName}-${i}`,
                            namespace: namespace.metadata.name,
                        },
                        spec: {
                            serviceName: "game",
                            replicas: 1,
                            selector: {
                                matchLabels: {
                                    app: "game",
                                },
                            },
                            template: {
                                metadata: {
                                    labels: {
                                        app: "game",
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
                                            name: "game",
                                            image: "ghcr.io/boyism80/fb/game:latest",
                                            imagePullPolicy: "Always",
                                            securityContext: {
                                                capabilities: {
                                                    add: ["SYS_PTRACE"]
                                                }
                                            },
                                            ports: [
                                                { containerPort: container.port, name: `game-${index}` },
                                            ],
                                            readinessProbe: {
                                                tcpSocket: {
                                                    port: `game-${index}`
                                                },
                                                initialDelaySeconds: 10,
                                                periodSeconds: 5,
                                                timeoutSeconds: 3,
                                                failureThreshold: 3
                                            },
                                            livenessProbe: {
                                                tcpSocket: {
                                                    port: `game-${index}`
                                                },
                                                initialDelaySeconds: 30,
                                                periodSeconds: 10,
                                                timeoutSeconds: 3,
                                                failureThreshold: 3
                                            },
                                            command: ["./game"],
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

                    ports.push({
                        name: `game-${worldName}-${i}`,
                        port: container.port,
                        targetPort: `game-${index}`,
                        nodePort: container.port 
                    })

                    index++
                }
            }

            const service = new k8s.core.v1.Service('game', {
                metadata: {
                    name: 'game',
                    namespace: namespace.metadata.name,
                },
                spec: {
                    type: "NodePort",
                    ports: ports,
                    selector: {
                        app: "game",
                    }
                },
            }, { dependsOn: dependsOn })
            
            resources.push(service)
            return resources
        }
    }
}()