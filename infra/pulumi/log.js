const pulumi = require("@pulumi/pulumi")
const k8s = require("@pulumi/kubernetes")

module.exports = {
    setup: function (namespace, conf, dependsOn) {

        const resources = []
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
                },
                "Log": {
                    "InstanceCount": 5
                },
                "HealthApi": {
                    "Enabled": true,
                    "Port": 80
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

            const healthApiEnabled = config.HealthApi?.Enabled !== false
            const healthApiPort = config.HealthApi?.Port || 80

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

            if (healthApiEnabled) {
                containerSpec.ports = [{ containerPort: healthApiPort }]
                containerSpec.readinessProbe = {
                    httpGet: {
                        path: "/health/ready",
                        port: healthApiPort
                    },
                    initialDelaySeconds: 5,
                    periodSeconds: 10,
                    timeoutSeconds: 3,
                    failureThreshold: 3
                }
                containerSpec.livenessProbe = {
                    httpGet: {
                        path: "/health/live",
                        port: healthApiPort
                    },
                    initialDelaySeconds: 10,
                    periodSeconds: 30,
                    timeoutSeconds: 5,
                    failureThreshold: 3
                }
                containerSpec.lifecycle = {
                    preStop: {
                        exec: {
                            command: ["/bin/sh", "-c", "sleep 30"]
                        }
                    }
                }
            }

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

            const hpa = new k8s.autoscaling.v2.HorizontalPodAutoscaler(`log-hpa-${section}`, {
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

