const pulumi = require("@pulumi/pulumi")
const k8s = require("@pulumi/kubernetes")


module.exports = {
    setup: function (namespace, conf) {
        
        const resources = []
        let globalIndex = 0
        
        // Process worlds' Redis instances
        for(const [worldName, worldConf] of Object.entries(conf.worlds)) {
            if (!worldConf.redis)
                continue
            
            const sectionConf = worldConf.redis
            const section = worldName
            const serviceName = `redis-${section}`
            const clusterPorts = []
            const nodePorts = []
            
            // Process global Redis instance
            if (sectionConf.global) {
                const globalConf = sectionConf.global
                const containerPortName = `r${globalIndex}`
                clusterPorts.push({
                    name: `redis-${section}-global`,
                    port: globalConf.port.cluster,
                    targetPort: containerPortName,
                })
                nodePorts.push({
                    name: `redis-${section}-global`,
                    port: globalConf.port.cluster,
                    targetPort: containerPortName,
                    nodePort: globalConf.port.node,
                })
                globalIndex++
            }
            
            // Process data Redis instances (array)
            if (sectionConf.data && Array.isArray(sectionConf.data)) {
                sectionConf.data.forEach((redisConf, index) => {
                    const containerPortName = `r${globalIndex}`
                    clusterPorts.push({
                        name: `redis-${section}-${index}`,
                        port: redisConf.port.cluster,
                        targetPort: containerPortName,
                    })
                    nodePorts.push({
                        name: `redis-${section}-${index}`,
                        port: redisConf.port.cluster,
                        targetPort: containerPortName,
                        nodePort: redisConf.port.node,
                    })
                    globalIndex++
                })
            }
            
            if (clusterPorts.length > 0) {
                // Create ClusterIP service for internal access with all ports
                const clusterIPService = new k8s.core.v1.Service(`redis-${section}`, {
                    metadata: { name: serviceName, namespace: namespace.metadata.name },
                    spec: {
                        type: "ClusterIP",
                        selector: { app: "redis", section: section },
                        ports: clusterPorts,
                    },
                });
                
                // Create StatefulSets
                const statefulSets = []
                const startIndex = globalIndex - clusterPorts.length
                globalIndex = startIndex
                
                // Create StatefulSet for global
                if (sectionConf.global) {
                    const globalConf = sectionConf.global
                    const containerPortName = `r${globalIndex}`
                    globalIndex++
                    const statefulSet = new k8s.apps.v1.StatefulSet(`redis-${section}-global`, {
                    metadata: {
                        name: `redis-${section}-global`,
                        namespace: namespace.metadata.name,
                    },
                    spec: {
                        serviceName: serviceName,
                        replicas: 1,
                        selector: {
                            matchLabels: {
                                app: "redis",
                                section: section,
                            },
                        },
                        template: {
                            metadata: {
                                labels: {
                                    app: "redis",
                                    section: section,
                                },
                            },
                            spec: {
                                affinity: {
                                    nodeAffinity: {
                                        requiredDuringSchedulingIgnoredDuringExecution: {
                                            nodeSelectorTerms: [{
                                                matchExpressions: [{
                                                    key: "infra",
                                                    operator: "In",
                                                    values: ["true"]
                                                }]
                                            }]
                                        }
                                    }
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
                                                name: `data-volume-${section}-global`,
                                                mountPath: "/data",
                                            },
                                        ],
                                    },
                                ],
                                volumes: [
                                    {
                                        name: `data-volume-${section}-global`,
                                        hostPath: {
                                            path: `/mnt/fb/redis/${section}/global`,
                                            type: "DirectoryOrCreate"
                                        }
                                    }]
                            },
                        }
                    }
                }, { dependsOn: [clusterIPService] })
                    statefulSets.push(statefulSet)
                }
                
                // Create StatefulSets for data shards
                if (sectionConf.data && Array.isArray(sectionConf.data)) {
                    sectionConf.data.forEach((redisConf, index) => {
                        const containerPortName = `r${globalIndex}`
                        globalIndex++
                        const statefulSet = new k8s.apps.v1.StatefulSet(`redis-${section}-${index}`, {
                            metadata: {
                                name: `redis-${section}-${index}`,
                                namespace: namespace.metadata.name,
                            },
                            spec: {
                                serviceName: serviceName,
                                replicas: 1,
                                selector: {
                                    matchLabels: {
                                        app: "redis",
                                        section: section,
                                    },
                                },
                                template: {
                                    metadata: {
                                        labels: {
                                            app: "redis",
                                            section: section,
                                        },
                                    },
                                    spec: {
                                        affinity: {
                                            nodeAffinity: {
                                                requiredDuringSchedulingIgnoredDuringExecution: {
                                                    nodeSelectorTerms: [{
                                                        matchExpressions: [{
                                                            key: "infra",
                                                            operator: "In",
                                                            values: ["true"]
                                                        }]
                                                    }]
                                                }
                                            }
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
                                                        name: `data-volume-${section}-${index}`,
                                                        mountPath: "/data",
                                                    },
                                                ],
                                            },
                                        ],
                                        volumes: [
                                            {
                                                name: `data-volume-${section}-${index}`,
                                                hostPath: {
                                                    path: `/mnt/fb/redis/${section}/${index}`,
                                                    type: "DirectoryOrCreate"
                                                }
                                            }]
                                        },
                                    }
                                }
                            }, { dependsOn: [clusterIPService] })
                        statefulSets.push(statefulSet)
                    })
                }
                
                // Create NodePort service for external access
                const nodeportService = new k8s.core.v1.Service(`redis-${section}-nodeport`, {
                    metadata: { name: `${serviceName}-nodeport`, namespace: namespace.metadata.name },
                    spec: {
                        type: "NodePort",
                        selector: { app: "redis", section: section },
                        ports: nodePorts,
                    },
                }, { dependsOn: [clusterIPService] });
                
                // Return all resources (Services and StatefulSets)
                resources.push(clusterIPService)
                resources.push(nodeportService)
                resources.push(...statefulSets)
            }
        }
        
        // Process unified-infra Redis instances (unified-infra has no global key, direct access)
        if (conf["unified-infra"] && conf["unified-infra"].redis && conf["unified-infra"].redis.port) {
            const unifiedRedis = conf["unified-infra"].redis
            const section = "unified"
            const serviceName = `redis-${section}`
            const containerPortName = `r${globalIndex}`
            
            // Create ClusterIP service for unified Redis
            const clusterIPService = new k8s.core.v1.Service(`redis-${section}`, {
                metadata: { name: serviceName, namespace: namespace.metadata.name },
                spec: {
                    type: "ClusterIP",
                    selector: { app: "redis", section: section },
                    ports: [{
                        name: `redis-${section}`,
                        port: unifiedRedis.port.cluster,
                        targetPort: containerPortName,
                    }],
                },
            });
            
            // Create StatefulSet for unified Redis
            const statefulSet = new k8s.apps.v1.StatefulSet(`redis-${section}`, {
                metadata: {
                    name: `redis-${section}`,
                    namespace: namespace.metadata.name,
                },
                spec: {
                    serviceName: serviceName,
                    replicas: 1,
                    selector: {
                        matchLabels: {
                            app: "redis",
                            section: section,
                        },
                    },
                    template: {
                        metadata: {
                            labels: {
                                app: "redis",
                                section: section,
                            },
                        },
                        spec: {
                            affinity: {
                                nodeAffinity: {
                                    requiredDuringSchedulingIgnoredDuringExecution: {
                                        nodeSelectorTerms: [{
                                            matchExpressions: [{
                                                key: "infra",
                                                operator: "In",
                                                values: ["true"]
                                            }]
                                        }]
                                    }
                                }
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
                                            name: `data-volume-${section}`,
                                            mountPath: "/data",
                                        },
                                    ],
                                },
                            ],
                            volumes: [
                                {
                                    name: `data-volume-${section}`,
                                    hostPath: {
                                        path: `/mnt/fb/redis/${section}`,
                                        type: "DirectoryOrCreate"
                                    }
                                }]
                        },
                    }
                },
            }, { dependsOn: [clusterIPService] })
            
            // Create NodePort service for external access
            const nodeportService = new k8s.core.v1.Service(`redis-${section}-nodeport`, {
                metadata: { name: `${serviceName}-nodeport`, namespace: namespace.metadata.name },
                spec: {
                    type: "NodePort",
                    selector: { app: "redis", section: section },
                    ports: [{
                        name: `redis-${section}`,
                        port: unifiedRedis.port.cluster,
                        targetPort: containerPortName,
                        nodePort: unifiedRedis.port.node,
                    }],
                },
            }, { dependsOn: [clusterIPService] });
            
            // Return all resources (Services and StatefulSets)
            resources.push(clusterIPService)
            resources.push(nodeportService)
            resources.push(statefulSet)
            globalIndex++
        }
        
        return resources
    }
}