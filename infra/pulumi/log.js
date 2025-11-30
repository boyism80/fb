const pulumi = require("@pulumi/pulumi")
const k8s = require("@pulumi/kubernetes")

module.exports = {
    setup: function (namespace, conf, dependsOn) {

        const resources = []
        for(const [section, sectionConf] of Object.entries(conf.log)) {
            const appLabels = { app: `log-${section}` }
            const config = {
                "Logging": {
                    "LogLevel": {
                        "Default": "Information",
                        "Microsoft.AspNetCore": "Warning"
                    }
                },
                "AllowedHosts": "*",
                "ConnectionStrings": {
                    "MySql": {}
                },
                "Redis": {},
                "Urls": `http://0.0.0.0:${sectionConf.port.cluster}`
            }

            // Use log MySQL instances
            if (conf.mysql[section] && conf.mysql[section].log && Array.isArray(conf.mysql[section].log)) {
                conf.mysql[section].log.forEach((logConf, index) => {
                    config.ConnectionStrings.MySql[index.toString()] = `Server=mysql-${section}-log;Port=${logConf.port.cluster};User ID=fb; Password=admin; Database=fb_log_${index}`
                })
            }

            // Use log Redis instances
            if (conf.redis[section] && conf.redis[section].log && Array.isArray(conf.redis[section].log)) {
                conf.redis[section].log.forEach((logConf, index) => {
                    config.Redis[index.toString()] = {
                        Host: `redis-${section}-log`,
                        Port: logConf.port.cluster
                    }
                })
            }

            const configMap = new k8s.core.v1.ConfigMap(`log-${section}`, {
                metadata: { name: `log-${section}`, namespace: namespace.metadata.name },
                data: {
                    "appsettings.k8s.json": JSON.stringify(config),
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
                                ports: [{ containerPort: 80, name: `log` }],
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

            const service = new k8s.core.v1.Service(`log-${section}`, {
                metadata: { name: `log-${section}`, namespace: namespace.metadata.name },
                spec: {
                    type: "NodePort",
                    ports: [{ 
                        name: `log-${section}`,
                        port: sectionConf.port.cluster,
                        targetPort: `log`,
                        protocol: "TCP",
                        nodePort: sectionConf.port.node 
                    }],
                    selector: appLabels,
                },
            }, { dependsOn: dependsOn })
            
            // Collect all resources
            resources.push(configMap)
            resources.push(deployment)
            resources.push(service)
        }
        
        return resources
    }
}

