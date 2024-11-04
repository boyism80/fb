const pulumi = require("@pulumi/pulumi")
const k8s = require("@pulumi/kubernetes")


module.exports = {
    setup: function (namespace, storageClass, conf) {

        let index = 0
        const ports = []
        for(const [section, sectionConf] of Object.entries(conf.redis)) {
            const pv = new k8s.core.v1.PersistentVolume(`redis-${section}`, {
                metadata: {
                    name: `redis-${section}`,
                    namespace: namespace.metadata.name,
                },
                spec: {
                    capacity: {
                        storage: "1Gi",
                    },
                    accessModes: ["ReadWriteOnce"],
                    persistentVolumeReclaimPolicy: "Retain",
                    storageClassName: storageClass.metadata.name,
                    hostPath: {
                        path: `/mnt/fb/redis/${section}`,
                        type: "DirectoryOrCreate"
                    },
                },
            }, { dependsOn: [storageClass] });

            const statefulSet = new k8s.apps.v1.StatefulSet(`redis-${section}`, {
                metadata: {
                    name: `redis-${section}`,
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
                                            name: `redis-${index}`,
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
                    persistentVolumeClaimRetentionPolicy: {
                        whenDeleted: 'Delete',
                        whenScaled: 'Delete'
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

            ports.push({
                name: `redis-${section}`,
                port: sectionConf.port.cluster,
                targetPort: `redis-${index}`,
                nodePort: sectionConf.port.node,
            })

            index++
        }
        
        return new k8s.core.v1.Service("redis", {
            metadata: {
                namespace: namespace.metadata.name,
                name: "redis",
            },
            spec: {
                type: "NodePort",
                selector: {
                    app: "redis",
                },
                ports: ports,
            },
        })
    }
}
