const pulumi = require("@pulumi/pulumi")
const k8s = require("@pulumi/kubernetes")

module.exports = {
    setup: function (namespace, conf, dependsOn) {

        const deployments = []
        const appLabels = { app: "write-back" }
        for(const [section, sectionConf] of Object.entries(conf['write-back'])) {
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
                "RabbitMQ": {
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
                },
                "Log": {
                    "Enabled": true,
                    "ServerId": "0",
                    "ServerName": "write-back"
                }
            }

            for(const [id, mysqlConfig] of Object.entries(conf.mysql[sectionConf.mysql].data)) {
                config.ConnectionStrings.MySql[id] = `Server=mysql-${sectionConf.mysql};Port=${mysqlConfig.port.cluster};User ID=fb; Password=admin; Database=fb`
            }

            for(const [id, redisConf] of Object.entries(conf.redis[sectionConf.redis])) {
                config.Redis[id] = {
                    Host: `redis-${sectionConf.redis}`,
                    Port: conf.redis[sectionConf.redis][id].port.cluster
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
                            nodeSelector: {
                                cpu: "epyc"
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
