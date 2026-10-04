const pulumi = require("@pulumi/pulumi")

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

// Hosts that download patched tables/scripts accept only these URLs, so they must match admin-tool.
function downloadBaseUrls() {
    const stackConfig = new pulumi.Config()
    return {
        table: secretOrEnv(stackConfig, "tablePublishDownloadBaseUrl", ["TABLE_PUBLISH_DOWNLOAD_BASE_URL"]),
        script: secretOrEnv(stackConfig, "scriptPublishDownloadBaseUrl", ["SCRIPT_PUBLISH_DOWNLOAD_BASE_URL"]),
    }
}

module.exports = { secretOrEnv, downloadBaseUrls }
