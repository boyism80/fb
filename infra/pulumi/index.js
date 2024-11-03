const pulumi = require("@pulumi/pulumi");
const k8s = require("@pulumi/kubernetes");
const mysql = require('./mysql')
const rabbitmq = require('./rabbitmq')
const redis = require('./redis')
const internal = require('./internal')
const db = require('./db')
const gateway = require('./gateway')
const login = require('./login')
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

mysql.setup(namespace, storageClass, conf)
redis.setup(namespace, storageClass, conf)
rabbitmq.setup(namespace, storageClass, conf)
internal.setup(namespace, conf)
db.setup(namespace, conf)
gateway.setup(namespace, conf)
login.setup(namespace, conf)
game.setup(namespace, conf)