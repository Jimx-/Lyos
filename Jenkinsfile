pipeline {
    agent {
        dockerfile true
    }
    stages {
        stage('Checkout') {
            steps {
                checkout scm
            }
        }
        stage('Compile') {
            steps {
                sh '''
                    ./scripts/setup-toolchain.sh -m i686
                	make SUBARCH=i686
                '''
            }
        }
    }
}