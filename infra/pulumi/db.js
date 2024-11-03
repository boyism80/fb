const pulumi = require("@pulumi/pulumi")
const k8s = require("@pulumi/kubernetes")

module.exports = {
    setup: function (namespace, conf) {
        let index = 0
        const ports = []
        const appLabels = { app: "db" }

        for(const [section, sectionConf] of Object.entries(conf.db)) {
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
                }
            }

            for(const [id, mysqlConfig] of Object.entries(conf.mysql[sectionConf.mysql])) {
                config.ConnectionStrings.MySql[id] = `Server=mysql;Port=${mysqlConfig.port.cluster};User ID=fb; Password=admin; Database=fb`
            }

            const configMap = new k8s.core.v1.ConfigMap(`db-${section}`, {
                metadata: { name: `db-${section}`, namespace: namespace.metadata.name },
                data: {
                    "appsettings.k8s.json": JSON.stringify(config),
                },
            })

            const deployment = new k8s.apps.v1.Deployment(`db-${section}`, {
                metadata: { name: `db-${section}`, namespace: namespace.metadata.name },
                spec: {
                    selector: { matchLabels: appLabels },
                    replicas: 1,
                    template: {
                        metadata: { labels: appLabels },
                        spec: {
                            containers: [{
                                name: "db",
                                image: "cshyeon/fb:db",
                                ports: [{ containerPort: 80, name: `db-${index}` }],
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
            })

            ports.push({ 
                name: `db-${section}`,
                port: sectionConf.port.cluster,
                targetPort: `db-${section}`,
                protocol: "TCP",
                nodePort: sectionConf.port.node
            })

            index++
        }

        const service = new k8s.core.v1.Service("db", {
            metadata: { name: "db", namespace: namespace.metadata.name },
            spec: {
                type: "NodePort",
                ports: ports,
                selector: appLabels,
            },
        })
    }
}
