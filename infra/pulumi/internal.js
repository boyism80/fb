const pulumi = require("@pulumi/pulumi")
const k8s = require("@pulumi/kubernetes")

module.exports = {
    setup: function (namespace, conf, dependsOn) {

        let index = 0
        const ports = []
        const appLabels = { app: "internal" }
        for(const [section, sectionConf] of Object.entries(conf.internal)) {
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
                "Redis": {
                    "AllowAdmin": true,
                    "ConnectTimeout": 6000,
                    "ConnectRetry": 2,
                    "Database": 0,
                    "Hosts": [
                        {
                            "Host": "redis",
                            "Port": conf.redis[sectionConf.redis].port.cluster
                        }
                    ]
                },
                "RabbitMQ": {
                    "Host": "rabbitmq",
                    "Port": conf.rabbitmq[sectionConf.redis].port.amqp.cluster,
                    "Uid": "fb",
                    "Pwd": "admin"
                }
            }

            for(const [id, mysqlConfig] of Object.entries(conf.mysql[sectionConf.mysql])) {
                config.ConnectionStrings.MySql[id] = `Server=mysql;Port=${mysqlConfig.port.cluster};User ID=fb; Password=admin; Database=fb`
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
                                name: "internal",
                                image: "ghcr.io/boyism80/fb/internal:latest",
                                imagePullPolicy: "Always",
                                securityContext: {
                                    capabilities: {
                                        add: ["SYS_PTRACE"]
                                    }
                                },
                                ports: [{ containerPort: 80, name: `internal-${index}` }],
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

            ports.push({ 
                name: `internal-${section}`,
                port: sectionConf.port.cluster,
                targetPort: `internal-${index}`,
                protocol: "TCP",
                nodePort: sectionConf.port.node 
            })

            index++
        }

        return new k8s.core.v1.Service("internal", {
            metadata: { name: "internal", namespace: namespace.metadata.name },
            spec: {
                type: "NodePort",
                ports: ports,
                selector: appLabels,
            },
        }, { dependsOn: dependsOn })
    }
}
