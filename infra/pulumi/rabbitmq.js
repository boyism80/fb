const pulumi = require("@pulumi/pulumi");
const k8s = require("@pulumi/kubernetes");

module.exports = {
    setup: function (namespace, conf) {
        let index = 0
        const ports = []

        for(const [section, sectionConf] of Object.entries(conf.rabbitmq)) {
            const statefulSet = new k8s.apps.v1.StatefulSet(`rabbitmq-${section}`, {
                metadata: { name: `rabbitmq-${section}`, namespace: namespace.metadata.name, },
                spec: {
                    serviceName: "rabbitmq",
                    replicas: 1,
                    selector: { matchLabels: { app: "rabbitmq" } },
                    template: {
                        metadata: { labels: { app: "rabbitmq" } },
                        spec: {
                            nodeSelector: {
                                "kubernetes.io/hostname": "ubuntu-1"
                            },
                            containers: [{
                                name: "rabbitmq",
                                image: "rabbitmq:management",
                                ports: [
                                    { name: `amqp-${index}`, containerPort: 5672 },
                                    { name: `management-${index}`, containerPort: 15672 },
                                ],
                                env: [
                                    { name: "RABBITMQ_DEFAULT_USER", value: "fb" }, // Default user
                                    { name: "RABBITMQ_DEFAULT_PASS", value: "admin" }, // Default user password
                                    { name: "RABBITMQ_ERLANG_COOKIE", value: "secret-cookie" },
                                    { name: "K8S_SERVICE_NAME", value: "rabbitmq" },
                                ],
                                volumeMounts: [{ 
                                    name: `data-volume-${section}`, 
                                    mountPath: "/var/lib/rabbitmq" 
                                }],
                            }],
                            volumes: [
                            {
                                name: `data-volume-${section}`,
                                hostPath: {
                                    path: `/mnt/fb/rabbitmq/${section}`,
                                    type: "DirectoryOrCreate"
                                },
                            }]
                        },
                    }
                },
            })

            ports.push({
                name: `amqp-${section}`,
                port: sectionConf.port.amqp.cluster,
                targetPort: `amqp-${index}`,
                nodePort: sectionConf.port.amqp.node,
            })
            ports.push({
                name: `management-${section}`,
                port: sectionConf.port.management.cluster,
                targetPort: `management-${index}`,
                nodePort: sectionConf.port.management.node,
            })

            index++
        }

        return new k8s.core.v1.Service("rabbitmq", {
            metadata: { name: "rabbitmq", namespace: namespace.metadata.name },
            spec: {
                type: "NodePort",
                selector: { app: "rabbitmq" },
                ports: ports,
            },
        });
    }
}