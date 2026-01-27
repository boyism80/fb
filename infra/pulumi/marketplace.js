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

        // Add unified MySQL connection (string) - marketplace uses unified for all operations
        if (conf["unified-infra"] && conf["unified-infra"].mysql && conf["unified-infra"].mysql.port) {
            const unifiedMysql = conf["unified-infra"].mysql
            if (unifiedMysql) {
                config.ConnectionStrings.MySql["unified"] = `Server=mysql-unified-global;Port=${unifiedMysql.port.cluster};User ID=fb; Password=admin; Database=fb`
            }
        }

        // Add unified Redis connection (string) - marketplace uses unified for marketplace operations
        if (conf["unified-infra"] && conf["unified-infra"].redis && conf["unified-infra"].redis.port) {
            const unifiedRedis = conf["unified-infra"].redis
            if (unifiedRedis) {
                config.Redis["unified"] = {
                    Host: `redis-unified-global`,
                    Port: unifiedRedis.port.cluster
                }
            }
        }

        // Build Redis connections for all worlds (marketplace uses world-specific Redis for storage operations)
        config.Redis["worlds"] = {}
        for(const [worldName, worldConf] of Object.entries(conf.worlds)) {
            const worldId = worldConf.id.toString()
            
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



