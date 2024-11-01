const pulumi = require("@pulumi/pulumi");
const k8s = require("@pulumi/kubernetes");

module.exports = {
    setup: function (namespace, storageClass, args) {
        const ports = []
        for(let i = 0; i < args.length; i++) {
            const arg = args[i]
            const pv = new k8s.core.v1.PersistentVolume(`rabbitmq-${i}`, {
                metadata: {
                    name: `rabbitmq-${i}`,
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
                        path: `/mnt/fb/rabbitmq/${i}`,
                        type: "DirectoryOrCreate"
                    },
                },
            }, { dependsOn: [storageClass] });

            const statefulSet = new k8s.apps.v1.StatefulSet(`rabbitmq-${i}`, {
                metadata: { name: `rabbitmq-${i}`, namespace: namespace.metadata.name, },
                spec: {
                    serviceName: "rabbitmq-headless",
                    replicas: 1,
                    selector: { matchLabels: { app: "rabbitmq" } },
                    template: {
                        metadata: { labels: { app: "rabbitmq" } },
                        spec: {
                            containers: [{
                                name: "rabbitmq",
                                image: "rabbitmq:management",
                                ports: [
                                    { name: `amqp-${i}`, containerPort: 5672 },
                                    { name: `management-${i}`, containerPort: 15672 },
                                ],
                                env: [
                                    { name: "RABBITMQ_DEFAULT_USER", value: "fb" }, // Default user
                                    { name: "RABBITMQ_DEFAULT_PASS", value: "admin" }, // Default user password
                                    { name: "RABBITMQ_ERLANG_COOKIE", value: "secret-cookie" },
                                    { name: "K8S_SERVICE_NAME", value: "rabbitmq" },
                                ],
                                volumeMounts: [{ name: "rabbitmq-data", mountPath: "/var/lib/rabbitmq" }],
                            }],
                        },
                    },
                    persistentVolumeClaimRetentionPolicy: {
                        whenDeleted: 'Delete',
                        whenScaled: 'Delete'
                    },
                    volumeClaimTemplates: [{
                        metadata: { name: "rabbitmq-data" },
                        spec: {
                            accessModes: ["ReadWriteOnce"],
                            resources: { requests: { storage: "1Gi" } },
                            storageClassName: storageClass.metadata.name,
                        },
                    }],
                },
            }, { dependsOn: [pv] });

            ports.push({
                name: `amqp-${i}`,
                port: arg.port.amqp.cluster,
                targetPort: `amqp-${i}`,
                nodePort: arg.port.amqp.node,
            })
            ports.push({
                name: `management-${i}`,
                port: arg.port.management.cluster,
                targetPort: `management-${i}`,
                nodePort: arg.port.management.node,
            })
        }

        const service = new k8s.core.v1.Service("rabbitmq", {
            metadata: { name: "rabbitmq", namespace: namespace.metadata.name },
            spec: {
                type: "NodePort",
                selector: { app: "rabbitmq" },
                ports: ports,
            },
        });

        return ports
    }
}