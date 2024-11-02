const pulumi = require("@pulumi/pulumi")
const k8s = require("@pulumi/kubernetes")

module.exports = {
    setup: function (namespace, args) {

        const ports = []
        const appLabels = { app: "db" }
        for(const [i, arg] of Object.entries(args)) {
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
                            "Port": arg.redis.port.cluster
                        }
                    ]
                }
            }

            for(const [key, mysqlConfig] of Object.entries(arg.mysql)) {
                config.ConnectionStrings.MySql[key] = `Server=mysql;Port=${mysqlConfig.port.cluster};User ID=fb; Password=admin; Database=fb`
            }

            const configMap = new k8s.core.v1.ConfigMap(`db-${i}`, {
                metadata: { name: `db-${i}`, namespace: namespace.metadata.name },
                data: {
                    "appsettings.k8s.json": JSON.stringify(config),
                },
            })

            const deployment = new k8s.apps.v1.Deployment(`db-${i}`, {
                metadata: { name: `db-${i}`, namespace: namespace.metadata.name },
                spec: {
                    selector: { matchLabels: appLabels },
                    replicas: 1,
                    template: {
                        metadata: { labels: appLabels },
                        spec: {
                            containers: [{
                                name: "db",
                                image: "cshyeon/fb:db",
                                ports: [{ containerPort: 80, name: `db-${i}` }],
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
                name: `db-${i}`,
                port: arg.port.cluster,
                targetPort: `db-${i}`,
                protocol: "TCP",
                nodePort: arg.port.node
            })
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
