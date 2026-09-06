const pulumi = require("@pulumi/pulumi");
const k8s = require("@pulumi/kubernetes");

module.exports = function () {

    return {
        setup: function (namespace, conf, dependsOn) {

            const resources = []
            let index = 0
            const ports = []
            const games = []
            const worldNames = Object.keys(conf.worlds || {})
            const firstWorldName = worldNames[0]
            const firstWorld = firstWorldName ? conf.worlds[firstWorldName] : null

            for (const [worldName, worldConf] of Object.entries(conf.worlds || {})) {
                if (!worldConf.game || !worldConf.game.containers) continue
                for (const [i, container] of Object.entries(worldConf.game.containers)) {
                    games.push({
                        resourceName: `game-${worldName}-${i}`,
                        id: parseInt(i),
                        name: `game-${worldName}-${i}`,
                        world: worldConf.id,
                        port: container.port,
                        loginPort: worldConf.login.port,
                        logHost: `log-${worldName}`,
                        logPort: worldConf.log.port.cluster,
                    })
                }
            }

            if (firstWorld && conf.cross && conf.cross.game && conf.cross.game.containers) {
                for (const [i, container] of Object.entries(conf.cross.game.containers)) {
                    games.push({
                        resourceName: `game-cross-${i}`,
                        id: parseInt(i),
                        name: `game-cross-${i}`,
                        world: null,
                        port: container.port,
                        loginPort: firstWorld.login.port,
                        logHost: `log-${firstWorldName}`,
                        logPort: firstWorld.log.port.cluster,
                    })
                }
            }

            for (const game of games) {
                const config = {
                    id: game.id,
                    name: game.name,
                    world: game.world,
                    delay: 5,
                    ip: conf.host,
                    port: game.port,
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
                    login: { ip: conf.host, port: game.loginPort },
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
                        ip: game.logHost,
                        port: game.logPort,
                        level: ["info", "warn", "fatal"]
                    },
                    lazy_load_maps: game.world == null,
                    exp_multiplier: 1.0,
                    drop_rate_multiplier: 1.0,
                    http: {
                        max_concurrent: 128
                    }
                }

                const configMap = new k8s.core.v1.ConfigMap(game.resourceName, {
                    metadata: { name: game.resourceName, namespace: namespace.metadata.name },
                    data: {
                        "config.json": JSON.stringify(config),
                    },
                })

                const portName = `game-${index}`
                const labels = { app: "game" }
                if (game.world == null)
                    labels.game = game.resourceName

                const statefulSet = new k8s.apps.v1.StatefulSet(game.resourceName, {
                    metadata: {
                        name: game.resourceName,
                        namespace: namespace.metadata.name,
                    },
                    spec: {
                        serviceName: "game",
                        replicas: 1,
                        selector: {
                            matchLabels: labels,
                        },
                        template: {
                            metadata: {
                                labels: labels,
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
                                            { containerPort: game.port, name: portName },
                                        ],
                                        readinessProbe: {
                                            tcpSocket: {
                                                port: portName
                                            },
                                            initialDelaySeconds: 10,
                                            periodSeconds: 5,
                                            timeoutSeconds: 3,
                                            failureThreshold: 3
                                        },
                                        livenessProbe: {
                                            tcpSocket: {
                                                port: portName
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

                resources.push(configMap)
                resources.push(statefulSet)

                ports.push({
                    name: game.resourceName,
                    port: game.port,
                    targetPort: portName,
                    nodePort: game.port
                })

                index++
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
