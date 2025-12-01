const pulumi = require("@pulumi/pulumi");
const k8s = require("@pulumi/kubernetes");

module.exports = {
    setup: function (namespace, conf) {
        // Entrypoint script that enables sharding plugin and sets up clustering
        // First pod (ordinal 0) starts standalone, others join the first pod
        const entrypointScript = `#!/bin/bash
set -e

# Get pod name and namespace from environment variables
RABBITMQ_POD_NAME="$RABBITMQ_POD_NAME"
RABBITMQ_POD_NAMESPACE="$RABBITMQ_POD_NAMESPACE"
FIRST_POD_NAME="$1"
K8S_SERVICE_NAME="$K8S_SERVICE_NAME"

# Construct current pod's node name
# Use K8S_SERVICE_NAME if available, otherwise construct from pod name
if [ -n "$K8S_SERVICE_NAME" ]; then
    POD_NAME="rabbit@\${RABBITMQ_POD_NAME}.\${K8S_SERVICE_NAME}.\${RABBITMQ_POD_NAMESPACE}.svc.cluster.local"
else
    # Fallback: extract section from pod name (e.g., rabbitmq-section-1-0 -> rabbitmq-section-1-headless)
    SECTION_NAME=$(echo "$RABBITMQ_POD_NAME" | sed 's/-[0-9]*$//')
    POD_NAME="rabbit@\${RABBITMQ_POD_NAME}.\${SECTION_NAME}-headless.\${RABBITMQ_POD_NAMESPACE}.svc.cluster.local"
fi

# Set RABBITMQ_NODENAME environment variable
export RABBITMQ_NODENAME="$POD_NAME"
echo "RABBITMQ_NODENAME set to: $RABBITMQ_NODENAME"

# Always clean Mnesia data directory to start fresh
MNESIA_DIR="/var/lib/rabbitmq/mnesia"
if [ -d "$MNESIA_DIR" ]; then
    echo "Cleaning Mnesia data directory for fresh start..."
    rm -rf "$MNESIA_DIR"/*
    rm -rf "$MNESIA_DIR"/.* 2>/dev/null || true
    echo "Mnesia data cleaned. RabbitMQ will start with fresh data."
else
    echo "Mnesia directory does not exist. Will be created on first start."
fi

# Check if this is the first pod (ordinal 0)
if [[ "$RABBITMQ_POD_NAME" == *"-0" ]]; then
    IS_FIRST_POD="true"
else
    IS_FIRST_POD="false"
fi

# Function to wait for RabbitMQ to be ready
wait_for_rabbitmq() {
    local max_attempts=60
    local attempt=0
    while [ $attempt -lt $max_attempts ]; do
        if [ -n "$RABBITMQ_NODENAME" ]; then
            if rabbitmqctl -n "$RABBITMQ_NODENAME" status > /dev/null 2>&1; then
                echo "RabbitMQ is ready"
                return 0
            fi
        else
            if rabbitmqctl status > /dev/null 2>&1; then
                echo "RabbitMQ is ready"
                return 0
            fi
        fi
        attempt=$((attempt + 1))
        if [ $((attempt % 10)) -eq 0 ]; then
            echo "Still waiting for RabbitMQ... (attempt $attempt/$max_attempts)"
        fi
        sleep 1
    done
    echo "Timeout waiting for RabbitMQ to start"
    return 1
}

# Function to wait for another node to be ready
wait_for_node() {
    local node_name=$1
    local max_attempts=30
    local attempt=0
    while [ $attempt -lt $max_attempts ]; do
        if rabbitmqctl -n "$node_name" status > /dev/null 2>&1; then
            return 0
        fi
        attempt=$((attempt + 1))
        sleep 2
    done
    return 1
}

# Check if plugin is already enabled (persistent across restarts)
if rabbitmq-plugins list 2>/dev/null | grep -q "\[E\] rabbitmq_sharding"; then
    PLUGIN_ENABLED=true
else
    PLUGIN_ENABLED=false
fi

# Start RabbitMQ in background using default entrypoint
echo "Starting RabbitMQ in background..."
echo "RABBITMQ_NODENAME is set to: $RABBITMQ_NODENAME"
/usr/local/bin/docker-entrypoint.sh rabbitmq-server -detached

# Wait a bit for RabbitMQ process to start
sleep 3

# Wait for RabbitMQ to be ready
echo "Waiting for RabbitMQ to start..."
if ! wait_for_rabbitmq; then
    echo "Failed to start RabbitMQ"
    echo "Checking RabbitMQ process..."
    ps aux | grep rabbitmq || true
    echo "Checking RabbitMQ logs..."
    tail -n 50 /var/log/rabbitmq/*.log 2>/dev/null || echo "No log files found"
    exit 1
fi

# Enable sharding plugin if not already enabled
if [ "$PLUGIN_ENABLED" = false ]; then
    echo "Enabling rabbitmq_sharding plugin..."
    rabbitmq-plugins enable rabbitmq_sharding
fi

# Cluster setup: first pod (ordinal 0) is standalone, others join the first pod
if [ "$IS_FIRST_POD" != "true" ] && [ -n "$FIRST_POD_NAME" ]; then
    echo "Joining RabbitMQ cluster with node: $FIRST_POD_NAME"
    # Stop the app to join cluster
    rabbitmqctl stop_app
    
    # Wait for the first pod to be ready
    if wait_for_node "$FIRST_POD_NAME"; then
        if rabbitmqctl join_cluster "$FIRST_POD_NAME"; then
            echo "Successfully joined cluster with node: $FIRST_POD_NAME"
        else
            echo "Warning: Failed to join cluster, starting as standalone node"
        fi
    else
        echo "Warning: Cluster node not ready, starting as standalone node"
    fi
    
    # Start the app
    rabbitmqctl start_app
fi

# Note: Queue Mirroring (HA policy) is optional
# Without mirroring: Queues exist on one node, but accessible from any node in cluster
# With mirroring: Queues are replicated to all nodes for high availability (uses more memory)
# Uncomment below to enable queue mirroring for high availability:
# if [ "$IS_FIRST_POD" = "true" ]; then
#     echo "Setting up HA policy for queue mirroring..."
#     sleep 3
#     rabbitmqctl set_policy ha-all "^" '{"ha-mode":"all","ha-sync-mode":"automatic"}' --priority 0 --apply-to all || {
#         echo "Warning: Failed to set HA policy (may already be set)"
#     }
#     echo "HA policy set: all queues will be mirrored to all nodes"
# fi

# Stop background RabbitMQ
echo "Stopping RabbitMQ to restart with all configurations..."
rabbitmqctl stop

# Wait a bit for clean shutdown
sleep 2

# Start RabbitMQ in foreground
echo "Starting RabbitMQ with all configurations applied..."
exec /usr/local/bin/docker-entrypoint.sh rabbitmq-server
`

        // Create ConfigMap for entrypoint script
        const entrypointConfigMap = new k8s.core.v1.ConfigMap("rabbitmq-entrypoint", {
            metadata: { name: "rabbitmq-entrypoint", namespace: namespace.metadata.name },
            data: {
                "entrypoint.sh": entrypointScript,
            },
        })

        const resources = []
        const sections = Object.entries(conf.rabbitmq)
        
        // Create all headless services first (required for StatefulSet serviceName)
        const headlessServices = {}
        const statefulSets = {}
        
        for(const [section, sectionConf] of sections) {
            // Process internal and log separately
            for(const [type, typeConf] of Object.entries(sectionConf)) {
                if (type !== "internal" && type !== "log") continue
                
                const resourceName = `rabbitmq-${section}-${type}`
                const headlessServiceName = `${resourceName}-headless`
                const replicas = typeConf.replicas || 1
                const serviceName = resourceName
                const firstPodName = pulumi.interpolate`rabbit@${resourceName}-0.${headlessServiceName}.${namespace.metadata.name}.svc.cluster.local`
                
                // Create headless service
                const headlessService = new k8s.core.v1.Service(headlessServiceName, {
                    metadata: { name: headlessServiceName, namespace: namespace.metadata.name },
                    spec: {
                        clusterIP: "None",
                        selector: { app: "rabbitmq", section: section, type: type },
                        ports: [
                            { name: "amqp", port: typeConf.port.amqp.cluster, targetPort: 5672 },
                            { name: "management", port: typeConf.port.management.cluster, targetPort: 15672 },
                        ],
                    },
                }, { dependsOn: [entrypointConfigMap] });
                headlessServices[resourceName] = headlessService
                
                // Create StatefulSet
                const statefulSet = new k8s.apps.v1.StatefulSet(resourceName, {
                    metadata: { name: resourceName, namespace: namespace.metadata.name, },
                    spec: {
                        serviceName: headlessServiceName,
                        replicas: replicas,
                        selector: { matchLabels: { app: "rabbitmq", section: section, type: type } },
                        template: {
                            metadata: { labels: { app: "rabbitmq", section: section, type: type } },
                            spec: {
                                subdomain: headlessServiceName,
                                containers: [{
                                    name: "rabbitmq",
                                    image: "rabbitmq:management",
                                    ports: [
                                        { name: `amqp`, containerPort: 5672 },
                                        { name: `management`, containerPort: 15672 },
                                    ],
                                    env: [
                                        { name: "RABBITMQ_DEFAULT_USER", value: "fb" },
                                        { name: "RABBITMQ_DEFAULT_PASS", value: "admin" },
                                        { name: "RABBITMQ_ERLANG_COOKIE", value: "secret-cookie" },
                                        { name: "K8S_SERVICE_NAME", value: headlessServiceName },
                                        { name: "RABBITMQ_USE_LONGNAME", value: "true" },
                                        { 
                                            name: "RABBITMQ_POD_NAME",
                                            valueFrom: {
                                                fieldRef: {
                                                    fieldPath: "metadata.name"
                                                }
                                            }
                                        },
                                        { 
                                            name: "RABBITMQ_POD_NAMESPACE",
                                            valueFrom: {
                                                fieldRef: {
                                                    fieldPath: "metadata.namespace"
                                                }
                                            }
                                        },
                                    ],
                                    command: ["/bin/bash"],
                                    args: firstPodName.apply(name => ["/scripts/entrypoint.sh", name]),
                                    readinessProbe: {
                                        exec: {
                                            command: ["/bin/sh", "-c", "rabbitmqctl status > /dev/null 2>&1"]
                                        },
                                        initialDelaySeconds: 15,
                                        periodSeconds: 5,
                                        timeoutSeconds: 5,
                                        successThreshold: 1,
                                        failureThreshold: 3
                                    },
                                    livenessProbe: {
                                        exec: {
                                            command: ["/bin/sh", "-c", "rabbitmqctl status > /dev/null 2>&1"]
                                        },
                                        initialDelaySeconds: 60,
                                        periodSeconds: 30,
                                        timeoutSeconds: 10,
                                        successThreshold: 1,
                                        failureThreshold: 3
                                    },
                                    volumeMounts: [
                                        { 
                                            name: "data-volume", 
                                            mountPath: "/var/lib/rabbitmq",
                                            subPathExpr: "$(RABBITMQ_POD_NAME)"
                                        },
                                        {
                                            name: "entrypoint-script",
                                            mountPath: "/scripts/entrypoint.sh",
                                            subPath: "entrypoint.sh"
                                        }
                                    ],
                                }],
                                volumes: [
                                {
                                    name: "entrypoint-script",
                                    configMap: {
                                        name: entrypointConfigMap.metadata.name,
                                        defaultMode: 0o755,
                                    },
                                },
                                {
                                    name: "data-volume",
                                    hostPath: {
                                        path: `/mnt/fb/rabbitmq/${section}-${type}`,
                                        type: "DirectoryOrCreate"
                                    }
                                }]
                            },
                        }
                    },
                }, { dependsOn: [entrypointConfigMap, headlessService] })
                statefulSets[resourceName] = statefulSet
                
                // Create ClusterIP service for internal access
                const clusterIPService = new k8s.core.v1.Service(serviceName, {
                    metadata: { name: serviceName, namespace: namespace.metadata.name },
                    spec: {
                        type: "ClusterIP",
                        selector: { app: "rabbitmq", section: section, type: type },
                        ports: [
                            {
                                name: "amqp",
                                port: typeConf.port.amqp.cluster,
                                targetPort: 5672,
                            },
                            {
                                name: "management",
                                port: typeConf.port.management.cluster,
                                targetPort: 15672,
                            },
                        ],
                    },
                }, { dependsOn: [statefulSet] });

                // Create NodePort service for external access
                const nodeportService = new k8s.core.v1.Service(`${serviceName}-nodeport`, {
                    metadata: { name: `${serviceName}-nodeport`, namespace: namespace.metadata.name },
                    spec: {
                        type: "NodePort",
                        selector: { app: "rabbitmq", section: section, type: type },
                        ports: [
                            {
                                name: `amqp-${section}-${type}`,
                                port: typeConf.port.amqp.cluster,
                                targetPort: 5672,
                                nodePort: typeConf.port.amqp.node,
                            },
                            {
                                name: `management-${section}-${type}`,
                                port: typeConf.port.management.cluster,
                                targetPort: 15672,
                                nodePort: typeConf.port.management.node,
                            },
                        ],
                    },
                }, { dependsOn: [statefulSet] });

                // Collect all resources
                resources.push(headlessService)
                resources.push(clusterIPService)
                resources.push(nodeportService)
                resources.push(statefulSet)
            }
        }

        return resources
    }
}