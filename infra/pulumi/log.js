const pulumi = require("@pulumi/pulumi")
const k8s = require("@pulumi/kubernetes")

module.exports = {
    setup: function (namespace, conf, dependsOn) {

        const resources = []
        const appLabels = { app: "log" }
        const autoMigration = !!(conf.database && conf.database.autoMigration)
        for(const [worldName, worldConf] of Object.entries(conf.worlds)) {
            if (!worldConf.log) continue
            const config = {
                "World": parseInt(worldConf.id),
                "Database": {
                    "AutoMigration": autoMigration
                },
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
                    "Internal": {
                        "Host": "rabbitmq-internal",
                        "Port": conf["unified-infra"].rabbitmq.internal.port.amqp.cluster,
                        "Uid": "fb",
                        "Pwd": "admin"
                    },
                    "Log": {
                        "Host": "rabbitmq-log",
                        "Port": conf["unified-infra"].rabbitmq.log.port.amqp.cluster,
                        "Uid": "fb",
                        "Pwd": "admin"
                    }
                }
            }

            // Use log MySQL instances (unified/global/data structure)
            const worldId = parseInt(worldConf.id);
            config.ConnectionStrings.MySql["worlds"] = config.ConnectionStrings.MySql["worlds"] || {}
            config.ConnectionStrings.MySql["worlds"][worldId.toString()] = {};
            
            // Global connection
            if (worldConf.mysql && worldConf.mysql.global) {
                const globalMysql = worldConf.mysql.global
                config.ConnectionStrings.MySql["worlds"][worldId.toString()]["global"] = `Server=mysql-${worldName}-global;Port=${globalMysql.port.cluster};User ID=fb; Password=admin; Database=fb`
            }
            
            // Data array (log shard connections)
            if (worldConf.mysql && worldConf.mysql.log && Array.isArray(worldConf.mysql.log)) {
                const dataArray = worldConf.mysql.log.map(logConf => 
                    `Server=mysql-${worldName}-log;Port=${logConf.port.cluster};User ID=fb; Password=admin; Database=fb`
                )
                if (dataArray.length > 0) {
                    config.ConnectionStrings.MySql["worlds"][worldId.toString()]["data"] = dataArray
                }
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
                    // replicas is managed by HPA, do not set it here
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

