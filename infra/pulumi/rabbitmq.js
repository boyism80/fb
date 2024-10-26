const pulumi = require("@pulumi/pulumi");
const k8s = require("@pulumi/kubernetes");

module.exports = {
    setup: function (namespace, storageClass) {
        const pv = new k8s.core.v1.PersistentVolume(`rabbitmq-pv`, {
            metadata: {
                name: `rabbitmq-pv`,
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
                    path: `/mnt/fb/rabbitmq`,
                    type: "DirectoryOrCreate"
                },
            },
        }, { dependsOn: [storageClass] });

        // Create a RabbitMQ StatefulSet with 1 master and 2 worker nodes
        const statefulSet = new k8s.apps.v1.StatefulSet("rabbitmq-statefulSet", {
            metadata: { name: "rabbitmq", namespace: namespace.metadata.name, },
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
                                { name: "amqp", containerPort: 5672 },
                                { name: "management", containerPort: 15672 },
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

        // Create a NodePort service for external access to RabbitMQ
        const service = new k8s.core.v1.Service("rabbitmq-service", {
            metadata: { name: "rabbitmq-service", namespace: namespace.metadata.name },
            spec: {
                type: "NodePort",
                selector: { app: "rabbitmq" },
                ports: [
                    {
                        name: "amqp",
                        port: 5672,
                        targetPort: 5672,
                        nodePort: 30010, // Node port for AMQP (RabbitMQ Messaging)
                    },
                    {
                        name: "management",
                        port: 15672,
                        targetPort: 15672,
                        nodePort: 30011, // Node port for RabbitMQ Management
                    },
                ],
            },
        });
    }
}