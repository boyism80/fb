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

        // Build MySQL connections for all sections (nested structure: MySql:{section}:{id})
        for(const [sectionName, sectionConf] of Object.entries(adminToolConf.sections)) {
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
        if (adminToolConf["unified-global"]) {
            const unifiedGlobal = adminToolConf["unified-global"]
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
        for(const [sectionName, sectionConf] of Object.entries(adminToolConf.sections)) {
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

        // Build SectionServers mapping from login configuration (same as gateway entrypoints)
        config.SectionServers = {}
        for(const [section, loginConf] of Object.entries(conf.login)) {
            if (loginConf.name) {
                config.SectionServers[section] = loginConf.name
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

