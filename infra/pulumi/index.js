const pulumi = require("@pulumi/pulumi");
const k8s = require("@pulumi/kubernetes");
const mysql = require('./mysql')
const rabbitmq = require('./rabbitmq')
const redis = require('./redis')


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

mysql.setup(namespace, storageClass, 4)
rabbitmq.setup(namespace, storageClass)
redis.setup(namespace, storageClass)