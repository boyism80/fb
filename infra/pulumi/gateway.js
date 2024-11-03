const pulumi = require("@pulumi/pulumi");
const k8s = require("@pulumi/kubernetes");

module.exports = function () {

    return {
        setup: function (namespace, conf) {

            for(let i = 0; i < conf.gateway.length; i++) {
                const containerConf = conf.gateway[i]
                const config = {
                    id: i,
                    name: `gateway-${i}`,
                    ip:  conf.host,
                    port: containerConf.port,
                    thread: {
                        logic: 12,
                        io: 12,
                        background: 8
                    },
                    log: ["info", "fatal"],
                    entrypoints: []
                }

                for(const [section, loginContainerConf] of Object.entries(conf.login)) {
                    config.entrypoints.push({
                        name: loginContainerConf.name,
                        desc: loginContainerConf.desc,
                        ip: conf.host,
                        port: loginContainerConf.port.node
                    })
                }

                const configMap = new k8s.core.v1.ConfigMap(`gateway-${i}`, {
                    metadata: { name: `gateway-${i}`, namespace: namespace.metadata.name },
                    data: {
                        "config.json": JSON.stringify(config),
                    },
                })

                const statefulSet = new k8s.apps.v1.StatefulSet(`gateway-${i}`, {
                    metadata: {
                        name: `gateway-${i}`,
                        namespace: namespace.metadata.name,
                    },
                    spec: {
                        serviceName: "gateway",
                        replicas: 1,
                        selector: {
                            matchLabels: {
                                app: "gateway",
                            },
                        },
                        template: {
                            metadata: {
                                labels: {
                                    app: "gateway",
                                },
                            },
                            spec: {
                                containers: [
                                    {
                                        name: "gateway",
                                        image: "cshyeon/fb:gateway",
                                        ports: [
                                            { containerPort: containerConf.port, name: `gateway-${i}` },
                                        ],
                                        volumeMounts: [{
                                            name: "config-volume",
                                            mountPath: "/app/config/config.json",
                                            subPath: "config.json"
                                        }],
                                    },
                                ],
                                volumes: [{
                                    name: "config-volume",
                                    configMap: {
                                        name: configMap.metadata.name,
                                    },
                                }],
                            },
                        },
                    },
                });
            }
        }
    }
}()