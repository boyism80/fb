const pulumi = require("@pulumi/pulumi")
const k8s = require("@pulumi/kubernetes")

module.exports = {
    setup: function (namespace, conf, dependsOn) {

        const resources = []
        const ports = []
        for(const [section, sectionConf] of Object.entries(conf.internal)) {
            const appLabels = { app: `internal-${section}` }
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
                    "Host": `rabbitmq-${sectionConf.rabbitmq}-internal`,
                    "Port": conf.rabbitmq[sectionConf.rabbitmq].internal.port.amqp.cluster,
                    "Uid": "fb",
                    "Pwd": "admin"
                }
            }

            for(const [id, mysqlConf] of Object.entries(conf.mysql[sectionConf.mysql].data)) {
                config.ConnectionStrings.MySql[id] = `Server=mysql-${sectionConf.mysql};Port=${mysqlConf.port.cluster};User ID=fb; Password=admin; Database=fb`
            }

            for(const [id, redisConf] of Object.entries(conf.redis[sectionConf.redis].data)) {
                config.Redis[id] = {
                    Host: `redis-${sectionConf.redis}`,
                    Port: conf.redis[sectionConf.redis].data[id].port.cluster
                }
            }

            const configMap = new k8s.core.v1.ConfigMap(`internal-${section}`, {
                metadata: { name: `internal-${section}`, namespace: namespace.metadata.name },
                data: {
                    "appsettings.k8s.json": JSON.stringify(config),
                },
            })

            const deployment = new k8s.apps.v1.Deployment(`internal-${section}`, {
                metadata: { name: `internal-${section}`, namespace: namespace.metadata.name },
                spec: {
                    selector: { matchLabels: appLabels },
                    replicas: 1,
                    template: {
                        metadata: { labels: appLabels },
                        spec: {
                            containers: [{
                                nodeSelector: {
                                    cpu: "epyc"
                                },
                                name: "internal",
                                image: "ghcr.io/boyism80/fb/internal:latest",
                                imagePullPolicy: "Always",
                                securityContext: {
                                    capabilities: {
                                        add: ["SYS_PTRACE"]
                                    }
                                },
                                ports: [{ containerPort: 80, name: `internal` }],
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

            const hpa = new k8s.autoscaling.v2.HorizontalPodAutoscaler(`internal-hpa-${section}`, {
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

            const service = new k8s.core.v1.Service(`internal-${section}`, {
                metadata: { name: `internal-${section}`, namespace: namespace.metadata.name },
                spec: {
                    type: "NodePort",
                    ports: [{ 
                        name: `internal-${section}`,
                        port: sectionConf.port.cluster,
                        targetPort: `internal`,
                        protocol: "TCP",
                        nodePort: sectionConf.port.node 
                    }],
                    selector: appLabels,
                },
            }, { dependsOn: dependsOn })
            
            // Collect all resources
            resources.push(configMap)
            resources.push(deployment)
            resources.push(hpa)
            resources.push(service)
        }
        
        return resources
    }
}
