pipeline {
    agent { label 'slave' }
    stages {
        stage('Build') {
            parallel { 
                stage('Build internal') {
                    steps{ 
                        dir('internal') {
                            sh 'mkdir -p bin'
                            // sh 'make clean'
                            sh 'make -j4'
                        }
                    }
                }
                stage('Build gateway') {
                    steps{ 
                        dir('gateway') {
                            sh 'mkdir -p bin'
                            // sh 'make clean'
                            sh 'make -j4'
                        }
                    }
                }
                stage('Build login') {
                    steps{ 
                        dir('login') {
                            sh 'mkdir -p bin'
                            // sh 'make clean'
                            sh 'make -j4'
                        }
                    }
                }
                stage('Build game') {
                    steps{ 
                        dir('game') {
                            sh 'mkdir -p bin'
                            // sh 'make clean'
                            sh 'make -j4'
                        }
                    }
                }
            }
        }
        stage('Deploy') {
            steps {
                dir('resources/maps') {
                    sh 'rm -f *.map'
                    sh 'rm -f *.block'
                    sh 'unzip -qq maps.zip'
                }
                sh 'fab -f deploy/fabfile.py environment:${ENVIRONMENT} deploy'
            }
        }
    }
}
