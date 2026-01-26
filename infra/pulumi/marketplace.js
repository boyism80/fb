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

        // Build MySQL connections for all sections (nested structure: MySql:{section}:{id})
        for(const [sectionName, sectionConf] of Object.entries(marketplaceConf.sections)) {
            if (sectionConf.mysql && conf.mysql[sectionConf.mysql]) {
                if (!config.ConnectionStrings.MySql[sectionName]) {
                    config.ConnectionStrings.MySql[sectionName] = {}
                }
                for(const [id, mysqlConf] of Object.entries(conf.mysql[sectionConf.mysql].data)) {
                    config.ConnectionStrings.MySql[sectionName][id] = `Server=mysql-${sectionConf.mysql};Port=${mysqlConf.port.cluster};User ID=fb; Password=admin; Database=fb`
                }
            }
            if (sectionConf.redis && conf.redis[sectionConf.redis]) {
                if (!config.Redis[sectionName]) {
                    config.Redis[sectionName] = {}
                }
                for(const [id, redisConf] of Object.entries(conf.redis[sectionConf.redis])) {
                    config.Redis[sectionName][id] = {
                        Host: `redis-${sectionConf.redis}`,
                        Port: conf.redis[sectionConf.redis][id].port.cluster
                    }
                }
            }
        }

        // Add unified-global MySQL and Redis (nested structure)
        if (marketplaceConf["unified-global"]) {
            const unifiedGlobal = marketplaceConf["unified-global"]
            if (unifiedGlobal.mysql && conf.mysql[unifiedGlobal.mysql]) {
                if (!config.ConnectionStrings.MySql["unified-global"]) {
                    config.ConnectionStrings.MySql["unified-global"] = {}
                }
                for(const [id, mysqlConf] of Object.entries(conf.mysql[unifiedGlobal.mysql].data)) {
                    config.ConnectionStrings.MySql["unified-global"][id] = `Server=mysql-${unifiedGlobal.mysql};Port=${mysqlConf.port.cluster};User ID=fb; Password=admin; Database=fb`
                }
            }
            if (unifiedGlobal.redis && conf.redis[unifiedGlobal.redis]) {
                if (!config.Redis["unified-global"]) {
                    config.Redis["unified-global"] = {}
                }
                for(const [id, redisConf] of Object.entries(conf.redis[unifiedGlobal.redis])) {
                    config.Redis["unified-global"][id] = {
                        Host: `redis-${unifiedGlobal.redis}`,
                        Port: conf.redis[unifiedGlobal.redis][id].port.cluster
                    }
                }
            }
        }

        // Build RabbitMQ connections for all sections
        for(const [sectionName, sectionConf] of Object.entries(marketplaceConf.sections)) {
            if (sectionConf.rabbitmq && conf.rabbitmq[sectionConf.rabbitmq]) {
                config.RabbitMQ[sectionName] = {
                    "Internal": {
                        "Host": `rabbitmq-${sectionConf.rabbitmq}-internal`,
                        "Port": conf.rabbitmq[sectionConf.rabbitmq].internal.port.amqp.cluster,
                        "Uid": "fb",
                        "Pwd": "admin"
                    },
                    "Log": {
                        "Host": `rabbitmq-${sectionConf.rabbitmq}-log`,
                        "Port": conf.rabbitmq[sectionConf.rabbitmq].log.port.amqp.cluster,
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



