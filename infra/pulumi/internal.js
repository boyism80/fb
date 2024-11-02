const pulumi = require("@pulumi/pulumi")
const k8s = require("@pulumi/kubernetes")

module.exports = {
    setup: function (namespace, args) {

        const ports = []
        const appLabels = { app: "internal" }
        for(const [i, arg] of Object.entries(args)) {
            const config = {
                "Logging": {
                    "LogLevel": {
                        "Default": "Information",
                        "Microsoft.AspNetCore": "Warning"
                    }
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
                },
                "RabbitMQ": {
                    "Host": "rabbitmq",
                    "Port": arg.rabbitmq.port.amqp.cluster,
                    "Uid": "fb",
                    "Pwd": "admin"
                }
            }

            const configMap = new k8s.core.v1.ConfigMap(`internal-${i}`, {
                metadata: { name: `internal-${i}`, namespace: namespace.metadata.name },
                data: {
                    "appsettings.k8s.json": JSON.stringify(config),
                },
            })

            const deployment = new k8s.apps.v1.Deployment(`internal-${i}`, {
                metadata: { name: `internal-${i}`, namespace: namespace.metadata.name },
                spec: {
                    selector: { matchLabels: appLabels },
                    replicas: 1,
                    template: {
                        metadata: { labels: appLabels },
                        spec: {
                            containers: [{
                                name: "internal",
                                image: "cshyeon/fb:internal",
                                ports: [{ containerPort: 80, name: `internal-${i}` }],
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
                name: `internal-${i}`,
                port: arg.port.cluster,
                targetPort: `internal-${i}`,
                protocol: "TCP",
                nodePort: arg.port.node 
            })
        }

        const service = new k8s.core.v1.Service("internal", {
            metadata: { name: "internal", namespace: namespace.metadata.name },
            spec: {
                type: "NodePort",
                ports: ports,
                selector: appLabels,
            },
        })

        return ports
    }
}
