pipeline {
    agent { label 'slave' }
    stages {
        stage('Initialize') {
            when {
                expression { params.CLEAN == true }
            }
            steps {
                dir('internal') {
                    sh 'make clean'
                }
                dir('gateway') {
                    sh 'make clean'
                }
                dir('login') {
                    sh 'make clean'
                }
                dir('game') {
                    sh 'make clean'
                }
            }
        }

        stage('Build') {
            parallel { 
                stage('Build internal') {
                    steps{ 
                        dir('internal') {
                            sh 'mkdir -p /dist/fb/internal'
                            sh 'make -j4'
                        }
                    }
                }
                stage('Build gateway') {
                    steps{ 
                        dir('gateway') {
                            sh 'mkdir -p /dist/fb/gateway'
                            sh 'make -j4'
                        }
                    }
                }
                stage('Build login') {
                    steps{ 
                        dir('login') {
                            sh 'mkdir -p /dist/fb/login'
                            sh 'make -j4'
                        }
                    }
                }
                stage('Build game') {
                    steps{ 
                        dir('game') {
                            sh 'mkdir -p /dist/fb/game'
                            sh 'make -j4'
                        }
                    }
                }
            }
        }

        stage('Preperence') {
            steps {
                dir('resources/maps') {
                    sh 'rm -f *.map'
                    sh 'rm -f *.block'
                    sh 'unzip -qq maps.zip'
                }
                sh 'fab -f deploy/fabfile.py optimize'
                sh 'fab -f deploy/fabfile.py environment:${ENVIRONMENT} docker_stop'
            }
        }

        stage('Deploy') {
            parallel {
                stage('Deploy internal') {
                    steps{ 
                        sh 'fab -f deploy/fabfile.py environment:${ENVIRONMENT} internal'
                    }
                }
                stage('Deploy gateway') {
                    steps{ 
                        sh 'fab -f deploy/fabfile.py environment:${ENVIRONMENT} gateway'
                    }
                }
                stage('Deploy login') {
                    steps{ 
                        sh 'fab -f deploy/fabfile.py environment:${ENVIRONMENT} login'
                    }
                }
                stage('Deploy game') {
                    steps{ 
                        sh 'fab -f deploy/fabfile.py environment:${ENVIRONMENT} game'
                    }
                }
            }
        }

        stage('Clean Up') {
            steps {
                sh 'fab -f deploy/fabfile.py environment:${ENVIRONMENT} docker_rm'
                sh 'fab -f deploy/fabfile.py cleanup'
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
