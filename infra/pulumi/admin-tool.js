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

        // Build MySQL connections for all worlds (nested structure: MySql:{worldId}:{id})
        for(const [worldName, worldConf] of Object.entries(conf.worlds)) {
            const worldId = worldConf.id.toString()
            if (worldConf.mysql && worldConf.mysql.data) {
                if (!config.ConnectionStrings.MySql[worldId]) {
                    config.ConnectionStrings.MySql[worldId] = {}
                }
                for(const [id, mysqlConf] of Object.entries(worldConf.mysql.data)) {
                    config.ConnectionStrings.MySql[worldId][id] = `Server=mysql-${worldName};Port=${mysqlConf.port.cluster};User ID=fb; Password=admin; Database=fb`
                }
            }
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
        }

        // Build RabbitMQ connections for all worlds
        for(const [worldName, worldConf] of Object.entries(conf.worlds)) {
            const worldId = worldConf.id.toString()
            if (worldConf.rabbitmq) {
                config.RabbitMQ[worldId] = {
                    "Internal": {
                        "Host": `rabbitmq-${worldName}-internal`,
                        "Port": worldConf.rabbitmq.internal.port.amqp.cluster,
                        "Uid": "fb",
                        "Pwd": "admin"
                    },
                    "Log": {
                        "Host": `rabbitmq-${worldName}-log`,
                        "Port": worldConf.rabbitmq.log.port.amqp.cluster,
                        "Uid": "fb",
                        "Pwd": "admin",
                        "QueueSize": 128
                    }
                }
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

