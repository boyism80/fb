const pulumi = require("@pulumi/pulumi");
const k8s = require("@pulumi/kubernetes");
const mysql =require('./mysql')


const namespace = new k8s.core.v1.Namespace("fb-namespace", {
    metadata: {
        name: "fb",
    }
});

mysql.setup(namespace, 4)