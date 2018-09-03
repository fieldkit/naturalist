@Library('conservify') _

conservifyProperties()

timestamps {
    node () {
        conservifyBuild(name: 'naturalist', archive: "build/firmware/main/*.bin")

        distributeFirmware(module: 'fk-naturalist', directory: "build/firmware/main")
    }

    refreshDistribution()
}
