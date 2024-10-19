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
    });

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
                            ports: [
                                { containerPort: 3306, name: `mysql-${i}` },
                            ],
                            env: [
                                {
                                    name: "MYSQL_ROOT_PASSWORD",
                                    valueFrom: {
                                        secretKeyRef: {
                                            name: mysqlSecret.metadata.name,
                                            key: "MYSQL_ROOT_PASSWORD",
                                        },
                                    },
                                },
                            ],
                            volumeMounts: [
                                {
                                    name: "mysql-persistent-storage",
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
                        name: "mysql-persistent-storage",
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
    });
}

// Define the Namespace
const namespace = new k8s.core.v1.Namespace("fb-namespace", {
    metadata: {
        name: "fb",
    }
});

// Create a Secret to store the MySQL password
const mysqlSecret = new k8s.core.v1.Secret("mysql-secret", {
    metadata: {
        name: "mysql-secret",
        namespace: namespace.metadata.name,
    },
    stringData: {
        MYSQL_ROOT_PASSWORD: "admin"
    }
});

const storageClass = new k8s.storage.v1.StorageClass("storage-class", {
    metadata: {
        name: "storage-class",
        namespace: namespace.metadata.name, // Set namespace to 'fb'
    },
    provisioner: "kubernetes.io/no-provisioner",
    volumeBindingMode: "WaitForFirstConsumer",
});

const ports = []
for(let i = 0; i < 4; i++) {
    setupMySql(namespace, mysqlSecret, storageClass, i)
    ports.push({
        name: `mysql-${i}`,
        port: 3306+i,
        targetPort: `mysql-${i}`,
        nodePort: 30000 + i, // Example NodePort. You can choose a port within the range 30000-32767
    })
}

// Create a NodePort Service for MySQL
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