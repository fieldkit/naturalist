@Library('conservify') _

conservifyProperties()

timestamps {
    node () {
        conservifyBuild(name: 'naturalist', archive: true)
        distributeFirmware()
    }

    refreshDistribution()
}
