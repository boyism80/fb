const pulumi = require("@pulumi/pulumi");
const k8s = require("@pulumi/kubernetes");

module.exports = function () {

    return {
        setup: function (namespace, conf, dependsOn) {

            let index = 0
            const ports = []
            for(const [section, sectionConf] of Object.entries(conf.game)) {
                for(const [i, container] of Object.entries(sectionConf.containers)) {
                    const config = {
                        id: parseInt(i),
                        name: `game-${section}-${i}`,
                        delay: 5,
                        ip: conf.host,
                        port: container.port,
                        thread: {
                            logic: 32,
                            io: 12,
                            background: 16
                        },
                        redis: {
                            ip: 'redis',
                            port: conf.redis[sectionConf.redis]['-1'].port.cluster,
                            pool: 1
                        },
                        save: 600,
                        internal: {
                            ip: `internal-${sectionConf.internal}`, 
                            port: conf.internal[sectionConf.internal].port.cluster
                        },
                        login: { ip: conf.host, port: conf.login[sectionConf.login].port },
                        amqp: {
                            ip: "rabbitmq",
                            port: conf.rabbitmq[sectionConf.rabbitmq].port.amqp.cluster,
                            uid: "fb",
                            pwd: "admin"
                        },
                        log: ["info", "warn", "fatal"]
                    }

                    const configMap = new k8s.core.v1.ConfigMap(`game-${section}-${i}`, {
                        metadata: { name: `game-${section}-${i}`, namespace: namespace.metadata.name },
                        data: {
                            "config.json": JSON.stringify(config),
                        },
                    })

                    const statefulSet = new k8s.apps.v1.StatefulSet(`game-${section}-${i}`, {
                        metadata: {
                            name: `game-${section}-${i}`,
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
                                    nodeSelector: {
                                        cpu: "epyc"
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

                    ports.push({
                        name: `game-${section}-${i}`,
                        port: container.port,
                        targetPort: `game-${index}`,
                        nodePort: container.port 
                    })

                    index++
                }
            }

            return new k8s.core.v1.Service('game', {
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
        }
    }
}()