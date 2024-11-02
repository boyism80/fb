const pulumi = require("@pulumi/pulumi");
const k8s = require("@pulumi/kubernetes");
const mysql = require('./mysql')
const rabbitmq = require('./rabbitmq')
const redis = require('./redis')
const internal = require('./internal')
const db = require('./db')
const game = require('./game')
const fs = require('fs');
const path = require('path')

const namespace = new k8s.core.v1.Namespace("fb-namespace", {
    metadata: {
        name: "fb",
    }
});

const storageClass = new k8s.storage.v1.StorageClass("storage-class", {
    metadata: {
        name: "storage-class",
        namespace: namespace.metadata.name, 
    },
    provisioner: "kubernetes.io/no-provisioner",
    volumeBindingMode: "WaitForFirstConsumer",
    reclaimPolicy: "Retain"
});

const filePath = path.join(__dirname, 'develop.json')
const content = fs.readFileSync(filePath, 'utf-8')
const conf = JSON.parse(content)

const mysqlConfigs = []
for(const [section, v] of Object.entries(conf.mysql)) {
    for(const [id, config] of Object.entries(v)) {
        mysqlConfigs.push({
            port: config.port
        })
    }
}
mysql.setup(namespace, storageClass, mysqlConfigs)


const redisConfigs = []
for(const [section, config] of Object.entries(conf.redis)) {
    redisConfigs.push({
        port: config.port
    })
}
redis.setup(namespace, storageClass, redisConfigs)


const rabbitmqConfigs = []
for(const [section, config] of Object.entries(conf.rabbitmq)) {
    rabbitmqConfigs.push({
        port: config.port
    })
}
rabbitmq.setup(namespace, storageClass, rabbitmqConfigs)

const httpInternalConfigs = []
for(const [section, config] of Object.entries(conf.internal)) {
    httpInternalConfigs.push({
        rabbitmq: conf.rabbitmq[config.rabbitmq],
        redis: conf.redis[config.redis],
        port: config.port
    })
}
internal.setup(namespace, httpInternalConfigs)


const httpDbConfigs = []
for(const [section, config] of Object.entries(conf.db)) {
    httpDbConfigs.push({
        mysql: conf.mysql[config.mysql],
        redis: conf.redis[config.redis],
        port: config.port
    })
}
db.setup(namespace, httpDbConfigs)

game.setup(namespace, conf)