const pulumi = require("@pulumi/pulumi");
const k8s = require("@pulumi/kubernetes");

module.exports = function () {

    return {
        setup: function (namespace, conf) {

            secret = new k8s.core.v1.Secret("mysql-secret", {
                metadata: {
                    name: "mysql-secret",
                    namespace: namespace.metadata.name,
                },
                stringData: {
                    MYSQL_ROOT_PASSWORD: "admin",
                    MYSQL_USER: "fb",
                    MYSQL_PASSWORD: "admin"
                }
            })
            configMap = new k8s.core.v1.ConfigMap("init-sql", {
                metadata: { 
                    name: "init-sql", 
                    namespace: namespace.metadata.name
                },
                data: {
                    "init-user.sql": `
                        GRANT ALL PRIVILEGES ON *.* TO 'fb'@'%' WITH GRANT OPTION;
                        FLUSH PRIVILEGES;
                    `
                }
            })

            let index = 0
            const ports = []
            for(const [section, sectionConfs] of Object.entries(conf.mysql)) {
                for(const [id, sectionConf] of Object.entries(sectionConfs)) {
                    const statefulSet = new k8s.apps.v1.StatefulSet(`mysql-${section}-${id}`, {
                        metadata: {
                            name: `mysql-${section}-${id}`,
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
                                    nodeSelector: {
                                        "kubernetes.io/hostname": "ubuntu-1"
                                    },
                                    containers: [
                                        {
                                            name: "mysql",
                                            image: "mysql:8.4.3",
                                            conf: [
                                                "--mysql-native-password=ON"
                                            ],
                                            ports: [
                                                { containerPort: 3306, name: `mysql-${index}` },
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
                                                    name: `data-volume-${section}-${id}`,
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
                                        name: `data-volume-${section}-${id}`,
                                        hostPath: {
                                            path: `/mnt/fb/mysql/${section}/${id}`,
                                            type: "DirectoryOrCreate"
                                        }
                                    },
                                    {
                                        name: "init-sql-volume",
                                        configMap: { name: configMap.metadata.name }
                                    }]
                                },
                            }
                        },
                    }, { dependsOn: [secret] });

                    ports.push({
                        name: `mysql-${section}-${id}`,
                        port: sectionConf.port.cluster,
                        targetPort: `mysql-${index}`,
                        nodePort: sectionConf.port.node, 
                    })

                    index++
                }
            }

            return new k8s.core.v1.Service('mysql', {
                metadata: {
                    name: 'mysql',
                    namespace: namespace.metadata.name,
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