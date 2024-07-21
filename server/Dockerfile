# Use the official Golang image as the base image
FROM golang:1.22

# Set the working directory inside the container
WORKDIR /usr/ephemeraldb-client

# Copy the Go Modules files first and download the dependencies
COPY go.mod go.sum ./
RUN go mod download

# Copy the rest of the application's source code
COPY . .

# Set the working directory for the build
WORKDIR /usr/ephemeraldb-client/src

# Build the Go application
RUN go build -v -o /usr/ephemeraldb-client/bin/main

# Command to run the application
CMD ["/usr/ephemeraldb-client/bin/main"]
