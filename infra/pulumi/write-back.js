const pulumi = require("@pulumi/pulumi")
const k8s = require("@pulumi/kubernetes")

module.exports = {
    setup: function (namespace, conf, dependsOn) {

        let index = 0
        const ports = []
        const appLabels = { app: "write-back" }
        for(const [section, sectionConf] of Object.entries(conf['write-back'])) {
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
                "Redis": {}
            }

            for(const [id, mysqlConfig] of Object.entries(conf.mysql[sectionConf.mysql])) {
                config.ConnectionStrings.MySql[id] = `Server=mysql;Port=${mysqlConfig.port.cluster};User ID=fb; Password=admin; Database=fb`
            }

            for(const [id, redisConf] of Object.entries(conf.redis[sectionConf.redis])) {
                config.Redis[id] = {
                    AllowAdmin: true,
                    ConnectTimeout: 6000,
                    ConnectRetry: 2,
                    Database: 0,
                    Hosts: [
                        {
                            Host: "redis",
                            Port: conf.redis[sectionConf.redis][id].port.cluster
                        }
                    ]
                }
            }

            const configMap = new k8s.core.v1.ConfigMap(`write-back-${section}`, {
                metadata: { name: `write-back-${section}`, namespace: namespace.metadata.name },
                data: {
                    "appsettings.json": JSON.stringify(config),
                },
            })

            return new k8s.apps.v1.Deployment(`write-back-${section}`, {
                metadata: { name: `write-back-${section}`, namespace: namespace.metadata.name },
                spec: {
                    selector: { matchLabels: appLabels },
                    replicas: 1,
                    template: {
                        metadata: { labels: appLabels },
                        spec: {
                            containers: [{
                                name: "write-back",
                                image: "ghcr.io/boyism80/fb/write-back:latest",
                                imagePullPolicy: "Always",
                                securityContext: {
                                    capabilities: {
                                        add: ["SYS_PTRACE"]
                                    }
                                },
                                volumeMounts: [{
                                    name: "config-volume",
                                    mountPath: "/app/appsettings.json",
                                    subPath: "appsettings.json"
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
        }
    }
}
