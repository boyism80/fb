const pulumi = require("@pulumi/pulumi")
const k8s = require("@pulumi/kubernetes")

module.exports = {
    setup: function (namespace, conf, dependsOn) {

        const ports = []
        for(const [section, sectionConf] of Object.entries(conf["admin-tool"])) {
            const appLabels = { app: `admin-tool-${section}` }
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
                    "Host": `rabbitmq-${sectionConf.rabbitmq}`,
                    "Port": conf.rabbitmq[sectionConf.rabbitmq].port.amqp.cluster,
                    "Uid": "fb",
                    "Pwd": "admin"
                },
                "Security": {
                    "ElevationSecret": ""
                }
            }

            for(const [id, mysqlConf] of Object.entries(conf.mysql[sectionConf.mysql])) {
                config.ConnectionStrings.MySql[id] = `Server=mysql-${sectionConf.mysql};Port=${mysqlConf.port.cluster};User ID=fb; Password=admin; Database=fb`
            }

            for(const [id, redisConf] of Object.entries(conf.redis[sectionConf.redis])) {
                config.Redis[id] = {
                    Host: `redis-${sectionConf.redis}`,
                    Port: conf.redis[sectionConf.redis][id].port.cluster
                }
            }

            if (sectionConf.security && sectionConf.security.elevationSecret) {
                config.Security.ElevationSecret = sectionConf.security.elevationSecret
            }

            const configMap = new k8s.core.v1.ConfigMap(`admin-tool-${section}`, {
                metadata: { name: `admin-tool-${section}`, namespace: namespace.metadata.name },
                data: {
                    "appsettings.k8s.json": JSON.stringify(config),
                },
            })

            const deployment = new k8s.apps.v1.Deployment(`admin-tool-${section}`, {
                metadata: { name: `admin-tool-${section}`, namespace: namespace.metadata.name },
                spec: {
                    selector: { matchLabels: appLabels },
                    replicas: sectionConf.replicas,
                    template: {
                        metadata: { labels: appLabels },
                        spec: {
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

            const service = new k8s.core.v1.Service(`admin-tool-${section}`, {
                metadata: { name: `admin-tool-${section}`, namespace: namespace.metadata.name },
                spec: {
                    type: "NodePort",
                    ports: [{ 
                        name: `admin-tool-${section}`,
                        port: sectionConf.port.cluster,
                        targetPort: `admin-tool`,
                        protocol: "TCP",
                        nodePort: sectionConf.port.node 
                    }],
                    selector: appLabels,
                },
            }, { dependsOn: dependsOn })
        }
    }
}

