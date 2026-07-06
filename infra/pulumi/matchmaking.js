const k8s = require("@pulumi/kubernetes")

module.exports = {
    setup: function (namespace, conf, dependsOn) {

        const resources = []
        const matchmakingConf = conf.matchmaking
        const appLabels = { app: "matchmaking" }
        const config = {
            "Logging": {
                "LogLevel": {
                    "Default": "Information",
                    "Microsoft.AspNetCore": "Warning"
                }
            },
            "RabbitMQ": {},
            "Matchmaking": {
                "TickIntervalMs": 500,
                "BaseSkillTolerance": 1.0,
                "SkillTolerancePerSecond": 0.1,
                "MaxSkillTolerance": 5.0,
                "EffectiveMuSigmaFactor": 3.0,
                "SkillBucketWidth": 1.0,
                "ConfirmTimeoutSeconds": 30
            }
        }

        if (conf["unified-infra"] && conf["unified-infra"].rabbitmq) {
            config.RabbitMQ["Internal"] = {
                "Host": "rabbitmq-internal",
                "Port": conf["unified-infra"].rabbitmq.internal.port.amqp.cluster,
                "Uid": "fb",
                "Pwd": "admin"
            }
        }

        const configMap = new k8s.core.v1.ConfigMap("matchmaking", {
            metadata: { name: "matchmaking", namespace: namespace.metadata.name },
            data: {
                "appsettings.k8s.json": JSON.stringify(config),
            },
        })

        const deployment = new k8s.apps.v1.Deployment("matchmaking", {
            metadata: { name: "matchmaking", namespace: namespace.metadata.name },
            spec: {
                selector: { matchLabels: appLabels },
                replicas: matchmakingConf.replicas,
                template: {
                    metadata: { labels: appLabels },
                    spec: {
                        nodeSelector: {
                            cpu: "epyc"
                        },
                        containers: [{
                            name: "matchmaking",
                            image: "ghcr.io/boyism80/fb/matchmaking:latest",
                            imagePullPolicy: "Always",
                            ports: [{ containerPort: 80, name: "matchmaking" }],
                            startupProbe: {
                                httpGet: {
                                    path: "/health",
                                    port: 80,
                                },
                                initialDelaySeconds: 10,
                                periodSeconds: 5,
                                failureThreshold: 30
                            },
                            resources: {
                                requests: {
                                    cpu: "250m"
                                }
                            },
                            env: [
                                {
                                    name: "ASPNETCORE_ENVIRONMENT",
                                    value: "k8s"
                                },
                                {
                                    name: "ASPNETCORE_HTTP_PORTS",
                                    value: "80"
                                }],
                            volumeMounts: [{
                                name: "config-volume",
                                mountPath: "/app/appsettings.k8s.json",
                                subPath: "appsettings.k8s.json"
                            }],
                        }],
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

        const service = new k8s.core.v1.Service("matchmaking", {
            metadata: { name: "matchmaking", namespace: namespace.metadata.name },
            spec: {
                type: "NodePort",
                ports: [{
                    name: "matchmaking",
                    port: matchmakingConf.port.cluster,
                    targetPort: "matchmaking",
                    protocol: "TCP",
                    nodePort: matchmakingConf.port.node
                }],
                selector: appLabels,
            },
        }, { dependsOn: dependsOn })

        resources.push(configMap)
        resources.push(deployment)
        resources.push(service)

        return resources
    }
}
