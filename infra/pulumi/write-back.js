const pulumi = require("@pulumi/pulumi")
const k8s = require("@pulumi/kubernetes")

module.exports = {
    setup: function (namespace, conf, dependsOn) {

        const deployments = []
        const appLabels = { app: "write-back" }
        for(const [worldName, worldConf] of Object.entries(conf.worlds)) {
            // Skip if write-back is not configured
            if (!worldConf['write-back'])
                continue
            
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

            // Build MySQL connections for this world (unified/global/data structure)
            if (worldConf.mysql) {
                config.ConnectionStrings.MySql["worlds"] = config.ConnectionStrings.MySql["worlds"] || {}
                config.ConnectionStrings.MySql["worlds"][worldId] = {}
                
                // Global connection
                if (worldConf.mysql.global) {
                    const globalMysql = worldConf.mysql.global
                    config.ConnectionStrings.MySql["worlds"][worldId]["global"] = `Server=mysql-${worldName}-global;Port=${globalMysql.port.cluster};User ID=fb; Password=admin; Database=fb`
                }
                
                // Data array (shard connections)
                if (worldConf.mysql.data && Array.isArray(worldConf.mysql.data)) {
                    const dataArray = worldConf.mysql.data.map(mysqlConfig => 
                        `Server=mysql-${worldName};Port=${mysqlConfig.port.cluster};User ID=fb; Password=admin; Database=fb`
                    )
                    if (dataArray.length > 0) {
                        config.ConnectionStrings.MySql["worlds"][worldId]["data"] = dataArray
                    }
                }
            }

            // Build Redis connections for this world (unified/global/data structure)
            if (worldConf.redis) {
                config.Redis["worlds"] = config.Redis["worlds"] || {}
                config.Redis["worlds"][worldId] = {}
                
                // Global Redis
                if (worldConf.redis.global) {
                    const globalRedis = worldConf.redis.global
                    config.Redis["worlds"][worldId]["global"] = {
                        Host: `redis-${worldName}`,
                        Port: globalRedis.port.cluster
                    }
                }
                
                // Data array (shard Redis)
                if (worldConf.redis.data && Array.isArray(worldConf.redis.data)) {
                    const dataArray = worldConf.redis.data.map(redisConf => ({
                        Host: `redis-${worldName}`,
                        Port: redisConf.port.cluster
                    }))
                    if (dataArray.length > 0) {
                        config.Redis["worlds"][worldId]["data"] = dataArray
                    }
                }
            }

            // Build RabbitMQ connections using unified (flat structure: RabbitMQ:{Internal/Log})
            if (conf["unified-infra"] && conf["unified-infra"].rabbitmq) {
                config.RabbitMQ["Internal"] = {
                    "Host": "rabbitmq-internal",
                    "Port": conf["unified-infra"].rabbitmq.internal.port.amqp.cluster,
                    "Uid": "fb",
                    "Pwd": "admin"
                }
                config.RabbitMQ["Log"] = {
                    "Host": "rabbitmq-log",
                    "Port": conf["unified-infra"].rabbitmq.log.port.amqp.cluster,
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
