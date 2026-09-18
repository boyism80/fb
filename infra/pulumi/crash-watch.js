const pulumi = require("@pulumi/pulumi")
const k8s = require("@pulumi/kubernetes")

function secretOrEnv(stackConfig, configKey, envKeys) {
    const fromConfig = stackConfig.getSecret(configKey)
    if (fromConfig !== undefined)
        return fromConfig

    for (const key of envKeys) {
        const value = process.env[key]
        if (value)
            return value
    }

    return ""
}

module.exports = {
    setup: function (namespace, dependsOn) {
        const stackConfig = new pulumi.Config()
        const botToken = secretOrEnv(stackConfig, "discordBotToken", ["DISCORD_BOT_TOKEN"])
        const channelId = secretOrEnv(stackConfig, "discordChannelId", ["DISCORD_CHANNEL_ID"])
        const appLabels = { app: "crash-watch" }

        const secret = new k8s.core.v1.Secret("discord-bot", {
            metadata: { name: "discord-bot", namespace: namespace.metadata.name },
            type: "Opaque",
            stringData: {
                token: botToken,
                "channel-id": channelId,
            },
        }, { dependsOn: dependsOn })

        const serviceAccount = new k8s.core.v1.ServiceAccount("crash-watch", {
            metadata: { name: "crash-watch", namespace: namespace.metadata.name },
        }, { dependsOn: dependsOn })

        const role = new k8s.rbac.v1.Role("crash-watch", {
            metadata: { name: "crash-watch", namespace: namespace.metadata.name },
            rules: [
                { apiGroups: [""], resources: ["pods"], verbs: ["get", "list", "watch"] },
                { apiGroups: [""], resources: ["pods/log"], verbs: ["get"] },
            ],
        }, { dependsOn: dependsOn })

        const roleBinding = new k8s.rbac.v1.RoleBinding("crash-watch", {
            metadata: { name: "crash-watch", namespace: namespace.metadata.name },
            roleRef: {
                apiGroup: "rbac.authorization.k8s.io",
                kind: "Role",
                name: "crash-watch",
            },
            subjects: [{
                kind: "ServiceAccount",
                name: "crash-watch",
                namespace: namespace.metadata.name,
            }],
        }, { dependsOn: [role, serviceAccount] })

        const deployment = new k8s.apps.v1.Deployment("crash-watch", {
            metadata: { name: "crash-watch", namespace: namespace.metadata.name },
            spec: {
                replicas: 1,
                selector: { matchLabels: appLabels },
                template: {
                    metadata: { labels: appLabels },
                    spec: {
                        serviceAccountName: "crash-watch",
                        containers: [{
                            name: "crash-watch",
                            image: "ghcr.io/boyism80/fb/crash-watch:latest",
                            imagePullPolicy: "Always",
                            env: [
                                { name: "WATCH_NAMESPACE", value: namespace.metadata.name },
                                {
                                    name: "DISCORD_BOT_TOKEN",
                                    valueFrom: {
                                        secretKeyRef: {
                                            name: "discord-bot",
                                            key: "token",
                                            optional: true,
                                        },
                                    },
                                },
                                {
                                    name: "DISCORD_CHANNEL_ID",
                                    valueFrom: {
                                        secretKeyRef: {
                                            name: "discord-bot",
                                            key: "channel-id",
                                            optional: true,
                                        },
                                    },
                                },
                            ],
                        }],
                    },
                },
            },
        }, { dependsOn: [secret, roleBinding] })

        return [secret, serviceAccount, role, roleBinding, deployment]
    }
}
