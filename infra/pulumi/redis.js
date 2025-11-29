const pulumi = require("@pulumi/pulumi")
const k8s = require("@pulumi/kubernetes")


module.exports = {
    setup: function (namespace, conf) {

        const services = []
        let globalIndex = 0
        for(const [section, sectionConf] of Object.entries(conf.redis)) {
            const serviceName = `redis-${section}`
            const clusterPorts = []
            const nodePorts = []
            
            // Collect all ports first with unique containerPort names using global index
            for(const [id, redisConf] of Object.entries(sectionConf)) {
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
                    selector: { app: "redis", section: section },
                    ports: clusterPorts,
                },
            });

            globalIndex = globalIndex - Object.keys(sectionConf).length
            for(const [id, redisConf] of Object.entries(sectionConf)) {
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
            
            services.push(clusterIPService)
            services.push(nodeportService)
        }

        return services
    }
}
