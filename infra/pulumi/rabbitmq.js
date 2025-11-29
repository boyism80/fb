const pulumi = require("@pulumi/pulumi");
const k8s = require("@pulumi/kubernetes");

module.exports = {
    setup: function (namespace, conf) {
        let index = 0
        const ports = []

        // Entrypoint script that enables sharding plugin and sets up clustering
        // First pod (ordinal 0) starts standalone, others join the first pod
        const entrypointScript = `#!/bin/bash
set -e

# Get pod name and namespace from environment variables
MY_POD_NAME="$MY_POD_NAME"
MY_POD_NAMESPACE="$MY_POD_NAMESPACE"
FIRST_POD_NAME="$1"

# Construct current pod's node name
POD_NAME="rabbit@${MY_POD_NAME}.rabbitmq.${MY_POD_NAMESPACE}.svc.cluster.local"

# Set RABBITMQ_NODENAME environment variable
export RABBITMQ_NODENAME="$POD_NAME"

# Check if this is the first pod (ordinal 0)
if [[ "$MY_POD_NAME" == *"-0" ]]; then
    IS_FIRST_POD="true"
else
    IS_FIRST_POD="false"
fi

# Function to wait for RabbitMQ to be ready
wait_for_rabbitmq() {
    local max_attempts=60
    local attempt=0
    while [ $attempt -lt $max_attempts ]; do
        if rabbitmqctl status > /dev/null 2>&1; then
            return 0
        fi
        attempt=$((attempt + 1))
        sleep 1
    done
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
/usr/local/bin/docker-entrypoint.sh rabbitmq-server -detached

# Wait for RabbitMQ to be ready
echo "Waiting for RabbitMQ to start..."
if ! wait_for_rabbitmq; then
    echo "Failed to start RabbitMQ"
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

        for(const [section, sectionConf] of Object.entries(conf.rabbitmq)) {
            const replicas = sectionConf.replicas || 1
            const firstPodName = `rabbit@rabbitmq-${section}-0.rabbitmq.${namespace.metadata.name}.svc.cluster.local`
            
            const statefulSet = new k8s.apps.v1.StatefulSet(`rabbitmq-${section}`, {
                metadata: { name: `rabbitmq-${section}`, namespace: namespace.metadata.name, },
                spec: {
                    serviceName: "rabbitmq",
                    replicas: replicas,
                    selector: { matchLabels: { app: "rabbitmq", section: section } },
                    template: {
                        metadata: { labels: { app: "rabbitmq", section: section } },
                        spec: {
                            nodeSelector: {
                                "kubernetes.io/hostname": "ubuntu-1"
                            },
                            subdomain: "rabbitmq",
                            containers: [{
                                name: "rabbitmq",
                                image: "rabbitmq:management",
                                ports: [
                                    { name: `amqp-${index}`, containerPort: 5672 },
                                    { name: `management-${index}`, containerPort: 15672 },
                                ],
                                env: [
                                    { name: "RABBITMQ_DEFAULT_USER", value: "fb" }, // Default user
                                    { name: "RABBITMQ_DEFAULT_PASS", value: "admin" }, // Default user password
                                    { name: "RABBITMQ_ERLANG_COOKIE", value: "secret-cookie" },
                                    { name: "K8S_SERVICE_NAME", value: "rabbitmq" },
                                    { name: "RABBITMQ_USE_LONGNAME", value: "true" },
                                    { 
                                        name: "MY_POD_NAME",
                                        valueFrom: {
                                            fieldRef: {
                                                fieldPath: "metadata.name"
                                            }
                                        }
                                    },
                                    { 
                                        name: "MY_POD_NAMESPACE",
                                        valueFrom: {
                                            fieldRef: {
                                                fieldPath: "metadata.namespace"
                                            }
                                        }
                                    },
                                ],
                                command: ["/bin/bash"],
                                args: ["/scripts/entrypoint.sh", firstPodName],
                                volumeMounts: [
                                    { 
                                        name: "data-volume", 
                                        mountPath: "/var/lib/rabbitmq" 
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
                            }]
                        },
                    },
                    volumeClaimTemplates: [{
                        metadata: { name: "data-volume" },
                        spec: {
                            accessModes: ["ReadWriteOnce"],
                            resources: {
                                requests: {
                                    storage: "10Gi"
                                }
                            }
                        }
                    }]
                },
            }, { dependsOn: [entrypointConfigMap] })

            ports.push({
                name: `amqp-${section}`,
                port: sectionConf.port.amqp.cluster,
                targetPort: `amqp-${index}`,
                nodePort: sectionConf.port.amqp.node,
            })
            ports.push({
                name: `management-${section}`,
                port: sectionConf.port.management.cluster,
                targetPort: `management-${index}`,
                nodePort: sectionConf.port.management.node,
            })

            index++
        }

        // Create headless service for StatefulSet DNS resolution (for load balancing)
        const headlessService = new k8s.core.v1.Service("rabbitmq", {
            metadata: { name: "rabbitmq", namespace: namespace.metadata.name },
            spec: {
                clusterIP: "None",
                selector: { app: "rabbitmq" },
                ports: [
                    { name: "amqp", port: 5672, targetPort: 5672 },
                    { name: "management", port: 15672, targetPort: 15672 },
                ],
            },
        });

        // Create NodePort service for external access (load balances across all replicas)
        return new k8s.core.v1.Service("rabbitmq-nodeport", {
            metadata: { name: "rabbitmq-nodeport", namespace: namespace.metadata.name },
            spec: {
                type: "NodePort",
                selector: { app: "rabbitmq" },
                ports: ports,
            },
        }, { dependsOn: [headlessService] });
    }
}