const pulumi = require("@pulumi/pulumi")
const k8s = require("@pulumi/kubernetes")


module.exports = {
    setup: function (namespace, conf) {

        const resources = []
        let globalIndex = 0
        for(const [section, sectionConf] of Object.entries(conf.redis)) {
            // Process data Redis instances
            if (sectionConf.data) {
                const serviceName = `redis-${section}`
                const clusterPorts = []
                const nodePorts = []
                
                // Collect all ports first with unique containerPort names using global index
                for(const [id, redisConf] of Object.entries(sectionConf.data)) {
                    const containerPortName = `r${globalIndex}`
                    clusterPorts.push({
                        name: `redis-${section}-${id}`,
                        port: redisConf.port.cluster,
                        targetPort: containerPortName,
                    })
                    nodePorts.push({
                        name: `redis-${section}-${id}`,
                        port: redisConf.port.cluster,
                        targetPort: containerPortName,
                        nodePort: redisConf.port.node,
                    })
                    globalIndex++
                }
                
                // Create ClusterIP service for internal access with all ports
                const clusterIPService = new k8s.core.v1.Service(`redis-${section}`, {
                    metadata: { name: serviceName, namespace: namespace.metadata.name },
                    spec: {
                        type: "ClusterIP",
                        selector: { app: "redis", section: section, type: "data" },
                        ports: clusterPorts,
                    },
                });

                // Create StatefulSets
                const statefulSets = []
                globalIndex = globalIndex - Object.keys(sectionConf.data).length
                for(const [id, redisConf] of Object.entries(sectionConf.data)) {
                    const containerPortName = `r${globalIndex}`
                    globalIndex++
                    const statefulSet = new k8s.apps.v1.StatefulSet(`redis-${section}-${id}`, {
                        metadata: {
                            name: `redis-${section}-${id}`,
                            namespace: namespace.metadata.name,
                        },
                        spec: {
                            serviceName: serviceName,
                            replicas: 1,
                            selector: {
                                matchLabels: {
                                    app: "redis",
                                    section: section,
                                    type: "data",
                                },
                            },
                            template: {
                                metadata: {
                                    labels: {
                                        app: "redis",
                                        section: section,
                                        type: "data",
                                    },
                                },
                                spec: {
                                    nodeSelector: {
                                        "kubernetes.io/hostname": "ubuntu-1"
                                    },
                                    containers: [
                                        {
                                            name: "redis",
                                            image: "redis:6.2.4",
                                            ports: [
                                                {
                                                    containerPort: 6379,
                                                    name: containerPortName,
                                                    protocol: "TCP",
                                                },
                                            ],
                                            volumeMounts: [
                                                {
                                                    name: `data-volume-${section}-${id}`,
                                                    mountPath: "/data",
                                                },
                                            ],
                                        },
                                    ],
                                    volumes: [
                                    {
                                        name: `data-volume-${section}-${id}`,
                                        hostPath: {
                                            path: `/mnt/fb/redis/${section}/${id}`,
                                            type: "DirectoryOrCreate"
                                        }
                                    }]
                                },
                            }
                        }
                    }, { dependsOn: [clusterIPService] })
                    statefulSets.push(statefulSet)
                }
                
                // Create NodePort service for external access
                const nodeportService = new k8s.core.v1.Service(`redis-${section}-nodeport`, {
                    metadata: { name: `${serviceName}-nodeport`, namespace: namespace.metadata.name },
                    spec: {
                        type: "NodePort",
                        selector: { app: "redis", section: section, type: "data" },
                        ports: nodePorts,
                    },
                }, { dependsOn: [clusterIPService] });
                
                // Return all resources (Services and StatefulSets)
                resources.push(clusterIPService)
                resources.push(nodeportService)
                resources.push(...statefulSets)
            }
            
            // Process log Redis instances
            if (sectionConf.log && Array.isArray(sectionConf.log)) {
                const logServiceName = `redis-${section}-log`
                const logClusterPorts = []
                const logNodePorts = []
                
                // Collect all ports for log instances
                sectionConf.log.forEach((logConf, index) => {
                    const containerPortName = `r${globalIndex}`
                    logClusterPorts.push({
                        name: `redis-${section}-log-${index}`,
                        port: logConf.port.cluster,
                        targetPort: containerPortName,
                    })
                    logNodePorts.push({
                        name: `redis-${section}-log-${index}`,
                        port: logConf.port.cluster,
                        targetPort: containerPortName,
                        nodePort: logConf.port.node,
                    })
                    globalIndex++
                })
                
                // Create ClusterIP service for log instances
                const logClusterIPService = new k8s.core.v1.Service(`redis-${section}-log`, {
                    metadata: { name: logServiceName, namespace: namespace.metadata.name },
                    spec: {
                        type: "ClusterIP",
                        selector: { app: "redis", section: section, type: "log" },
                        ports: logClusterPorts,
                    },
                });

                // Create StatefulSets for log instances
                const logStatefulSets = []
                globalIndex = globalIndex - sectionConf.log.length
                sectionConf.log.forEach((logConf, index) => {
                    const containerPortName = `r${globalIndex}`
                    globalIndex++
                    const statefulSet = new k8s.apps.v1.StatefulSet(`redis-${section}-log-${index}`, {
                        metadata: {
                            name: `redis-${section}-log-${index}`,
                            namespace: namespace.metadata.name,
                        },
                        spec: {
                            serviceName: logServiceName,
                            replicas: 1,
                            selector: {
                                matchLabels: {
                                    app: "redis",
                                    section: section,
                                    type: "log",
                                    index: index.toString(),
                                },
                            },
                            template: {
                                metadata: {
                                    labels: {
                                        app: "redis",
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
                                            name: "redis",
                                            image: "redis:6.2.4",
                                            ports: [
                                                {
                                                    containerPort: 6379,
                                                    name: containerPortName,
                                                    protocol: "TCP",
                                                },
                                            ],
                                            volumeMounts: [
                                                {
                                                    name: `data-volume-${section}-log-${index}`,
                                                    mountPath: "/data",
                                                },
                                            ],
                                        },
                                    ],
                                    volumes: [
                                    {
                                        name: `data-volume-${section}-log-${index}`,
                                        hostPath: {
                                            path: `/mnt/fb/redis/${section}/log/${index}`,
                                            type: "DirectoryOrCreate"
                                        }
                                    }]
                                },
                            }
                        }
                    }, { dependsOn: [logClusterIPService] })
                    logStatefulSets.push(statefulSet)
                })
                
                // Create NodePort service for log instances
                const logNodeportService = new k8s.core.v1.Service(`redis-${section}-log-nodeport`, {
                    metadata: { name: `${logServiceName}-nodeport`, namespace: namespace.metadata.name },
                    spec: {
                        type: "NodePort",
                        selector: { app: "redis", section: section, type: "log" },
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
