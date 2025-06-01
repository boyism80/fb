#!/bin/bash

set -e

# Function to print error messages
function error_exit {
  echo "Error: $1"
  exit 1
}

# Obtain external IP and store it
echo "Retrieving external IP..."
EXTERNAL_IP=$(curl -s https://ifconfig.me)
if [[ -z "$EXTERNAL_IP" ]]; then
  error_exit "Failed to retrieve external IP."
fi
echo "Detected External IP: $EXTERNAL_IP"

# Prompt user to confirm or override
read -p "Enter external IP [${EXTERNAL_IP}]: " INPUT_IP
if [[ -n "$INPUT_IP" ]]; then
  EXTERNAL_IP="$INPUT_IP"
fi
echo "Using External IP: $EXTERNAL_IP"

# Build Docker images with error handling
echo "Building Docker images..."

if ! sudo docker buildx build --progress=plain --tag fb/build:latest -f Dockerfile .; then
  error_exit "Docker build failed for fb/build:latest"
fi

if ! sudo docker buildx build --progress=plain --tag fb/gateway:latest -f gateway/Dockerfile .; then
  error_exit "Docker build failed for fb/gateway:latest"
fi

if ! sudo docker buildx build --progress=plain --tag fb/login:latest -f login/Dockerfile .; then
  error_exit "Docker build failed for fb/login:latest"
fi

if ! sudo docker buildx build --progress=plain --tag fb/game:latest -f game/Dockerfile .; then
  error_exit "Docker build failed for fb/game:latest"
fi

if ! sudo docker buildx build --progress=plain --tag fb/bot:latest -f bot/Dockerfile .; then
  error_exit "Docker build failed for fb/bot:latest"
fi

if ! sudo docker buildx build --progress=plain --tag fb/internal:latest -f http/Dockerfile --build-arg SERVICE=internal .; then
  error_exit "Docker build failed for fb/internal:latest"
fi

if ! sudo docker buildx build --progress=plain --tag fb/write-back:latest -f http/Dockerfile --build-arg SERVICE=write-back .; then
  error_exit "Docker build failed for fb/write-back:latest"
fi

echo "Docker images built successfully."

# Change directory to Pulumi folder
pushd infra/pulumi > /dev/null

# Configure Pulumi with external IP
echo "Configuring Pulumi with external IP..."
if ! pulumi config set --secret host "$EXTERNAL_IP"; then
  popd > /dev/null
  error_exit "Pulumi config set failed."
fi

# Destroy existing stack (if any) and then deploy the stack
echo "Destroying existing Pulumi stack (if any)..."
if ! pulumi down -y; then
  popd > /dev/null
  error_exit "Pulumi destroy failed."
fi

echo "Deploying Pulumi stack..."
if ! pulumi up -y; then
  popd > /dev/null
  error_exit "Pulumi up failed."
fi

popd > /dev/null

echo "Setup completed successfully."
