pipeline {
    agent { label 'slave' }
    stages {
        stage('Initialize') {
            when {
                expression { params.CLEAN == true }
            }
            steps {
                dir('internal') {
                    sh 'rm -rf build'
                }
                dir('gateway') {
                    sh 'rm -rf build'
                }
                dir('login') {
                    sh 'rm -rf build'
                }
                dir('game') {
                    sh 'rm -rf build'
                }
            }
        }

        stage('Preperence') {
            steps {
                sh 'docker pull cshyeon/fb:build'
                sh 'docker pull cshyeon/fb:latest'

                sh 'fab -f deploy/fabfile.py optimize'
                sh 'fab -f deploy/fabfile.py environment:${ENVIRONMENT} stop setup'
            }
        }

        stage('Build') {
            steps { 
                sh 'fab -f deploy/fabfile.py build_cpp:game'
                sh 'fab -f deploy/fabfile.py build_cpp:login'
                sh 'fab -f deploy/fabfile.py build_cpp:gateway'
                sh 'fab -f deploy/fabfile.py build_dotnet:internal'
            }
        }

        stage('Deploy') {
            steps { 
                sh 'fab -f deploy/fabfile.py environment:${ENVIRONMENT} deploy_dotnet:internal'
                sh 'fab -f deploy/fabfile.py environment:${ENVIRONMENT} deploy_cpp:gateway'
                sh 'fab -f deploy/fabfile.py environment:${ENVIRONMENT} deploy_cpp:login'
                sh 'fab -f deploy/fabfile.py environment:${ENVIRONMENT} deploy_cpp:game'
            }
        }

        stage('Clean Up') {
            steps {
                sh 'fab -f deploy/fabfile.py clean'
                sh 'fab -f deploy/fabfile.py environment:${ENVIRONMENT} prune'
            }
        }
    }

    post {
        always {
            discordSend description: "'Kingdom of the wind' build complete.\nBuild ${env.BUILD_NUMBER}", 
                        footer: currentBuild.currentResult, 
                        link: env.BUILD_URL, 
                        result: currentBuild.currentResult, 
                        title: "Kingdom of the Wind", 
                        webhookURL: "https://discordapp.com/api/webhooks/797560289905082388/y1bsZom7NB61ssDyxz4h0ByE9TMr3UfCjY5j_raAecuMdiQg79FdvVbat5ZzaalmOFTC",
                        thumbnail: "https://w.namu.la/s/ed2ca36f0585f9ad670c225e05de8ad0032447c0e9cbfa92126030bcffa0630243c1bd7c54cc3622713b05a18f09a08075f1be803f3083336f73a99d1dbed51e962cd08d2fa4dd34fecb4625b3bbd05efd56e03553b5a033ecd60cd933090f40"
        }
    }
}
