const pulumi = require("@pulumi/pulumi");
const k8s = require("@pulumi/kubernetes");

module.exports = function () {

    return {
        setup: function (namespace, conf, dependsOn) {

            let index = 0
            const ports = []
            for(const [section, sectionConf] of Object.entries(conf.login)) {
                const config = {
                    id: 0,
                    name: `login-${section}`,
                    ip: conf.host,
                    port: sectionConf.port,
                    thread: {
                        logic: 12,
                        io: 12,
                        background: 8
                    },
                    internal: {
                        ip: `internal-${sectionConf.internal}`,
                        port: conf.internal[sectionConf.internal].port.cluster
                    },
                    'transfer delay': 0,
                    'allow other language': true,
                    forbidden: [ "김대중", "노무현" ],
                    agreement: "바람의나라에 접속하셨습니다.\n\n중요한 내용입니다. 꼭 읽어주세요.\n읽어보시고 동의하시면 'a'키를, 동의하지 않으시면 'd' 키를 눌러주세요.\n'a' 키를 누르심으로써 여러분은 아래 계약을 따르기로 동의합니다.\n\n1. 바람의나라 이용자는 지정된 아이디를 사용하며 본 게임의 호스트 컴퓨터에 정당한 방법으로만 접속하여 본 게임을 이용하실 수 있습니다.\n기타 게임내의 각종 이벤트와 아이템 게임내용 자체는 서버에서 모든 권리를 소유하며, 수정이 필요하다고 판단될 때는 임의로 변경할 권리가 서버에 있습니다.\n이러한 게임내 변경사항의 적용을 위해서나 기타 필요하다고 판단될 때, 서버는 서비스의 감정적인 중단을 할 권리를 갖습니다.\n\n2. 본 게임이나 호스트 컴퓨터에 정당하지 아니한 방법으로 접속하거나, 타인의 아이디/계정을 무단으로 도용하여 사용하는 행위가 적발될 시에는 서버내에서 제재를 받을 수 있습니다.\n서버는 이용자가 다음 각 호에 해당하는 행위를 하였을경우 사전 통지없이 이용 계약을 해지하거나 또는, 기간을 정하여 서비스 이용을 중지할 수 있습니다.",
                    admin_mode: true,
                    log: ["debug", "info", "warn", "fatal"],
                    init: {
                        map: 1,
                        position: [
                            { x: 6, y: 6 },
                            { x: 14, y: 6 },
                            { x: 6, y: 12 },
                            { x: 14, y: 12 }
                        ],
                        hp: {
                            base: 50,
                            range: 10
                        },
                        mp: {
                            base: 30,
                            range: 10
                        }
                    },
                    name_size: {
                        min: 2,
                        max: 256
                    },
                    pw_size: {
                        min: 4,
                        max: 16
                    }
                }


                const configMap = new k8s.core.v1.ConfigMap(`login-${section}`, {
                    metadata: { name: `login-${section}`, namespace: namespace.metadata.name },
                    data: {
                        "config.json": JSON.stringify(config),
                    },
                })

                const statefulSet = new k8s.apps.v1.StatefulSet(`login-${section}`, {
                    metadata: {
                        name: `login-${section}`,
                        namespace: namespace.metadata.name,
                    },
                    spec: {
                        serviceName: "login",
                        replicas: sectionConf.replicas,
                        selector: {
                            matchLabels: {
                                app: "login",
                            },
                        },
                        template: {
                            metadata: {
                                labels: {
                                    app: "login",
                                },
                            },
                            spec: {
                                nodeSelector: {
                                    cpu: "epyc"
                                },
                                containers: [
                                    {
                                        name: "login",
                                        image: "ghcr.io/boyism80/fb/login:latest",
                                        imagePullPolicy: "Always",
                                        securityContext: {
                                            capabilities: {
                                                add: ["SYS_PTRACE"]
                                            }
                                        },
                                        ports: [
                                            { containerPort: sectionConf.port, name: `login-${index}` },
                                        ],
                                        volumeMounts: [{
                                            name: "config-volume",
                                            mountPath: "/app/config/config.json",
                                            subPath: "config.json"
                                        }],
                                    },
                                ],
                                volumes: [{
                                    name: "config-volume",
                                    configMap: {
                                        name: configMap.metadata.name,
                                    },
                                }],
                            },
                        },
                    },
                }, { dependsOn: dependsOn })

                ports.push({
                    name: `login-${section}`,
                    port: sectionConf.port,
                    targetPort: `login-${index}`,
                    nodePort: sectionConf.port 
                })

                index++
            }

            return new k8s.core.v1.Service('login', {
                metadata: {
                    name: 'login',
                    namespace: namespace.metadata.name,
                },
                spec: {
                    type: "NodePort",
                    ports: ports,
                    selector: {
                        app: "login",
                    }
                },
            }, { dependsOn: dependsOn })
        }
    }
}()