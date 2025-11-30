const pulumi = require("@pulumi/pulumi");
const k8s = require("@pulumi/kubernetes");

module.exports = function () {

    return {
        setup: function (namespace, conf, dependsOn) {

            const resources = []
            const config = {
                id: 0,
                name: `gateway`,
                ip:  conf.host,
                port: conf.gateway.port,
                thread: {
                    logic: 12,
                    io: 12
                },
                log: ["info", "warn", "fatal"],
                entrypoints: [],
                internal: {
                    ip: `internal-${sectionConf.internal}`, 
                    port: conf.internal[sectionConf.internal].port.cluster
                },
                amqp: {
                    ip: "rabbitmq",
                    port: conf.rabbitmq['section-1'].port.amqp.cluster,
                    uid: "fb",
                    pwd: "admin"
                },
            }

            for(const [section, loginContainerConf] of Object.entries(conf.login)) {
                config.entrypoints.push({
                    name: loginContainerConf.name,
                    desc: loginContainerConf.desc,
                    ip: conf.host,
                    port: loginContainerConf.port
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
                            nodeSelector: {
                                cpu: "epyc"
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