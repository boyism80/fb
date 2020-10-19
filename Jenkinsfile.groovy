pipeline {
    agent { label 'master' }
    stages {
        stage('Build') {
            parallel { 
                stage('Build internal') {
                    steps{ 
                        dir('internal') {
                            sh 'mkdir -p bin'
                            sh 'make'
                        }
                    }
                }
                stage('Build gateway') {
                    steps{ 
                        dir('gateway') {
                            sh 'mkdir -p bin'
                            sh 'make'
                        }
                    }
                }
                stage('Build login') {
                    steps{ 
                        dir('login') {
                            sh 'mkdir -p bin'
                            sh 'make'
                        }
                    }
                }
                stage('Build game') {
                    steps{ 
                        dir('game') {
                            sh 'mkdir -p bin'
                            sh 'make'
                        }
                    }
                }
            }
        }
        stage('Deploy') {
            steps{
                echo 'none'
            }
        }
    }
}
