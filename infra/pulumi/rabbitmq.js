const pulumi = require("@pulumi/pulumi");
const k8s = require("@pulumi/kubernetes");

module.exports = {
    setup: function (namespace, storageClass, conf) {
        let index = 0
        const ports = []

        for(const [section, sectionConf] of Object.entries(conf.rabbitmq)) {
            const pv = new k8s.core.v1.PersistentVolume(`rabbitmq-${section}`, {
                metadata: {
                    name: `rabbitmq-${section}`,
                    namespace: namespace.metadata.name, // Set namespace to 'fb'
                },
                spec: {
                    capacity: {
                        storage: "1Gi",
                    },
                    accessModes: ["ReadWriteOnce"],
                    persistentVolumeReclaimPolicy: "Retain",
                    storageClassName: storageClass.metadata.name,
                    hostPath: {
                        path: `/mnt/fb/rabbitmq/${section}`,
                        type: "DirectoryOrCreate"
                    },
                },
            }, { dependsOn: [storageClass] })

            const statefulSet = new k8s.apps.v1.StatefulSet(`rabbitmq-${section}`, {
                metadata: { name: `rabbitmq-${section}`, namespace: namespace.metadata.name, },
                spec: {
                    serviceName: "rabbitmq",
                    replicas: 1,
                    selector: { matchLabels: { app: "rabbitmq" } },
                    template: {
                        metadata: { labels: { app: "rabbitmq" } },
                        spec: {
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
                                volumeMounts: [{ name: `rabbitmq-${section}`, mountPath: "/var/lib/rabbitmq" }],
                            }],
                        },
                    },
                    persistentVolumeClaimRetentionPolicy: {
                        whenDeleted: 'Delete',
                        whenScaled: 'Delete'
                    },
                    volumeClaimTemplates: [{
                        metadata: { name: `rabbitmq-${section}` },
                        spec: {
                            accessModes: ["ReadWriteOnce"],
                            resources: { requests: { storage: "1Gi" } },
                            storageClassName: storageClass.metadata.name,
                        },
                    }],
                },
            }, { dependsOn: [pv] })

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