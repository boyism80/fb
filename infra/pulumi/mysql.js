const pulumi = require("@pulumi/pulumi");
const k8s = require("@pulumi/kubernetes");

module.exports = function () {
    let $secret = null
    let $configMap = null
    let $namespace = null
    let $storageClass = null

    return {
        init: function (namespace, storageClass) {
            $namespace = namespace
            $storageClass = storageClass
            $secret = new k8s.core.v1.Secret("mysql-secret", {
                metadata: {
                    name: "mysql-secret",
                    namespace: $namespace.metadata.name,
                },
                stringData: {
                    MYSQL_ROOT_PASSWORD: "admin",
                    MYSQL_USER: "fb",
                    MYSQL_PASSWORD: "admin"
                }
            })
            $configMap = new k8s.core.v1.ConfigMap("init-sql", {
                metadata: { 
                    name: "init-sql", 
                    namespace: $namespace.metadata.name
                },
                data: {
                    "init-user.sql": `
                        GRANT ALL PRIVILEGES ON *.* TO 'fb'@'%' WITH GRANT OPTION;
                        FLUSH PRIVILEGES;
                    `
                }
            })
        },
        create: function (name, index) {
            if ($secret == null)
                throw 'secret cannot be null'

            if ($configMap == null)
                throw 'config map cannot be null'

            const pv = new k8s.core.v1.PersistentVolume(`${name}-pv`, {
                metadata: {
                    name: `${name}-pv`,
                    namespace: $namespace.metadata.name, 
                },
                spec: {
                    capacity: {
                        storage: "1Gi",
                    },
                    accessModes: ["ReadWriteOnce"],
                    persistentVolumeReclaimPolicy: "Retain",
                    storageClassName: $storageClass.metadata.name,
                    hostPath: {
                        path: `/mnt/fb/mysql/${name}`,
                        type: "DirectoryOrCreate"
                    },
                },
            }, { dependsOn: [$storageClass] });

            const statefulSet = new k8s.apps.v1.StatefulSet(`${name}`, {
                metadata: {
                    name: `${name}`,
                    namespace: $namespace.metadata.name,
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
                                        { containerPort: 3306, name: `mysql-${index}` },
                                    ],
                                    env: [
                                        {
                                            name: "MYSQL_ROOT_PASSWORD",
                                            valueFrom: { secretKeyRef: { name: $secret.metadata.name, key: "MYSQL_ROOT_PASSWORD" } },
                                        },
                                        {
                                            name: "MYSQL_USER", 
                                            valueFrom: { secretKeyRef: { name: $secret.metadata.name, key: "MYSQL_USER" } }
                                        },
                                        { 
                                            name: "MYSQL_PASSWORD", 
                                            valueFrom: { secretKeyRef: { name: $secret.metadata.name, key: "MYSQL_PASSWORD" } }
                                        },
                                    ],
                                    volumeMounts: [
                                        {
                                            name: `${name}-pvc`,
                                            mountPath: "/var/lib/mysql",
                                        },
                                        {
                                            name: "init-sql-volume",
                                            mountPath: "/docker-entrypoint-initdb.d"
                                        }
                                    ],
                                },
                            ],
                            volumes: [
                            {
                                name: "init-sql-volume",
                                configMap: { name: $configMap.metadata.name }
                            }]
                        },
                    },
                    persistentVolumeClaimRetentionPolicy: {
                        whenDeleted: 'Delete',
                        whenScaled: 'Delete'
                    },
                    volumeClaimTemplates: [
                        {
                            metadata: {
                                name: `${name}-pvc`,
                            },
                            spec: {
                                accessModes: ["ReadWriteOnce"],
                                resources: {
                                    requests: {
                                        storage: "1Gi",
                                    },
                                },
                                storageClassName: $storageClass.metadata.name,
                            },
                        },
                    ],
                },
            }, { dependsOn: [pv] });

            return { pv, statefulSet}
        },
        setup: function (args) {
            if ($secret == null)
                throw 'secret cannot be null'

            if ($configMap == null)
                throw 'config map cannot be null'

            const ports = []
            for(const arg of args) {
                ports.push({
                    name: `${arg.name}`,
                    port: arg.clusterPort,
                    targetPort: `mysql-${arg.index}`,
                    nodePort: arg.nodePort, 
                })
            }
            
            return new k8s.core.v1.Service('mysql', {
                metadata: {
                    name: 'mysql',
                    namespace: $namespace.metadata.name,
                },
                spec: {
                    type: "NodePort",
                    ports: ports,
                    selector: {
                        app: "mysql",
                    },
                },
            })
        }
    }
}()