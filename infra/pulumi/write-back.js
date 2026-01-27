const pulumi = require("@pulumi/pulumi")
const k8s = require("@pulumi/kubernetes")

module.exports = {
    setup: function (namespace, conf, dependsOn) {

        const deployments = []
        const appLabels = { app: "write-back" }
        for(const [worldName, worldConf] of Object.entries(conf.worlds)) {
            // Skip if write-back is not configured (empty object)
            if (!worldConf['write-back'] || Object.keys(worldConf['write-back']).length === 0) continue
            
            const worldId = worldConf.id.toString()
            const config = {
                "World": worldConf.id,
                "Logging": {
                    "LogLevel": {
                        "Default": "Information",
                        "Microsoft.AspNetCore": "Warning"
                    }
                },
                "ConnectionStrings": {
                    "MySql": {}
                },
                "Redis": {},
                "RabbitMQ": {},
                "Log": {
                    "Enabled": true,
                    "ServerId": "0",
                    "ServerName": "write-back"
                }
            }

            // Build MySQL connections for this world (nested structure: MySql:{worldId}:{id})
            if (worldConf.mysql && worldConf.mysql.data) {
                if (!config.ConnectionStrings.MySql[worldId]) {
                    config.ConnectionStrings.MySql[worldId] = {}
                }
                for(const [id, mysqlConfig] of Object.entries(worldConf.mysql.data)) {
                    config.ConnectionStrings.MySql[worldId][id] = `Server=mysql-${worldName};Port=${mysqlConfig.port.cluster};User ID=fb; Password=admin; Database=fb`
                }
            }

            // Build Redis connections for this world (nested structure: Redis:{worldId}:{id})
            if (worldConf.redis) {
                if (!config.Redis[worldId]) {
                    config.Redis[worldId] = {}
                }
                for(const [id, redisConf] of Object.entries(worldConf.redis)) {
                    config.Redis[worldId][id] = {
                        Host: `redis-${worldName}`,
                        Port: redisConf.port.cluster
                    }
                }
            }

            // Build RabbitMQ connections for this world (nested structure: RabbitMQ:{worldId}:{Internal/Log})
            if (worldConf.rabbitmq) {
                if (!config.RabbitMQ[worldId]) {
                    config.RabbitMQ[worldId] = {}
                }
                config.RabbitMQ[worldId]["Internal"] = {
                    "Host": `rabbitmq-${worldName}-internal`,
                    "Port": worldConf.rabbitmq.internal.port.amqp.cluster,
                    "Uid": "fb",
                    "Pwd": "admin"
                }
                config.RabbitMQ[worldId]["Log"] = {
                    "Host": `rabbitmq-${worldName}-log`,
                    "Port": worldConf.rabbitmq.log.port.amqp.cluster,
                    "Uid": "fb",
                    "Pwd": "admin",
                    "QueueSize": 128
                }
            }

            const configMap = new k8s.core.v1.ConfigMap(`write-back-${worldName}`, {
                metadata: { name: `write-back-${worldName}`, namespace: namespace.metadata.name },
                data: {
                    "appsettings.json": JSON.stringify(config),
                },
            })

            const containerSpec = {
                name: "write-back",
                image: "ghcr.io/boyism80/fb/write-back:latest",
                imagePullPolicy: "Always",
                securityContext: {
                    capabilities: {
                        add: ["SYS_PTRACE"]
                    }
                },
                volumeMounts: [{
                    name: "config-volume",
                    mountPath: "/app/appsettings.json",
                    subPath: "appsettings.json"
                }],
            }

            const deployment = new k8s.apps.v1.Deployment(`write-back-${worldName}`, {
                metadata: { name: `write-back-${worldName}`, namespace: namespace.metadata.name },
                spec: {
                    selector: { matchLabels: appLabels },
                    replicas: 1,
                    template: {
                        metadata: { labels: appLabels },
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
                            containers: [containerSpec],
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
            
            deployments.push(deployment)
        }
        
        return deployments
    }
}
