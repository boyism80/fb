const pulumi = require("@pulumi/pulumi")
const k8s = require("@pulumi/kubernetes")


module.exports = {
    setup: function (namespace, storageClass) {
        const pv = new k8s.core.v1.PersistentVolume(`redis-pv`, {
            metadata: {
                name: `redis-pv`,
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
                    path: `/mnt/fb/redis`,
                    type: "DirectoryOrCreate"
                },
            },
        }, { dependsOn: [storageClass] });

        const statefulSet = new k8s.apps.v1.StatefulSet("redis-statefulset", {
            metadata: {
                name: "redis",
                namespace: namespace.metadata.name,
            },
            spec: {
                serviceName: "redis",
                replicas: 1,
                selector: {
                    matchLabels: {
                        app: "redis",
                    },
                },
                template: {
                    metadata: {
                        labels: {
                            app: "redis",
                        },
                    },
                    spec: {
                        containers: [
                            {
                                name: "redis",
                                image: "redis:6.2.4",
                                ports: [
                                    {
                                        containerPort: 6379,
                                        name: "redis",
                                    },
                                ],
                                volumeMounts: [
                                    {
                                        name: "redis-data",
                                        mountPath: "/data",
                                    },
                                ],
                            },
                        ],
                    },
                },
                volumeClaimTemplates: [
                    {
                        metadata: {
                            name: "redis-data",
                        },
                        spec: {
                            accessModes: ["ReadWriteOnce"],
                            storageClassName: storageClass.metadata.name,
                            resources: {
                                requests: {
                                    storage: "1Gi",
                                },
                            },
                        },
                    },
                ],
            },
        })


        const service = new k8s.core.v1.Service("redis-nodeport", {
            metadata: {
                namespace: namespace.metadata.name,
                name: "redis-service",
            },
            spec: {
                type: "NodePort",
                selector: {
                    app: "redis",
                },
                ports: [{
                    port: 6379, // Port Redis is running on inside the container
                    targetPort: 6379,
                    nodePort: 30020, // External port to access Redis
                }],
            },
        })

        return { statefulSet, service }
    }
}
