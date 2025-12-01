const pulumi = require("@pulumi/pulumi")
const k8s = require("@pulumi/kubernetes")

module.exports = {
    setup: function (namespace, conf, dependsOn) {

        const deployments = []
        const appLabels = { app: "log" }
        for(const [section, sectionConf] of Object.entries(conf.log)) {
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
                "RabbitMQ": {
                    "Host": `rabbitmq-${sectionConf.rabbitmq || "section-1"}-log`,
                    "Port": conf.rabbitmq[sectionConf.rabbitmq || "section-1"].log.port.amqp.cluster,
                    "Uid": "fb",
                    "Pwd": "admin",
                    "QueueSize": 128
                }
            }

            // Use log MySQL instances
            if (conf.mysql[section] && conf.mysql[section].log && Array.isArray(conf.mysql[section].log)) {
                conf.mysql[section].log.forEach((logConf, index) => {
                    config.ConnectionStrings.MySql[index.toString()] = `Server=mysql-${section}-log;Port=${logConf.port.cluster};User ID=fb; Password=admin; Database=fb`
                })
            }

            const configMap = new k8s.core.v1.ConfigMap(`log-${section}`, {
                metadata: { name: `log-${section}`, namespace: namespace.metadata.name },
                data: {
                    "appsettings.json": JSON.stringify(config),
                },
            })

            const deployment = new k8s.apps.v1.Deployment(`log-${section}`, {
                metadata: { name: `log-${section}`, namespace: namespace.metadata.name },
                spec: {
                    selector: { matchLabels: appLabels },
                    replicas: 1,
                    template: {
                        metadata: { labels: appLabels },
                        spec: {
                            nodeSelector: {
                                cpu: "epyc"
                            },
                            containers: [{
                                name: "log",
                                image: "ghcr.io/boyism80/fb/log:latest",
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
            
            deployments.push(deployment)
        }
        
        return deployments
    }
}

