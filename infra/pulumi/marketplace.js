const pulumi = require("@pulumi/pulumi")
const k8s = require("@pulumi/kubernetes")

module.exports = {
    setup: function (namespace, conf, dependsOn) {

        const resources = []
        const ports = []
        const marketplaceConf = conf.marketplace
        const appLabels = { app: "marketplace" }
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
                "ServerName": "marketplace"
            },
            "Marketplace": {
                "UseSharding": false
            }
        }

        // Build MySQL connections for all worlds (nested structure: MySql:{worldId}:{id})
        // Marketplace uses world-1 for all worlds (shared marketplace)
        for(const [worldName, worldConf] of Object.entries(conf.worlds)) {
            const worldId = worldConf.id.toString()
            // Use world-1's MySQL and Redis for all worlds (shared marketplace)
            const marketplaceWorld = conf.worlds["world-1"]
            if (marketplaceWorld && marketplaceWorld.mysql && marketplaceWorld.mysql.data) {
                if (!config.ConnectionStrings.MySql[worldId]) {
                    config.ConnectionStrings.MySql[worldId] = {}
                }
                for(const [id, mysqlConf] of Object.entries(marketplaceWorld.mysql.data)) {
                    config.ConnectionStrings.MySql[worldId][id] = `Server=mysql-world-1;Port=${mysqlConf.port.cluster};User ID=fb; Password=admin; Database=fb`
                }
            }
            if (marketplaceWorld && marketplaceWorld.redis) {
                if (!config.Redis[worldId]) {
                    config.Redis[worldId] = {}
                }
                for(const [id, redisConf] of Object.entries(marketplaceWorld.redis)) {
                    config.Redis[worldId][id] = {
                        Host: `redis-world-1`,
                        Port: redisConf.port.cluster
                    }
                }
            }
        }

        // Add unified-infra MySQL and Redis (nested structure)
        if (conf["unified-infra"]) {
            const unifiedInfra = conf["unified-infra"]
            if (unifiedInfra.mysql && unifiedInfra.mysql.data) {
                if (!config.ConnectionStrings.MySql["unified-global"]) {
                    config.ConnectionStrings.MySql["unified-global"] = {}
                }
                for(const [id, mysqlConf] of Object.entries(unifiedInfra.mysql.data)) {
                    config.ConnectionStrings.MySql["unified-global"][id] = `Server=mysql-unified-global;Port=${mysqlConf.port.cluster};User ID=fb; Password=admin; Database=fb`
                }
            }
            if (unifiedInfra.redis) {
                if (!config.Redis["unified-global"]) {
                    config.Redis["unified-global"] = {}
                }
                for(const [id, redisConf] of Object.entries(unifiedInfra.redis)) {
                    config.Redis["unified-global"][id] = {
                        Host: `redis-unified-global`,
                        Port: redisConf.port.cluster
                    }
                }
            }
        }

        // Build RabbitMQ connections for all worlds (use world-1's RabbitMQ for all)
        const marketplaceWorld = conf.worlds["world-1"]
        if (marketplaceWorld && marketplaceWorld.rabbitmq) {
            for(const [worldName, worldConf] of Object.entries(conf.worlds)) {
                const worldId = worldConf.id.toString()
                config.RabbitMQ[worldId] = {
                    "Internal": {
                        "Host": `rabbitmq-world-1-internal`,
                        "Port": marketplaceWorld.rabbitmq.internal.port.amqp.cluster,
                        "Uid": "fb",
                        "Pwd": "admin"
                    },
                    "Log": {
                        "Host": `rabbitmq-world-1-log`,
                        "Port": marketplaceWorld.rabbitmq.log.port.amqp.cluster,
                        "Uid": "fb",
                        "Pwd": "admin",
                        "QueueSize": 128
                    }
                }
            }
        }

        const configMap = new k8s.core.v1.ConfigMap("marketplace", {
            metadata: { name: "marketplace", namespace: namespace.metadata.name },
            data: {
                "appsettings.k8s.json": JSON.stringify(config),
            },
        })

        const deployment = new k8s.apps.v1.Deployment("marketplace", {
            metadata: { name: "marketplace", namespace: namespace.metadata.name },
            spec: {
                selector: { matchLabels: appLabels },
                replicas: marketplaceConf.replicas || 1,
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
                                name: "marketplace",
                                image: "ghcr.io/boyism80/fb/marketplace:latest",
                                imagePullPolicy: "Always",
                                securityContext: {
                                    capabilities: {
                                        add: ["SYS_PTRACE"]
                                    }
                                },
                                ports: [{ containerPort: 80, name: `marketplace` }],
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

        const hpa = new k8s.autoscaling.v2.HorizontalPodAutoscaler("marketplace-hpa", {
            metadata: {
                namespace: namespace.metadata.name,
            },
            spec: {
                scaleTargetRef: {
                    apiVersion: "apps/v1",
                    kind: "Deployment",
                    name: deployment.metadata.name,
                },
                minReplicas: 5,
                maxReplicas: 30,
                metrics: [{
                    type: "Resource",
                    resource: {
                        name: "cpu",
                        target: {
                            type: "Utilization",
                            averageUtilization: 50,
                        },
                    },
                }],
            },
        })

        const service = new k8s.core.v1.Service("marketplace", {
            metadata: { name: "marketplace", namespace: namespace.metadata.name },
            spec: {
                type: "NodePort",
                ports: [{ 
                    name: "marketplace",
                    port: marketplaceConf.port.cluster,
                    targetPort: "marketplace",
                    protocol: "TCP",
                    nodePort: marketplaceConf.port.node 
                }],
                selector: appLabels,
            },
        }, { dependsOn: dependsOn })
        
        // Collect all resources
        resources.push(configMap)
        resources.push(deployment)
        resources.push(hpa)
        resources.push(service)
        
        return resources
    }
}



