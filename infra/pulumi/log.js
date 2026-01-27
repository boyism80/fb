const pulumi = require("@pulumi/pulumi")
const k8s = require("@pulumi/kubernetes")

module.exports = {
    setup: function (namespace, conf, dependsOn) {

        const resources = []
        const appLabels = { app: "log" }
        for(const [worldName, worldConf] of Object.entries(conf.worlds)) {
            if (!worldConf.log) continue
            const config = {
                "World": parseInt(worldConf.id),
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
                    "Host": `rabbitmq-${worldName}-log`,
                    "Port": worldConf.rabbitmq.log.port.amqp.cluster,
                    "Uid": "fb",
                    "Pwd": "admin",
                    "QueueSize": 128
                },
                "Log": {
                    "InstanceCount": 5
                }
            }

            // Use log MySQL instances
            if (worldConf.mysql && worldConf.mysql.log && Array.isArray(worldConf.mysql.log)) {
                worldConf.mysql.log.forEach((logConf, index) => {
                    config.ConnectionStrings.MySql[index.toString()] = `Server=mysql-${worldName}-log;Port=${logConf.port.cluster};User ID=fb; Password=admin; Database=fb`
                })
            }

            const configMap = new k8s.core.v1.ConfigMap(`log-${worldName}`, {
                metadata: { name: `log-${worldName}`, namespace: namespace.metadata.name },
                data: {
                    "appsettings.json": JSON.stringify(config),
                },
            })

            const containerSpec = {
                name: "log",
                image: "ghcr.io/boyism80/fb/log:latest",
                imagePullPolicy: "Always",
                securityContext: {
                    capabilities: {
                        add: ["SYS_PTRACE"]
                    }
                },
                env: [{
                    name: "HOSTNAME",
                    valueFrom: {
                        fieldRef: {
                            fieldPath: "metadata.name"
                        }
                    }
                }],
                volumeMounts: [{
                    name: "config-volume",
                    mountPath: "/app/appsettings.json",
                    subPath: "appsettings.json"
                }],
            }

            const deployment = new k8s.apps.v1.Deployment(`log-${worldName}`, {
                metadata: { name: `log-${worldName}`, namespace: namespace.metadata.name },
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

            const hpa = new k8s.autoscaling.v2.HorizontalPodAutoscaler(`log-hpa-${worldName}`, {
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
            
            // Collect all resources
            resources.push(configMap)
            resources.push(deployment)
            resources.push(hpa)
        }
        
        return resources
    }
}

