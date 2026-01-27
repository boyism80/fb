const pulumi = require("@pulumi/pulumi")
const k8s = require("@pulumi/kubernetes")

module.exports = {
    setup: function (namespace, conf, dependsOn) {

        const resources = []
        const ports = []
        const adminToolConf = conf["admin-tool"]
        const appLabels = { app: "admin-tool" }
        const config = {
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
                "ServerName": "admin-tool"
            },
            "Security": {
                "ElevationSecret": ""
            }
        }

        // Add unified MySQL connection (string)
        if (conf["unified-infra"] && conf["unified-infra"].mysql && conf["unified-infra"].mysql.port) {
            const unifiedMysql = conf["unified-infra"].mysql
            if (unifiedMysql) {
                config.ConnectionStrings.MySql["unified"] = `Server=mysql-unified-global;Port=${unifiedMysql.port.cluster};User ID=fb; Password=admin; Database=fb`
            }
        }

        // Add unified Redis connection (string)
        if (conf["unified-infra"] && conf["unified-infra"].redis && conf["unified-infra"].redis.port) {
            const unifiedRedis = conf["unified-infra"].redis
            if (unifiedRedis) {
                config.Redis["unified"] = {
                    Host: `redis-unified-global`,
                    Port: unifiedRedis.port.cluster
                }
            }
        }

        // Build MySQL and Redis connections for all worlds (unified/global/data structure)
        config.ConnectionStrings.MySql["worlds"] = {}
        config.Redis["worlds"] = {}
        for(const [worldName, worldConf] of Object.entries(conf.worlds)) {
            const worldId = worldConf.id.toString()
            
            // Build MySQL: worlds:{worldId}:{global, data[]}
            if (worldConf.mysql) {
                config.ConnectionStrings.MySql["worlds"][worldId] = {}
                
                // Global connection
                if (worldConf.mysql.global) {
                    const globalMysql = worldConf.mysql.global
                    config.ConnectionStrings.MySql["worlds"][worldId]["global"] = `Server=mysql-${worldName}-global;Port=${globalMysql.port.cluster};User ID=fb; Password=admin; Database=fb`
                }
                
                // Data array (shard connections)
                if (worldConf.mysql.data && Array.isArray(worldConf.mysql.data)) {
                    const dataArray = worldConf.mysql.data.map(mysqlConf => 
                        `Server=mysql-${worldName};Port=${mysqlConf.port.cluster};User ID=fb; Password=admin; Database=fb`
                    )
                    if (dataArray.length > 0) {
                        config.ConnectionStrings.MySql["worlds"][worldId]["data"] = dataArray
                    }
                }
            }
            
            // Build Redis: worlds:{worldId}:{global, data[]}
            if (worldConf.redis) {
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
        }

        // Build RabbitMQ connections using unified-global (flat structure: RabbitMQ:{Internal/Log})
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

        // Build WorldServers mapping from worlds configuration (same as gateway entrypoints)
        config.WorldServers = {}
        for(const [worldName, worldConf] of Object.entries(conf.worlds)) {
            if (worldConf.name) {
                config.WorldServers[worldConf.id.toString()] = worldConf.name
            }
        }

        if (adminToolConf.security && adminToolConf.security.elevationSecret) {
            config.Security.ElevationSecret = adminToolConf.security.elevationSecret
        }

        const configMap = new k8s.core.v1.ConfigMap("admin-tool", {
            metadata: { name: "admin-tool", namespace: namespace.metadata.name },
            data: {
                "appsettings.k8s.json": JSON.stringify(config),
            },
        })

        const deployment = new k8s.apps.v1.Deployment("admin-tool", {
            metadata: { name: "admin-tool", namespace: namespace.metadata.name },
            spec: {
                selector: { matchLabels: appLabels },
                replicas: adminToolConf.replicas,
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
                            containers: [{
                                nodeSelector: {
                                    cpu: "epyc"
                                },
                                name: "admin-tool",
                                image: "ghcr.io/boyism80/fb/admin-tool:latest",
                                imagePullPolicy: "Always",
                                ports: [{ containerPort: 80, name: `admin-tool` }],
                                startupProbe: {
                                    httpGet: {
                                        path: "/",
                                        port: 80,
                                    },
                                    initialDelaySeconds: 10,
                                    periodSeconds: 5,
                                    failureThreshold: 30
                                },
                                resources: {
                                    requests: {
                                        cpu: "500m"
                                    }
                                },
                                env: [
                                    {
                                        name: 'ASPNETCORE_ENVIRONMENT',
                                        value: 'k8s'
                                    },
                                    {
                                        name: 'ASPNETCORE_HTTP_PORTS',
                                        value: '80'
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

        const service = new k8s.core.v1.Service("admin-tool", {
            metadata: { name: "admin-tool", namespace: namespace.metadata.name },
            spec: {
                type: "NodePort",
                ports: [{ 
                    name: "admin-tool",
                    port: adminToolConf.port.cluster,
                    targetPort: "admin-tool",
                    protocol: "TCP",
                    nodePort: adminToolConf.port.node 
                }],
                selector: appLabels,
            },
        }, { dependsOn: dependsOn })
        
        // Collect all resources
        resources.push(configMap)
        resources.push(deployment)
        resources.push(service)
        
        return resources
    }
}

