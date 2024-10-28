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
const serviceArgs = []
let index = 0
mysql.init(namespace, storageClass)
for(const [section, v] of Object.entries(json.mysql)) {
    for(const [id, config] of Object.entries(v)) {
        const name = `mysql-${section}-${id}`
        mysql.create(name, index)
        serviceArgs.push({
            name: name,
            clusterPort: config.port.cluster,
            nodePort: config.port.node,
            index: index
        })
        index++
    }
}
mysql.setup(serviceArgs)

// rabbitmq.setup(namespace, storageClass)
// redis.setup(namespace, storageClass)