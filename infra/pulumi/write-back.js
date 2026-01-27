const pulumi = require("@pulumi/pulumi")
const k8s = require("@pulumi/kubernetes")

module.exports = {
    setup: function (namespace, conf, dependsOn) {

        const deployments = []
        const appLabels = { app: "write-back" }
        for(const [section, sectionConf] of Object.entries(conf['write-back'])) {
            const config = {
                "Section": section,
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

            // Build MySQL connections for this section (nested structure: MySql:{section}:{id})
            if (sectionConf.mysql && conf.mysql[sectionConf.mysql]) {
                if (!config.ConnectionStrings.MySql[section]) {
                    config.ConnectionStrings.MySql[section] = {}
                }
                for(const [id, mysqlConfig] of Object.entries(conf.mysql[sectionConf.mysql].data)) {
                    config.ConnectionStrings.MySql[section][id] = `Server=mysql-${sectionConf.mysql};Port=${mysqlConfig.port.cluster};User ID=fb; Password=admin; Database=fb`
                }
            }

            // Build Redis connections for this section (nested structure: Redis:{section}:{id})
            if (sectionConf.redis && conf.redis[sectionConf.redis]) {
                if (!config.Redis[section]) {
                    config.Redis[section] = {}
                }
                for(const [id, redisConf] of Object.entries(conf.redis[sectionConf.redis])) {
                    config.Redis[section][id] = {
                        Host: `redis-${sectionConf.redis}`,
                        Port: conf.redis[sectionConf.redis][id].port.cluster
                    }
                }
            }

            // Build RabbitMQ connections for this section (nested structure: RabbitMQ:{section}:{Internal/Log})
            if (sectionConf.rabbitmq && conf.rabbitmq[sectionConf.rabbitmq]) {
                if (!config.RabbitMQ[section]) {
                    config.RabbitMQ[section] = {}
                }
                config.RabbitMQ[section]["Internal"] = {
                    "Host": `rabbitmq-${sectionConf.rabbitmq}-internal`,
                    "Port": conf.rabbitmq[sectionConf.rabbitmq].internal.port.amqp.cluster,
                    "Uid": "fb",
                    "Pwd": "admin"
                }
                config.RabbitMQ[section]["Log"] = {
                    "Host": `rabbitmq-${sectionConf.rabbitmq}-log`,
                    "Port": conf.rabbitmq[sectionConf.rabbitmq].log.port.amqp.cluster,
                    "Uid": "fb",
                    "Pwd": "admin",
                    "QueueSize": 128
                }
            }

            const configMap = new k8s.core.v1.ConfigMap(`write-back-${section}`, {
                metadata: { name: `write-back-${section}`, namespace: namespace.metadata.name },
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

            const deployment = new k8s.apps.v1.Deployment(`write-back-${section}`, {
                metadata: { name: `write-back-${section}`, namespace: namespace.metadata.name },
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
