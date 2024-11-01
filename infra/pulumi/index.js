const pulumi = require("@pulumi/pulumi");
const k8s = require("@pulumi/kubernetes");
const mysql = require('./mysql')
const rabbitmq = require('./rabbitmq')
const redis = require('./redis')
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
const json = JSON.parse(content)

const mysqlConfigs = []
for(const [section, v] of Object.entries(json.mysql)) {
    for(const [id, config] of Object.entries(v)) {
        mysqlConfigs.push({
            port: config.port
        })
    }
}
mysql.setup(namespace, storageClass, mysqlConfigs)


const redisConfigs = []
for(const [section, config] of Object.entries(json.redis)) {
    redisConfigs.push({
        port: config.port
    })
}
redis.setup(namespace, storageClass, redisConfigs)


const rabbitmqConfigs = []
for(const [section, config] of Object.entries(json.rabbitmq)) {
    rabbitmqConfigs.push({
        port: config.port
    })
}