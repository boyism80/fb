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
                
                // Create PersistentVolumes for each replica
                for (let i = 0; i < replicas; i++) {
                    const pvName = `${resourceName}-data-${i}`
                    const pv = new k8s.core.v1.PersistentVolume(pvName, {
                        metadata: { 
                            name: pvName,
                            labels: {
                                app: "rabbitmq",
                                section: section,
                                type: type
                            }
                        },
                        spec: {
                            capacity: { storage: "10Gi" },
                            accessModes: ["ReadWriteOnce"],
                            persistentVolumeReclaimPolicy: "Retain",
                            storageClassName: "manual",
                            hostPath: {
                                path: `/mnt/fb/rabbitmq/${section}-${type}/data-${i}`,
                                type: "DirectoryOrCreate"
                            }
                        }
                    })
                    resources.push(pv)
                }
                
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
                                initContainers: [
                                    {
                                        name: "setup-data-dir",
                                        image: "busybox:latest",
                                        securityContext: {
                                            runAsUser: 0,
                                            runAsGroup: 0,
                                            privileged: false
                                        },
                                        command: [
                                            "sh", "-c",
                                            `#!/bin/sh
set -e

# Create directories and set proper permissions
mkdir -p /var/lib/rabbitmq/mnesia
mkdir -p /var/lib/rabbitmq/log

# Set ownership recursively (must be root to change ownership on hostPath)
echo "Setting ownership to 999:999..."
chown -R 999:999 /var/lib/rabbitmq || {
    echo "Warning: chown failed, attempting to fix permissions..."
    # If chown fails, try to at least make it writable
    chmod -R 777 /var/lib/rabbitmq
    exit 1
}

# Set directory permissions
find /var/lib/rabbitmq -type d -exec chmod 755 {} + || true

# Set file permissions (excluding .erlang.cookie which needs 400)
find /var/lib/rabbitmq -type f ! -name ".erlang.cookie" -exec chmod 644 {} + || true

# Ensure .erlang.cookie has correct permissions (owner read-only, 400)
if [ -f /var/lib/rabbitmq/.erlang.cookie ]; then
    chmod 400 /var/lib/rabbitmq/.erlang.cookie || true
    chown 999:999 /var/lib/rabbitmq/.erlang.cookie || true
fi

# Ensure mnesia directory is writable
chmod 755 /var/lib/rabbitmq/mnesia || true

# Verify ownership
OWNER=$(stat -c '%U:%G' /var/lib/rabbitmq 2>/dev/null || stat -f '%Su:%Sg' /var/lib/rabbitmq 2>/dev/null || echo "unknown")
echo "Data directory owner: $OWNER"
echo "Data directory setup complete"
`
                                        ],
                                        volumeMounts: [
                                            { name: "data", mountPath: "/var/lib/rabbitmq" }
                                        ]
                                    },
                                    {
                                        name: "reset-cluster-metadata",
                                        image: "busybox:latest",
                                        securityContext: {
                                            runAsUser: 0,
                                            runAsGroup: 0,
                                            privileged: false
                                        },
                                        command: [
                                            "sh", "-c",
                                            `#!/bin/sh
set -e

# Remove cluster metadata files while preserving queue data and messages
# This allows fresh cluster formation on each deployment

MNESIA_DIR="/var/lib/rabbitmq/mnesia"

# Ensure permissions are correct before cleanup (must be root to change ownership on hostPath)
echo "Ensuring correct permissions..."
chown -R 999:999 /var/lib/rabbitmq || {
    echo "Warning: chown failed, attempting to fix permissions..."
    chmod -R 777 /var/lib/rabbitmq
}
find /var/lib/rabbitmq -type d -exec chmod 755 {} + || true
# Set file permissions (excluding .erlang.cookie which needs 400)
find /var/lib/rabbitmq -type f ! -name ".erlang.cookie" -exec chmod 644 {} + || true
# Ensure .erlang.cookie has correct permissions (owner read-only, 400)
if [ -f /var/lib/rabbitmq/.erlang.cookie ]; then
    chmod 400 /var/lib/rabbitmq/.erlang.cookie || true
    chown 999:999 /var/lib/rabbitmq/.erlang.cookie || true
fi

if [ -d "$MNESIA_DIR" ]; then
    echo "Cleaning cluster metadata and corrupted files..."
    
    # Remove all coordination directories (they will be recreated)
    find "$MNESIA_DIR" -type d -name "coordination" -exec rm -rf {} + 2>/dev/null || true
    
    # Find all node directories
    NODE_DIRS=$(find $MNESIA_DIR -maxdepth 1 -type d -name "rabbit@*" 2>/dev/null || true)
    
    if [ -n "$NODE_DIRS" ]; then
        echo "$NODE_DIRS" | while read NODE_DIR; do
            if [ -n "$NODE_DIR" ] && [ -d "$NODE_DIR" ]; then
                echo "Cleaning node directory: $NODE_DIR"
                
                # Remove cluster-related files (NOT queue data)
                rm -f "$NODE_DIR/LATEST.LOG" 2>/dev/null || true
                rm -f "$NODE_DIR/LATEST.LOG.previous" 2>/dev/null || true
                rm -f "$NODE_DIR/cluster_nodes.config" 2>/dev/null || true
                rm -rf "$NODE_DIR/coordination" 2>/dev/null || true
                
                # Remove schema files to force re-initialization
                rm -f "$NODE_DIR/schema.DAT" 2>/dev/null || true
                rm -f "$NODE_DIR/schema_version" 2>/dev/null || true
                
                # Remove crash dump files that might prevent startup
                rm -f "$NODE_DIR/erl_crash.dump" 2>/dev/null || true
                find "$NODE_DIR" -name "*.dets" -type f -delete 2>/dev/null || true
                
                # Remove lock files
                find "$NODE_DIR" -name "*.lock" -type f -delete 2>/dev/null || true
                
                # Remove any corrupted dets files in subdirectories
                find "$NODE_DIR" -name "meta.dets" -type f -delete 2>/dev/null || true
                find "$NODE_DIR" -name "*.dets.*" -type f -delete 2>/dev/null || true
            fi
        done
        
        echo "Cluster metadata cleaned"
    else
        echo "No existing node directory found - fresh start"
    fi
else
    echo "No mnesia directory - fresh start"
fi

# Final permission check
chown -R 999:999 /var/lib/rabbitmq || true
chmod 755 /var/lib/rabbitmq/mnesia || true
# Ensure .erlang.cookie has correct permissions (owner read-only, 400)
if [ -f /var/lib/rabbitmq/.erlang.cookie ]; then
    chmod 400 /var/lib/rabbitmq/.erlang.cookie || true
    chown 999:999 /var/lib/rabbitmq/.erlang.cookie || true
fi

echo "Reset complete"
`
                                        ],
                                        volumeMounts: [
                                            { name: "data", mountPath: "/var/lib/rabbitmq" }
                                        ]
                                    }
                                ],
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
                                                { name: "config", mountPath: "/etc/rabbitmq" },
                                                { name: "data", mountPath: "/var/lib/rabbitmq" },
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
                                                configMap: { name: configMap.metadata.name },
                                            }
                                        ],
                                    },
                                },
                        volumeClaimTemplates: [{
                            metadata: { name: "data" },
                            spec: {
                                accessModes: ["ReadWriteOnce"],
                                storageClassName: "manual",
                                resources: { requests: { storage: "10Gi" } }
                            }
                        }]
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