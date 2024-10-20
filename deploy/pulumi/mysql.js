const pulumi = require("@pulumi/pulumi");
const k8s = require("@pulumi/kubernetes");

function setupMySql(namespace, secret, storageClass, i) {
    // Define PersistentVolume (this assumes the volume already exists as per previous setup)
    const pv = new k8s.core.v1.PersistentVolume(`mysql-pv-${i}`, {
        metadata: {
            name: `mysql-pv-${i}`,
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
                path: `/mnt/fb/mysql/${i}`,
                type: "DirectoryOrCreate"
            },
        },
    }, { dependsOn: [storageClass] });

    // Create a StatefulSet for MySQL
    const statefulSet = new k8s.apps.v1.StatefulSet(`mysql-${i}`, {
        metadata: {
            name: `mysql-${i}`,
            namespace: namespace.metadata.name,
        },
        spec: {
            serviceName: "mysql",
            replicas: 1,
            selector: {
                matchLabels: {
                    app: "mysql",
                },
            },
            template: {
                metadata: {
                    labels: {
                        app: "mysql",
                    },
                },
                spec: {
                    containers: [
                        {
                            name: "mysql",
                            image: "mysql:8.4.3",
                            args: [
                                "--mysql-native-password=ON"
                            ],
                            ports: [
                                { containerPort: 3306, name: `mysql-${i}` },
                            ],
                            env: [
                                {
                                    name: "MYSQL_ROOT_PASSWORD",
                                    valueFrom: { secretKeyRef: { name: secret.metadata.name, key: "MYSQL_ROOT_PASSWORD" } },
                                },
                                {
                                    name: "MYSQL_USER", 
                                    valueFrom: { secretKeyRef: { name: secret.metadata.name, key: "MYSQL_USER" } }
                                },
                                { 
                                    name: "MYSQL_PASSWORD", 
                                    valueFrom: { secretKeyRef: { name: secret.metadata.name, key: "MYSQL_PASSWORD" } }
                                },
                            ],
                            volumeMounts: [
                                {
                                    name: `mysql-persistent-storage-${i}`,
                                    mountPath: "/var/lib/mysql",
                                },
                            ],
                        },
                    ],
                },
            },
            volumeClaimTemplates: [
                {
                    metadata: {
                        name: `mysql-persistent-storage-${i}`,
                    },
                    spec: {
                        accessModes: ["ReadWriteOnce"],
                        resources: {
                            requests: {
                                storage: "1Gi",
                            },
                        },
                        storageClassName: storageClass.metadata.name,
                    },
                },
            ],
        },
    }, { dependsOn: [pv] });
}

module.exports = {
    setup: function (namespace, storageClass, size) {
        const mysqlSecret = new k8s.core.v1.Secret("mysql-secret", {
            metadata: {
                name: "mysql-secret",
                namespace: namespace.metadata.name,
            },
            stringData: {
                MYSQL_ROOT_PASSWORD: "admin",
                MYSQL_USER: "fb",
                MYSQL_PASSWORD: "admin"
            }
        });

        const ports = []
        for(let i = 0; i < size; i++) {
            setupMySql(namespace, mysqlSecret, storageClass, i)
            ports.push({
                name: `mysql-${i}`,
                port: 3306+i,
                targetPort: `mysql-${i}`,
                nodePort: 30000 + i, 
            })
        }

        
        const mysqlService = new k8s.core.v1.Service("mysql-service", {
            metadata: {
                name: "mysql-service",
                namespace: namespace.metadata.name,
            },
            spec: {
                type: "NodePort",
                ports: ports,
                selector: {
                    app: "mysql",
                },
            },
        });
    }
}