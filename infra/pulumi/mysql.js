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

            const resources = []
            let globalIndex = 0
            for(const [section, sectionConfs] of Object.entries(conf.mysql)) {
                // Process data MySQL instances
                if (sectionConfs.data) {
                    const serviceName = `mysql-${section}`
                    const clusterPorts = []
                    const nodePorts = []
                    
                    // Collect all ports first with unique containerPort names using global index
                    for(const [id, sectionConf] of Object.entries(sectionConfs.data)) {
                        const containerPortName = `m${globalIndex}`
                        clusterPorts.push({
                            name: `mysql-${section}-${id}`,
                            port: sectionConf.port.cluster,
                            targetPort: containerPortName,
                        })
                        nodePorts.push({
                            name: `mysql-${section}-${id}`,
                            port: sectionConf.port.cluster,
                            targetPort: containerPortName,
                            nodePort: sectionConf.port.node,
                        })
                        globalIndex++
                    }
                    
                    // Create ClusterIP service for internal access with all ports
                    const clusterIPService = new k8s.core.v1.Service(`mysql-${section}`, {
                        metadata: { name: serviceName, namespace: namespace.metadata.name },
                        spec: {
                            type: "ClusterIP",
                            selector: { app: "mysql", section: section, type: "data" },
                            ports: clusterPorts,
                        },
                    });
                    
                    // Create StatefulSets
                    const statefulSets = []
                    globalIndex = globalIndex - Object.keys(sectionConfs.data).length
                    for(const [id, sectionConf] of Object.entries(sectionConfs.data)) {
                        const containerPortName = `m${globalIndex}`
                        globalIndex++
                        const statefulSet = new k8s.apps.v1.StatefulSet(`mysql-${section}-${id}`, {
                            metadata: {
                                name: `mysql-${section}-${id}`,
                                namespace: namespace.metadata.name,
                            },
                            spec: {
                                serviceName: serviceName,
                                replicas: 1,
                                selector: {
                                    matchLabels: {
                                        app: "mysql",
                                        section: section,
                                        type: "data",
                                        id: id,
                                    },
                                },
                                template: {
                                    metadata: {
                                        labels: {
                                            app: "mysql",
                                            section: section,
                                            type: "data",
                                            id: id,
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
                                                    { containerPort: 3306, name: containerPortName, protocol: "TCP" },
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
                        }, { dependsOn: [secret, clusterIPService] });
                        statefulSets.push(statefulSet)
                    }
                    
                    // Create NodePort service for external access
                    const nodeportService = new k8s.core.v1.Service(`mysql-${section}-nodeport`, {
                        metadata: { name: `${serviceName}-nodeport`, namespace: namespace.metadata.name },
                        spec: {
                            type: "NodePort",
                            selector: { app: "mysql", section: section, type: "data" },
                            ports: nodePorts,
                        },
                    }, { dependsOn: [clusterIPService] });
                    
                    // Return all resources (Services and StatefulSets)
                    resources.push(clusterIPService)
                    resources.push(nodeportService)
                    resources.push(...statefulSets)
                }
                
                // Process log MySQL instances
                if (sectionConfs.log && Array.isArray(sectionConfs.log)) {
                    const logServiceName = `mysql-${section}-log`
                    const logClusterPorts = []
                    const logNodePorts = []
                    
                    // Collect all ports for log instances
                    sectionConfs.log.forEach((logConf, index) => {
                        const containerPortName = `m${globalIndex}`
                        logClusterPorts.push({
                            name: `mysql-${section}-log-${index}`,
                            port: logConf.port.cluster,
                            targetPort: containerPortName,
                        })
                        logNodePorts.push({
                            name: `mysql-${section}-log-${index}`,
                            port: logConf.port.cluster,
                            targetPort: containerPortName,
                            nodePort: logConf.port.node,
                        })
                        globalIndex++
                    })
                    
                    // Create ClusterIP service for log instances
                    const logClusterIPService = new k8s.core.v1.Service(`mysql-${section}-log`, {
                        metadata: { name: logServiceName, namespace: namespace.metadata.name },
                        spec: {
                            type: "ClusterIP",
                            selector: { app: "mysql", section: section, type: "log" },
                            ports: logClusterPorts,
                        },
                    });
                    
                    // Create StatefulSets for log instances
                    const logStatefulSets = []
                    globalIndex = globalIndex - sectionConfs.log.length
                    sectionConfs.log.forEach((logConf, index) => {
                        const containerPortName = `m${globalIndex}`
                        globalIndex++
                        const statefulSet = new k8s.apps.v1.StatefulSet(`mysql-${section}-log-${index}`, {
                            metadata: {
                                name: `mysql-${section}-log-${index}`,
                                namespace: namespace.metadata.name,
                            },
                            spec: {
                                serviceName: logServiceName,
                                replicas: 1,
                                selector: {
                                    matchLabels: {
                                        app: "mysql",
                                        section: section,
                                        type: "log",
                                        index: index.toString(),
                                    },
                                },
                                template: {
                                    metadata: {
                                        labels: {
                                            app: "mysql",
                                            section: section,
                                            type: "log",
                                            index: index.toString(),
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
                                                    { containerPort: 3306, name: containerPortName, protocol: "TCP" },
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
                                                        name: `data-volume-${section}-log-${index}`,
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
                                            name: `data-volume-${section}-log-${index}`,
                                            hostPath: {
                                                path: `/mnt/fb/mysql/${section}/log/${index}`,
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
                        }, { dependsOn: [secret, logClusterIPService] });
                        logStatefulSets.push(statefulSet)
                    })
                    
                    // Create NodePort service for log instances
                    const logNodeportService = new k8s.core.v1.Service(`mysql-${section}-log-nodeport`, {
                        metadata: { name: `${logServiceName}-nodeport`, namespace: namespace.metadata.name },
                        spec: {
                            type: "NodePort",
                            selector: { app: "mysql", section: section, type: "log" },
                            ports: logNodePorts,
                        },
                    }, { dependsOn: [logClusterIPService] });
                    
                    resources.push(logClusterIPService)
                    resources.push(logNodeportService)
                    resources.push(...logStatefulSets)
                }
            }

            return resources
        }
    }
}()