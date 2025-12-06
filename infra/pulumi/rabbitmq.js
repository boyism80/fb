const pulumi = require("@pulumi/pulumi");
const k8s = require("@pulumi/kubernetes");

module.exports = {
    setup: function (namespace, conf) {
        const resources = []
        
        // Create ServiceAccount for RabbitMQ
        const serviceAccount = new k8s.core.v1.ServiceAccount("rabbitmq-sa", {
            metadata: { 
                name: "rabbitmq", 
                namespace: namespace.metadata.name 
            }
        });
        
        // Create Role for K8s API access (required for peer discovery)
        const role = new k8s.rbac.v1.Role("rabbitmq-role", {
            metadata: { 
                name: "rabbitmq", 
                namespace: namespace.metadata.name 
            },
            rules: [
                {
                    apiGroups: [""],
                    resources: ["endpoints"],
                    verbs: ["get", "list", "watch"]
                },
                {
                    apiGroups: [""],
                    resources: ["events"],
                    verbs: ["create"]
                }
            ]
        });
        
        // Create RoleBinding
        const roleBinding = new k8s.rbac.v1.RoleBinding("rabbitmq-rolebinding", {
            metadata: { 
                name: "rabbitmq", 
                namespace: namespace.metadata.name 
            },
            subjects: [{
                kind: "ServiceAccount",
                name: serviceAccount.metadata.name,
                namespace: namespace.metadata.name
            }],
            roleRef: {
                kind: "Role",
                name: role.metadata.name,
                apiGroup: "rbac.authorization.k8s.io"
            }
        });
        
        resources.push(serviceAccount, role, roleBinding);
        
        const sections = Object.entries(conf.rabbitmq)
        
        for(const [section, sectionConf] of sections) {
            for(const [type, typeConf] of Object.entries(sectionConf)) {
                if (type !== "internal" && type !== "log") continue
                
                const resourceName = `rabbitmq-${section}-${type}`
                const headlessServiceName = `${resourceName}-headless`
                const replicas = typeConf.replicas || 1
                
                // Create ConfigMap for RabbitMQ configuration
                const configMap = new k8s.core.v1.ConfigMap(`${resourceName}-config`, {
                    metadata: { name: `${resourceName}-config`, namespace: namespace.metadata.name },
                    data: {
                        "enabled_plugins": "[rabbitmq_management,rabbitmq_peer_discovery_k8s,rabbitmq_sharding].",
                        "rabbitmq.conf": `cluster_formation.peer_discovery_backend = rabbit_peer_discovery_k8s
cluster_formation.k8s.host = kubernetes.default.svc.cluster.local
cluster_formation.k8s.address_type = hostname
cluster_formation.k8s.service_name = ${headlessServiceName}
cluster_formation.node_cleanup.interval = 10
cluster_formation.node_cleanup.only_log_warning = true
cluster_partition_handling = autoheal
`
                    },
                })
                
                // Create headless service for StatefulSet
                const headlessService = new k8s.core.v1.Service(headlessServiceName, {
                    metadata: { 
                        name: headlessServiceName, 
                        namespace: namespace.metadata.name 
                    },
                    spec: {
                        clusterIP: "None",
                        selector: { app: "rabbitmq", section: section, type: type },
                        ports: [
                            { name: "amqp", port: 5672, targetPort: 5672 },
                            { name: "management", port: 15672, targetPort: 15672 },
                            { name: "epmd", port: 4369, targetPort: 4369 },
                        ],
                    },
                }, { dependsOn: [configMap] });
                
                // Create StatefulSet with volumeClaimTemplates for persistent storage
                const statefulSet = new k8s.apps.v1.StatefulSet(resourceName, {
                    metadata: { name: resourceName, namespace: namespace.metadata.name },
                    spec: {
                        serviceName: headlessServiceName,
                        replicas: replicas,
                        selector: { matchLabels: { app: "rabbitmq", section: section, type: type } },
                        podManagementPolicy: "OrderedReady",
                        template: {
                            metadata: { labels: { app: "rabbitmq", section: section, type: type } },
                            spec: {
                                serviceAccountName: serviceAccount.metadata.name,
                                nodeSelector: {
                                    "kubernetes.io/hostname": "ubuntu-1"
                                },
                                containers: [{
                                    name: "rabbitmq",
                                    image: "rabbitmq:4.0.2-management",
                                    ports: [
                                        { name: "amqp", containerPort: 5672 },
                                        { name: "management", containerPort: 15672 },
                                        { name: "epmd", containerPort: 4369 },
                                    ],
                                    env: [
                                        { name: "RABBITMQ_DEFAULT_USER", value: "fb" },
                                        { name: "RABBITMQ_DEFAULT_PASS", value: "admin" },
                                        { name: "RABBITMQ_ERLANG_COOKIE", value: "secret-cookie" },
                                        { name: "RABBITMQ_USE_LONGNAME", value: "true" },
                                        { name: "K8S_SERVICE_NAME", value: headlessServiceName },
                                        { name: "K8S_HOSTNAME_SUFFIX", value: pulumi.interpolate`.${headlessServiceName}.${namespace.metadata.name}.svc.cluster.local` },
                                        { name: "MY_POD_NAME", valueFrom: { fieldRef: { fieldPath: "metadata.name" } } },
                                        { name: "MY_POD_NAMESPACE", valueFrom: { fieldRef: { fieldPath: "metadata.namespace" } } },
                                    ],
                                    command: ["/bin/bash", "-c"],
                                    args: [pulumi.interpolate`
export RABBITMQ_NODENAME=rabbit@$MY_POD_NAME.${headlessServiceName}.$MY_POD_NAMESPACE.svc.cluster.local
exec docker-entrypoint.sh rabbitmq-server
`],
                                        volumeMounts: [
                                            { name: "config", mountPath: "/etc/rabbitmq" }
                                        ],
                                        readinessProbe: {
                                            exec: { command: ["rabbitmq-diagnostics", "ping"] },
                                            initialDelaySeconds: 20,
                                            periodSeconds: 10,
                                            timeoutSeconds: 5,
                                        },
                                        livenessProbe: {
                                            exec: { command: ["rabbitmq-diagnostics", "ping"] },
                                            initialDelaySeconds: 60,
                                            periodSeconds: 30,
                                            timeoutSeconds: 10,
                                        },
                                    }],
                                    volumes: [
                                        {
                                            name: "config",
                                            configMap: { name: configMap.metadata.name }
                                        }
                                    ]
                                }
                            }
                        },
                    }, { dependsOn: [headlessService, configMap, serviceAccount, roleBinding] })
                    
                    // Create ClusterIP service
                    const clusterIPService = new k8s.core.v1.Service(resourceName, {
                        metadata: { name: resourceName, namespace: namespace.metadata.name },
                        spec: {
                            type: "ClusterIP",
                            selector: { app: "rabbitmq", section: section, type: type },
                            ports: [
                                { name: "amqp", port: typeConf.port.amqp.cluster, targetPort: 5672 },
                                { name: "management", port: typeConf.port.management.cluster, targetPort: 15672 },
                            ],
                        },
                    }, { dependsOn: [statefulSet] });
                    
                    // Create NodePort service
                    const nodeportService = new k8s.core.v1.Service(`${resourceName}-nodeport`, {
                        metadata: { name: `${resourceName}-nodeport`, namespace: namespace.metadata.name },
                        spec: {
                            type: "NodePort",
                            selector: { app: "rabbitmq", section: section, type: type },
                            ports: [
                                {
                                    name: "amqp",
                                    port: typeConf.port.amqp.cluster,
                                    targetPort: 5672,
                                    nodePort: typeConf.port.amqp.node,
                                },
                                {
                                    name: "management",
                                    port: typeConf.port.management.cluster,
                                    targetPort: 15672,
                                    nodePort: typeConf.port.management.node,
                                },
                            ],
                        },
                    }, { dependsOn: [statefulSet] });
                    
                    resources.push(configMap, headlessService, statefulSet, clusterIPService, nodeportService)
                }
            }
            
            return resources
        }
    }